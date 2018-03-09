#include <voice_manager.h>

void voice_manager::init()
{
    // define crucial values
    m_ms_to_samples = dsp_samples_to_ms * static_cast<float>(dsp_sample_rate);
    // initialize components
    m_params.init();
    m_decoder.init();
    m_synth.init();
    // initialize signal (zero everywhere)
    for(int v = 0; v < dsp_number_of_voices; v++)
    {
        // init param signals
        for(int s = 0; s < sig_number_of_signal_items; s++)
        {
            m_signal[v][s] = 0;
        };
    };
};

void voice_manager::evalMidiEvent(int status, int data0, int data1)
{
    // is status byte argument 0...127? or 128...255? (msb present, wrapping may be needed)
    // status byte passed by parent of voice_manager passes status containing msb = 1, so wrapping is required
    // main project already has simple param_engine and passing different midi modes present, by #defines and #ifdef
    if(ext_midi_mode == 0) {
        // "anton" mode
        if((status & 15) != ext_midi_control_channel_id) {
            // 15 midi channels for anton's code with familiar arguments
            int ctrlId = data0;
            float ctrlVal = static_cast<float>(data1) / 127;
            std::cout << "midi in (a-mode): (ctrlId, ctrlVal): (" << ctrlId << ", " << ctrlVal << ")" << std::endl;
            // ...
        } else {
            // 1 reserved midi channel for pseudo TCD control (wrap status in order to ignore msb)
            std::cout << "midi in (a-mode, pseudo TCD) (st, d0, d1): (" << status << ", " << data0 << ", " << data1 << ")" << std::endl;
            // ...
        };
    } else {
        // pure tcd mode (all midi channels) -- later separate function (evalTcdMessage)
        std::cout << "TCD ";
        float f;
        switch(m_decoder.getCommandId(status & 127))
        {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 0:
            // ignore
#if dbg_tcd_in
            std::cout << "ignore" << std::endl;
#endif
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 1:
            // selectVoice
            m_decoder.m_voiceFrom = m_decoder.m_voiceTo = m_decoder.unsigned14(data0, data1);
#if dbg_tcd_in
            std::cout << "V " << m_decoder.m_voiceFrom << std::endl;
#endif
            voiceSelectionUpdate();
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 2:
            // selectMultipleVoices
            m_decoder.m_voiceTo = m_decoder.unsigned14(data0, data1);
#if dbg_tcd_in
            std::cout << "VM " << m_decoder.m_voiceTo << std::endl;
#endif
            voiceSelectionUpdate();
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 3:
            // selectParam
            m_decoder.m_paramFrom = m_decoder.m_paramTo = m_decoder.unsigned14(data0, data1);
#if dbg_tcd_in
            std::cout << "P " << m_decoder.m_paramFrom << std::endl;
#endif
            paramSelectionUpdate();
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 4:
            // selectMultipleParams
            m_decoder.m_paramTo = m_decoder.unsigned14(data0, data1);
#if dbg_tcd_in
            std::cout << "PM " << m_decoder.m_paramTo << std::endl;
#endif
            paramSelectionUpdate();
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 5:
            // setTime (pass sample-based integer time to param_engine, which evaluates by clock type)
            f = 1 / static_cast<float>((dsp_upsample_factor * m_decoder.unsigned14(data0, data1)) + 1);
#if dbg_tcd_in
            std::cout << "dx " << f << std::endl;
#endif
            timeUpdate(f);
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 6:
            // setTimeUpper (unsigned)
            m_decoder.unsigned28upper(data0, data1);
#if dbg_tcd_in
            std::cout << "TU " << m_decoder.m_value << std::endl;
#endif
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 7:
            // setTimeLower
            f = 1 / static_cast<float>((dsp_upsample_factor * m_decoder.apply28lower(data0, data1)) + 1);
#if dbg_tcd_in
            std::cout << "dx " << f << std::endl;
#endif
            timeUpdate(f);
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 8:
            // setDestination (unsigned)
            f = static_cast<float>(m_decoder.unsigned14(data0, data1));
#if dbg_tcd_in
            std::cout << "D " << f << std::endl;
#endif
            if(m_decoder.m_listId == 0)
            {
                destinationUpdate(f);
            } else {
                listUpdate(f);
            };
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 9:
            // setSignedDestination
            f = static_cast<float>(m_decoder.signed14(data0, data1));
#if dbg_tcd_in
            std::cout << "DS " << f << std::endl;
#endif
            if(m_decoder.m_listId == 0)
            {
                destinationUpdate(f);
            } else {
                listUpdate(f);
            };
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 10:
            // setDestinationUpper (signed)
            m_decoder.signed28upper(data0, data1);
#if dbg_tcd_in
            std::cout << "DU " << (m_decoder.m_sign * m_decoder.m_value) << std::endl;
#endif
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 11:
            // setDestinationLower
            f = static_cast<float>(m_decoder.apply28lower(data0, data1));
#if dbg_tcd_in
            std::cout << "DL " << f << std::endl;
#endif
            if(m_decoder.m_listId == 0)
            {
                destinationUpdate(f);
            } else {
                listUpdate(f);
            };
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 12:
            // preload - separate function
#if dbg_tcd_in
            std::cout << "PL " << data1 << " (listID: " << data0 << ")" << std::endl;
#endif
            preloadUpdate(data1, data0);
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 13:
            // keyUp (preloadable)
            f = static_cast<float>(m_decoder.unsigned14(data0, data1));
#if dbg_tcd_in
            std::cout << "KU " << f << std::endl;
#endif
            keyUp(m_decoder.m_voiceFrom, f);
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 14:
            // keyDown (preloadable)
            f = static_cast<float>(m_decoder.unsigned14(data0, data1));
#if dbg_tcd_in
            std::cout << "KD " << f << std::endl;
#endif
            keyDown(m_decoder.m_voiceFrom, f);
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 15:
            // monophonic flush (current, monophonic reaktor-based approach), not preloadable
#if dbg_tcd_in
            std::cout << "FL " << std::endl;
#endif
            // actual flushing after wait, fadepoint handling by rendering
            // fade out and start wait mechanism (flush engine):
            /*
            m_params.m_fadepoint = 0;   // fade out
            m_flush.m_position = 0;     // reset wait counter position
            m_flush.m_state = 1;        // onstate of counter
            */
            m_synth.flushBuffer();
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 16:
            // selectUtility (established in protocol) - for global, monophonic parameters (not preloadable)
            m_decoder.m_utilityId = m_decoder.unsigned14(data0, data1);
#if dbg_tcd_in
            std::cout << "U " << m_decoder.m_utilityId << std::endl;
#endif
            break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case 17:
            // setUtility (established in protocol)
            f = static_cast<float>(m_decoder.unsigned14(data0, data1));
#if dbg_tcd_in
            std::cout << "UD " << f << std::endl;
#endif
            utilityUpdate(f);
            break;
        };
    };
};

void voice_manager::tickMain()
{
    // simplify post processing (one function (distribute mono to poly, copy poly) per clock)
    // eval timestamp (clocks & rates), voice loop, order
    int i;
    int v;
    int p;
    // (sync tick does not exist, sync post processing is currently impossible)
    // - - - - - - - - - - - - - - - - - - - - -
    // slow tick according to clock division (param_engine & post processing)
    if(m_clockPosition[3] == 0) {
        // mono: (params)
        for(p = 0; p < m_params.m_clock_ids.m_data[3].m_data[0].m_length; p++)
        {
            i = m_params.m_head[m_params.m_clock_ids.m_data[3].m_data[0].m_data[p]].m_index;
            m_params.tickItem(i);
        };
        // - - - - - - - - - - - - - - - - - - - - -
        // poly: (params, post)
        for(v = 0; v < dsp_number_of_voices; v++)
        {
            for(p = 0; p < m_params.m_clock_ids.m_data[3].m_data[1].m_length; p++)
            {
                i = m_params.m_head[m_params.m_clock_ids.m_data[3].m_data[1].m_data[p]].m_index + v;
                m_params.tickItem(i);
            };
            // one post proc function
            m_params.postProcess_slow(m_signal[v], v);
            // trigger audio engine slow (voiceId)
        };
    };
    // - - - - - - - - - - - - - - - - - - - - -
    // fast tick according to clock division (param_engine & post processing)
    if(m_clockPosition[2] == 0) {
        // mono: flush tick (fast rendering implementation)
        //tickFlush();      // update fadepoint in poly update (pass/distribute to shared signal)
        // - - - - - - - - - - - - - - - - - - - - -
        // mono: (params)
        for(p = 0; p < m_params.m_clock_ids.m_data[2].m_data[0].m_length; p++)
        {
            i = m_params.m_head[m_params.m_clock_ids.m_data[2].m_data[0].m_data[p]].m_index;
            m_params.tickItem(i);
        };
        // - - - - - - - - - - - - - - - - - - - - -
        // poly: (params, post)
        for(v = 0; v < dsp_number_of_voices; v++)
        {
            for(p = 0; p < m_params.m_clock_ids.m_data[2].m_data[1].m_length; p++)
            {
                i = m_params.m_head[m_params.m_clock_ids.m_data[2].m_data[1].m_data[p]].m_index + v;
                m_params.tickItem(i);
            };
            // one post proc function
            m_params.postProcess_fast(m_signal[v], v);
        };
    };
    // - - - - - - - - - - - - - - - - - - - - -
    // audio tick performs every time (global voice loop: param_engine, audio_engine)
    // mono: (params)
    for(p = 0; p < m_params.m_clock_ids.m_data[1].m_data[0].m_length; p++)
    {
        i = m_params.m_head[m_params.m_clock_ids.m_data[1].m_data[0].m_data[p]].m_index;
        m_params.tickItem(i);
    };
    // - - - - - - - - - - - - - - - - - - - - -
    // poly: (params, post, audio_engine)
    for(v = 0; v < dsp_number_of_voices; v++)
    {
        // param rendering
        for(p = 0; p < m_params.m_clock_ids.m_data[1].m_data[1].m_length; p++)
        {
            i = m_params.m_head[m_params.m_clock_ids.m_data[1].m_data[1].m_data[p]].m_index + v;
            m_params.tickItem(i);
        };
        // one post proc function (including envelope rendering)
        m_params.postProcess_audio(m_signal[v], v);
        // NO TRIGGER (if possible)
        // audio engine poly section rendering
        m_synth.renderPolySection(m_signal[v], v);
    };
    // finally, audio engine mono section rendering
    m_synth.renderMonoSection(m_signal[0]);
    // - - - - - - - - - - - - - - - - - - - - -
    // advance timestamp (fast, slow) (audio timestamp does not need counting or evaluation)
    m_clockPosition[2] = (m_clockPosition[2] + 1) % m_clockDivision[2];
    m_clockPosition[3] = (m_clockPosition[3] + 1) % m_clockDivision[3];
};

void voice_manager::voiceSelectionUpdate()
{
    // case ALL voices
    if(m_decoder.m_voiceTo == 16383)
    {
        m_decoder.m_voiceFrom = 0;
        m_decoder.m_voiceTo = 16382;
    };
    // reset voice id list (no voice id selected)
    m_decoder.m_selectedVoices.reset();
    // prepare selection event (determine if voice selection wraps around max)
    m_decoder.m_event[0] = m_decoder.m_voiceFrom > m_decoder.m_voiceTo ? 1 : 0;
    for(int v = 0; v < dsp_number_of_voices; v++)
    {
        // sort voice ids by adding ids to sublists (sublist 0 - deselected, sublist 1 - selected)
        int s = m_decoder.selectionEvent(m_decoder.m_voiceFrom, m_decoder.m_voiceTo, v);
        m_decoder.m_selectedVoices.add(s, v);
    };
};

void voice_manager::paramSelectionUpdate()
{
    // case ALL params
    if(m_decoder.m_paramTo == 16383)
    {
        m_decoder.m_paramFrom = 0;
        m_decoder.m_paramTo = 16382;
    };
    // reset param id list (no param id selected)
    m_decoder.m_selectedParams.reset();
    // prepare selection event (determine if param selection wraps around max)
    m_decoder.m_event[0] = m_decoder.m_paramFrom > m_decoder.m_paramTo ? 1 : 0;
    for(int p = 0; p < sig_number_of_params; p++)
    {
        // sort param ids by adding ids to sublists (sublist 0 - deselected, sublist 1 - selected)
        int s = m_decoder.selectionEvent(m_decoder.m_paramFrom, m_decoder.m_paramTo, m_params.m_head[p].m_id);
        m_decoder.m_selectedParams.add(m_params.m_head[p].m_polyType, s, p);
    };
};

void voice_manager::preloadUpdate(int mode, int listId)
{
    //
    int p;
    int v;
    //
    switch(mode)
    {
    case 0:
        // disable preload and reset listId
        m_params.m_preload = 0;
        m_decoder.m_listId = 0;
        break;
    case 1:
        // enable preload - set preload mode and listId, reset listIndex
        m_params.m_preload = 1;
        m_decoder.m_listId = listId;
        m_decoder.m_listIndex = 0;
        // if needed, prepare list mechanism
        // enable preload - distribution loop (reset preload counters) (params)
        for(p = 0; p < sig_number_of_param_items; p++)
        {
            m_params.m_body[p].m_preload = 0;
        };
        // key event preload updates
        m_params.m_event.m_mono.m_preload = 0;
        for(v = 0; v < dsp_number_of_voices; v++)
        {
            m_params.m_event.m_poly[v].m_preload = 0;
        };
        break;
    case 2:
        // reset preload mode and listId
        m_params.m_preload = 0;
        m_decoder.m_listId = 0;
        // apply preloaded values - distribution loop (params)
        for(p = 0; p < sig_number_of_params; p++)
        {
            for(v = 0; v < m_params.m_head[p].m_size; v++)
            {
                m_params.applyPreloaded(v, p);
            };
        };
        // apply preloaded key event(s) - evaluation here in order to trigger audio_engine osc phase reset
        if(m_params.m_event.m_mono.m_preload > 0)
        {
            m_params.m_event.m_mono.m_preload = 0;
            m_params.keyApplyMono();
        };
        for(v = 0; v < dsp_number_of_voices; v++)
        {
            if(m_params.m_event.m_poly[v].m_preload > 0)
            {
                m_params.m_event.m_poly[v].m_preload = 0;
                m_params.keyApply(v);
                keyApply(v);
            };
        };
        break;
    };
};

void voice_manager::destinationUpdate(float value)
{
    int p; int v;
    // for mono params
    for(p = 0; p < m_decoder.m_selectedParams.m_data[0].m_data[1].m_length; p++)
    {
        m_params.setDest(0, m_decoder.m_selectedParams.m_data[0].m_data[1].m_data[p], value);
    };
    // for selected voices (for selected poly params)
    for(v = 0; v < m_decoder.m_selectedVoices.m_data[1].m_length; v++)
    {
        for(p = 0; p < m_decoder.m_selectedParams.m_data[1].m_data[1].m_length; p++)
        {
            m_params.setDest(v, m_decoder.m_selectedParams.m_data[1].m_data[1].m_data[p], value);
        };
    };
};

void voice_manager::timeUpdate(float value)
{
    int p; int v;
    // for mono params
    for(p = 0; p < m_decoder.m_selectedParams.m_data[0].m_data[1].m_length; p++)
    {
        m_params.setDx(0, m_decoder.m_selectedParams.m_data[0].m_data[1].m_data[p], value);
    };
    // for selected voices (for selected poly params)
    for(v = 0; v < m_decoder.m_selectedVoices.m_data[1].m_length; v++)
    {
        for(p = 0; p < m_decoder.m_selectedParams.m_data[1].m_data[1].m_length; p++)
        {
            m_params.setDx(v, m_decoder.m_selectedParams.m_data[1].m_data[1].m_data[p], value);
        };
    };
};

void voice_manager::utilityUpdate(float value)
{
    // utilities (such as reference tone) work as sync parameters, but have predefined quasi post-processing and trigger capability
    // conceivable additions: times (macros, edit, recall, morph)
    switch(m_decoder.m_utilityId)
    {
    case 0:
        // ignore, reserved for velocity param definition (shall not be overwritten)
        break;
    case 1:
        // set reference tone (the frequency of a played un-tuned A3, usually 440 Hz)
        value *= m_params.m_utilities[1].m_normalize;
        m_params.m_pitch_reference = m_params.scale(m_params.m_utilities[1].m_scaleId, m_params.m_utilities[1].m_scaleArg, value);
        // update pitch related values
        break;
    };
};

void voice_manager::listUpdate(float dest)
{
    //
    switch(m_decoder.m_listId)
    {
    case 1:
        // recall traversal (currently monophonic)
        m_params.setDest(0, m_decoder.traverseRecall(), dest);
        break;
    case 2:
        // key event traversal (polyphonic - voice selection by sender !!!)
        m_params.setDest(m_decoder.m_voiceFrom, m_decoder.traverseKeyEvent(), dest);
        break;
    };
};

void voice_manager::keyUp(int voiceId, float velocity)
{
    // update values
    m_params.keyUp(voiceId, velocity);
    // handle preload
    if(m_params.m_preload == 0)
    {
        // direct key apply
        m_params.keyApplyMono();
        m_params.keyApply(voiceId);
        keyApply(voiceId);
    }
    else
    {
        // preload: increase PL counters
        m_params.m_event.m_mono.m_preload++;
        m_params.m_event.m_poly[voiceId].m_preload++;
    };
};

void voice_manager::keyDown(int voiceId, float velocity)
{
    // update values
    m_params.keyDown(voiceId, velocity);
    // handle preload
    if(m_params.m_preload == 0)
    {
        // direct key apply
        m_params.keyApplyMono();
        m_params.keyApply(voiceId);
        keyApply(voiceId);
    }
    else
    {
        // preload: increase PL counters
        m_params.m_event.m_mono.m_preload++;
        m_params.m_event.m_poly[voiceId].m_preload++;
    };
};

void voice_manager::keyApply(int voiceId)
{
    // keyDown events cause triggers to the audio_engine
    if(m_params.m_event.m_poly[voiceId].m_type == 1)
    {
        // determine note steal
        if(m_params.m_body[m_params.m_head[par_noteSteal].m_index].m_signal == 1)
        {
            m_synth.triggerVoiceSteal(m_signal[voiceId], voiceId);
        }
        else
        {
            m_synth.triggerNonVoiceSteal(m_signal[voiceId], voiceId);
        };
        // update and reset oscillator phases
        m_signal[voiceId][7] = m_params.m_body[m_params.m_head[24].m_index + voiceId].m_signal;   // POLY PhsA -> OscA Phase
        m_synth.resetOscillatorPhase(m_signal[voiceId], voiceId);
    };
};

void voice_manager::getStatus()
{
    std::cout << "voice_manager_status: (signal)" << std::endl;
    for(int s = 0; s < sig_number_of_signal_items; s++)
    {
        std::cout << "signal " << s << " : ";
        for(int v = 0; v < dsp_number_of_voices; v++)
        {
            std::cout << m_signal[v][s] << ", ";
        };
        std::cout << std::endl;
    };
};

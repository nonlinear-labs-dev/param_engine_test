#include <math.h>
#include <param_engine.h>

void param_head::getStatus()
{
    std::cout << "param_head_status: (id: " << m_id << ", size: " << m_size << ", index: " << m_index;
    std::cout << ", clockType: " << m_clockType << ", polyType: " << m_polyType;
    std::cout << ", normalize: " << m_normalize << ", scaleId: " << m_scaleId;
    std::cout << ", scaleArg: " << m_scaleArg << ")" << std::endl;
};

void param_body::getStatus()
{
    std::cout << "param_body_status: (state: " << m_state << ", preload: " << m_preload << ", signal: " << m_signal;
    std::cout << ", dx: [" << m_dx[0] << ", " << m_dx[1] << "], x: " << m_x;
    std::cout << ", start; " << m_start << ", diff: " << m_diff << ", dest: " << m_dest << ")" << std::endl;
};

void param_engine::init()
{
    // initialize variables
    m_preload = 0;
    // define time handling according to different clock types (sync, audio, fast, slow)
    m_timeFactors[0] = 0; // sync params lack time sensivity
    m_timeFactors[1] = 1; // audio params require sample-accurate time, no conversion
    m_timeFactors[2] = static_cast<float>(dsp_clock_fast_division); // fast rate conversion
    m_timeFactors[3] = static_cast<float>(dsp_clock_slow_division); // slow rate conversion
    // reset id_lists
    //m_poly_ids.reset();
    m_clock_ids.reset();
    m_post_ids.reset();
    // initialize other components
    m_convert.init();
    int i, p;
    for(i = 0; i < sig_number_of_env_events; i++)
    {
        m_event.m_env[i].init();
    };
    // initialize parameter configuration by header (setting param_head components)
    i = 0;  // index pointer
    for(p = 0; p < sig_number_of_params; p++)
    {
        // parameter declaration
        m_head[p].m_index = i;
        m_head[p].m_id = param_definition[p][0];
        m_head[p].m_clockType = param_definition[p][1];
        m_head[p].m_polyType = param_definition[p][2];
        m_head[p].m_size = m_routePolyphony[m_head[p].m_polyType];
        m_head[p].m_normalize = 1.f / param_definition[p][3];
        m_head[p].m_scaleId = param_definition[p][4];
        m_head[p].m_scaleArg = param_definition[p][5];
        m_head[p].m_postId = param_definition[p][6];
        // add param (internal) id to id_lists
        //m_poly_ids.add(m_head[p].m_polyType, m_head[p].m_id); // poyl IDs seem obsolete...
        m_clock_ids.add(m_head[p].m_clockType, m_head[p].m_polyType, p);
        if(m_head[p].m_postId > -1)
        {
            // add param id to post ids (if postId > -1)
            m_post_ids.add(m_head[p].m_clockType, m_head[p].m_polyType, p);
        };
        // update item pointer
        i += m_head[p].m_size;
    };
    // initialize utility parameters
    for(i = 0; i < sig_number_of_utilities; i++)
    {
        m_utilities[i].m_normalize = 1.f / utility_definition[i][0];
        m_utilities[i].m_scaleId = utility_definition[i][1];
        m_utilities[i].m_scaleArg = utility_definition[i][2];
    };
    // initialize envelopes (pass gate release time - currently constant)
    m_envelopes.init(1 / ((1 * m_millisecond) + 1));   // gate release is 1 millisecond
};

float param_engine::scale(int scaleId, float scaleArg, float value)
{
    // tcd scale methods (currently 12 established, one experimental)
    float result;
    switch(scaleId)
    {
    case 0:
        // bi-linear, offset
        result = value + scaleArg;
        break;
    case 1:
        // bi-linear, factor
        result = value * scaleArg;
        break;
    case 2:
        // inv-linear, offset
        result = scaleArg - value;
        break;
    case 3:
        // bi-parabolic, offset
        result = (abs(value) * value) + scaleArg;
        break;
    case 4:
        // bi-parabolic, factor
        result = abs(value) * value * scaleArg;
        break;
    case 5:
        // bi-cubic, offset
        result = (value * value * value) + scaleArg;
        break;
    case 6:
        // inv-s-curve, offset
        value = (2 * (scaleArg - value)) - 1;
        result = ((value * value * value) - 0.25) + (0.75 * value) + 0.5;
        break;
    case 7:
        // expon-gain, offset
        result = m_convert.eval_level(value + scaleArg);
        break;
    case 8:
        // expon-osc-pitch, offset (oscillators only)
        result = m_convert.eval_osc_pitch(value + scaleArg);
        break;
    case 9:
        // expon-linear-pitch, offset (other pitches)
        result = m_convert.eval_lin_pitch(value + scaleArg);
        break;
    case 10:
        // expon-drive, factor
        result = (m_convert.eval_level(value) * scaleArg) - scaleArg;
        break;
    case 11:
        // expon-mix-drive, factor
        result = m_convert.eval_level(value) * scaleArg;
        break;
    case 12:
        // expon-env-time, offset
        result = m_convert.eval_time((value * dsp_expon_time_factor) + scaleArg) * dsp_upsample_factor; // aware of sampling-rate
        break;
    case 13:
        // experimental: squared curvature (scaleArg = curvature) (value and scaleArg should be [-1 .. 1])
        result = m_envelopes.squaredCurvature(value, scaleArg);
    };
    // return scaled value
    return result;
};

void param_engine::setDx(int voiceId, int paramId, float value)
{
    // body pointer
    int i = m_head[paramId].m_index + voiceId;
    // convert dx argument to clock type (clip to 1 - max)
    value *= m_timeFactors[m_head[paramId].m_clockType];
    if(value > 1) {
        value = 1;
    };
    // pass dx argument (0 ... 1) to body item
    m_body[i].m_dx[0] = value;
};

void param_engine::setDest(int voiceId, int paramId, float value)
{
    // body pointer
    int i = m_head[paramId].m_index + voiceId;
    // normalize and scale dest argument, pass result to body item
    value *= m_head[paramId].m_normalize;
    m_body[i].m_dest = scale(m_head[paramId].m_scaleId, m_head[paramId].m_scaleArg, value);
    // if sync-type, apply to signal, else apply according to preload
    if(m_preload == 0) {
        if(m_head[paramId].m_clockType > 0) {
            applyDest(i);
        } else {
            applySync(i);
        };
    } else {
        m_body[i].m_preload++;
    };
};

void param_engine::applyDest(int index)
{
    // construct segment and set state flag
    m_body[index].m_start = m_body[index].m_signal;
    m_body[index].m_diff = m_body[index].m_dest - m_body[index].m_start;
    m_body[index].m_x = m_body[index].m_dx[1] = m_body[index].m_dx[0];
    m_body[index].m_state = 1;
    // update signal?? - ensure that signal is present (even before clock tick)
    //m_body[index].m_signal = m_body[index].m_start + (m_body[index].m_diff * m_body[index].m_x);
};

void param_engine::applySync(int index)
{
    // just update signal
    m_body[index].m_signal = m_body[index].m_dest;
};

void param_engine::applyPreloaded(int voiceId, int paramId)
{
    // body pointer
    int i = m_head[paramId].m_index + voiceId;
    // check preload status
    if(m_body[i].m_preload > 0) {
        m_body[i].m_preload = 0;
        // route by clockType (non-sync or sync, reason to pass paramId down here..)
        if(m_head[paramId].m_clockType > 0) {
            applyDest(i);
        } else {
            applySync(i);
        };
    };
};

void param_engine::tickItem(int index)
{
    if(m_body[index].m_state == 1) {
        // stop segment on final sample
        if(m_body[index].m_x >= 1) {
            m_body[index].m_x = 1;
            m_body[index].m_state = 0;
        };
        // update signal (and x)
        m_body[index].m_signal = m_body[index].m_start + (m_body[index].m_diff * m_body[index].m_x);
        m_body[index].m_x += m_body[index].m_dx[1];
    };
};

void param_engine::keyUp(int voiceId, float velocity)
{
    velocity = scale(m_utilities[0].m_scaleId, m_utilities[0].m_scaleArg, velocity * m_utilities[0].m_normalize);
    // update values only, preload handling by parent (voice_manager)
    m_event.m_mono.m_velocity = velocity;           // set mono velocity (always last velocity)
    m_event.m_mono.m_type = 0;                      // set mono type (0: up)
    m_event.m_poly[voiceId].m_velocity = velocity;  // set poly velocity (at voiceId)
    m_event.m_poly[voiceId].m_type = 0;             // set poly type (0: up, at voiceId)
};

void param_engine::keyDown(int voiceId, float velocity)
{
    velocity = scale(m_utilities[0].m_scaleId, m_utilities[0].m_scaleArg, velocity * m_utilities[0].m_normalize);
    // update values only, preload handling by parent (voice_manager)
    m_event.m_mono.m_velocity = velocity;           // set mono velocity (always last velocity)
    m_event.m_mono.m_type = 1;                      // set mono type (1: down)
    m_event.m_poly[voiceId].m_velocity = velocity;  // set poly velocity (at voiceId)
    m_event.m_poly[voiceId].m_type = 1;             // set poly type (1: down, at voiceId)
};

void param_engine::keyApply(int voiceId)
{
    // apply key event: update envelopes according to event type (down, up)
    float pitch = m_body[m_head[par_notePitch].m_index + voiceId].m_signal;
    float velocity = m_event.m_poly[voiceId].m_velocity;
    if(m_event.m_poly[voiceId].m_type == 0)
    {
        // keyUp:
        envUpdateStop(voiceId, 0, pitch, velocity);     // Envelope A
        // envUpdateStop(voiceId, 1, pitch, velocity);
        // envUpdateStop(voiceId, 2, pitch, velocity);
        m_envelopes.stopEnvelope(voiceId, 3);           // Gate
    }
    else
    {
        // keyDown:
        envUpdateStart(voiceId, 0, pitch, velocity);    // Envelope A
        // envUpdateStart(voiceId, 1, pitch, velocity);
        // envUpdateStart(voiceId, 2, pitch, velocity);
        m_envelopes.startEnvelope(voiceId, 3);          // Gate
    };
};

void param_engine::keyApplyMono()
{
    // segment dest update of mono flanger env (if keyDown) (velocity)
    if(m_event.m_mono.m_type == 1)
    {
        // keyDown (mono flanger env only handles keyDown)
        m_envelopes.setSegmentDest(0, 4, 1, m_event.m_mono.m_velocity);
        m_envelopes.startEnvelope(0, 4);
    };
};

void param_engine::envUpdateStart(int voiceId, int envId, float pitch, float velocity)
{
    // envelope index pointer, segment parameters:
    int envIndex = m_envIds[envId];
    float time, dest;
    // event parameters:
    float timeKT = m_body[m_head[envIndex + 11].m_index].m_signal * pitch;
    float levelVel = m_body[m_head[envIndex + 7].m_index].m_signal;
    float attackVel = -m_body[m_head[envIndex + 8].m_index].m_signal * velocity;
    float levelKT = m_body[m_head[envIndex + 10].m_index].m_signal * pitch;
    float peak = fmin(m_convert.eval_level((-(1 - velocity) * levelVel * env_norm_peak) + levelKT), env_clip_peak);
    // event updates:
    m_event.m_env[envId].m_levelFactor[voiceId] = peak;
    m_event.m_env[envId].m_timeFactor[voiceId][0] = m_convert.eval_level(timeKT + attackVel) * m_millisecond;
    m_event.m_env[envId].m_timeFactor[voiceId][1] = m_convert.eval_level(timeKT) * m_millisecond;
    m_event.m_env[envId].m_timeFactor[voiceId][2] = m_event.m_env[envId].m_timeFactor[voiceId][1];
    // segment updates:
    // - attack: time, dest
    time = m_body[m_head[envIndex].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][0];
    m_envelopes.setSegmentDx(voiceId, envId, 1, 1 / (time + 1));
    m_envelopes.setSegmentDest(voiceId, envId, 1, peak);
    // - decay1: time, dest (breakpoint level)
    time = m_body[m_head[envIndex + 1].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][1];
    m_envelopes.setSegmentDx(voiceId, envId, 2, 1 / (time + 1));
    dest = peak * m_body[m_head[envIndex + 2].m_index].m_signal;
    m_envelopes.setSegmentDest(voiceId, envId, 2, dest);
    // - decay2: time, dest (sustain level)
    time = m_body[m_head[envIndex + 3].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][2];
    m_envelopes.setSegmentDx(voiceId, envId, 3, 1 / (time + 1));
    dest = peak * m_body[m_head[envIndex + 4].m_index].m_signal;
    m_envelopes.setSegmentDest(voiceId, envId, 3, dest);
    // trigger envelope start:
    m_envelopes.startEnvelope(voiceId, envId);
};

void param_engine::envUpdateStop(int voiceId, int envId, float pitch, float velocity)
{
    // envelope index pointer, segment parameters
    int envIndex = m_envIds[envId];
    float time;
    // event parameters:
    float timeKT = m_body[m_head[envIndex + 11].m_index].m_signal * pitch;
    float releaseVel = -m_body[m_head[envIndex + 9].m_index].m_signal * velocity;
    // event updates:
    m_event.m_env[envId].m_timeFactor[voiceId][3] = m_convert.eval_level(timeKT + releaseVel) * m_millisecond;
    // segment updates:
    // - release: time only (infinity check!)
    if(m_body[m_head[envIndex + 5].m_index].m_signal > m_highest_finite_time)
    {
        // infinite release time
        m_envelopes.setSegmentDx(voiceId, envId, 4, 0);
    }
    else
    {
        // finite release time
        time = m_body[m_head[envIndex + 5].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][3];
        m_envelopes.setSegmentDx(voiceId, envId, 4, 1 / (time + 1));
    };
    // trigger envelope stop:
    m_envelopes.stopEnvelope(voiceId, envId);
};

void param_engine::envUpdateTimes(int voiceId, int envId)
{
    // envelope index pointer, segment parameters
    int envIndex = m_envIds[envId];
    float time;
    // segment updates:
    // - attack time
    time = m_body[m_head[envIndex].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][0];
    m_envelopes.setSegmentDx(voiceId, envId, 1, 1 / (time + 1));
    // - decay1 time
    time = m_body[m_head[envIndex + 1].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][1];
    m_envelopes.setSegmentDx(voiceId, envId, 2, 1 / (time + 1));
    // - decay2 time
    time = m_body[m_head[envIndex + 3].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][2];
    m_envelopes.setSegmentDx(voiceId, envId, 3, 1 / (time + 1));
    // - release time (infinity check!)
    if(m_body[m_head[envIndex + 5].m_index].m_signal > m_highest_finite_time)
    {
        // infinite release time
        m_envelopes.setSegmentDx(voiceId, envId, 4, 0);
    }
    else
    {
        // finite release time
        time = m_body[m_head[envIndex + 5].m_index].m_signal * m_event.m_env[envId].m_timeFactor[voiceId][3];
        m_envelopes.setSegmentDx(voiceId, envId, 4, 1 / (time + 1));
    };
};

void param_engine::envUpdateLevels(int voiceId, int envId)
{
    // envelope index pointer, segment parameters
    int envIndex = m_envIds[envId];
    // peak level reference
    float peak = m_event.m_env[envId].m_levelFactor[voiceId];
    // segment updates:
    // - breakpoint level
    m_envelopes.setSegmentDest(voiceId, envId, 2, peak * m_body[m_head[envIndex + 2].m_index].m_signal);
    // - sustain level
    m_envelopes.setSegmentDest(voiceId, envId, 3, peak * m_body[m_head[envIndex + 4].m_index].m_signal);
};

void param_engine::getStatus()
{
    std::cout << "param_engine_status: (" << std::endl;
    for(int p = 0; p < sig_number_of_params; p++)
    {
        m_head[p].getStatus();
        for(int i = 0; i < m_head[p].m_size; i++)
        {
            m_body[m_head[p].m_index + i].getStatus();
        };
    };
    std::cout << ")" << std::endl;
};

/*
 * PARAM_ENGINE prototype for C15 Linux Renderer (Version 1.5)
 * - definitions of component variables and data structures (which should be implemented into the current voice_manager)
 *
 * - status:    developing MATURE (establish and test event and env parameters)
 * - notes:     when establishing and testing key events, increase number of predefined params and test post processing as well
 *
 * - so far established:
 *  - tcd midi input: V, VM, P, PM, T, TU, TL, D, DS, DU, DL, PL, FL, U, UD - tested, working
 *  - tcd midi input: KD, KU                                                - established, not tested (parameters missing for evaluations)
 *  - tcd select ALL (VA, PA):                                              - implemented due to global time updates
 *  - tcd scale methods: (pre processing: 13 implemented), squared curvature and exponential conversions - tested, working
 *  - parameters: definition, clockTypes, polyTypes, (linear) rendering - tested (including preload), working
 *  - post processing: main functions (and simple explicit implementation) - tested, working
 *      - post processing functions are reduced to one function per clock type (except, of course, sync) and voice iteration
 *  - tcd lists: (recall, key event) - predefined paramId lists for simplified TCD update sequences - tested, working
 *      - updated preload behavior: tcd_command preload(listId, preloadMode)
 *  - tcd utilities: define velocity parameter (range, scaling), reference tone (and possibly more global/mono sync config params)
 *      - new tcd commands: selectUtility(unsigned14 id), setUtility(unsigned14 value)
 *      - currently defining velocity parameter, reference tone (440 Hz)
 *      - utility candidates: global times (edit, macros[], recall, morph), setSampleRate?
 *  - tcd flush mechanism (audio engine flush trigger, wait mechanism for fadepoint, fadepoint part of signal array) - tested, working
 *      - rendered by fast clock, taking 30 ticks at 48kHz (meaning 150 samples = 3.125 ms)
 *      - fadepoint and flush are monophonic, similar to the Reaktor Renderer
 *  - envelopes: definition, rendering mechanism established - tested for adbdsr, working
 *      - envelope rendering will be accomplished within the audio post processing function
 *      - mechanism established, éxcept for parameter-relevant calculations (updating time- and level- factors and envelope parameters - bigger session needed)
 *
 * - todo:
 *  - tcd midi input: KEYUP, KEYDOWN, later (MB, MP)                                            - add NEW to SPECS !!! - KU, KD established
 *      - time updates need to be sample rate aware                                             - done
 *      - create utility mechanism (requires predefinition of utilities)                        - done
 *      - create list mechanism (pre-defined paramId lists for recall and keyEvent updates)     - done
 *      - create key event data structure (polyphonic), catch latest velocity for mono vel      - done
 *      - create envelope generator within param engine                                         - done
 *  - envelope generator (within param_engine component, mono and poly rendering!)              - done, tested for adbdsr
 *  - post processing optimization: collect all param ids that perform mere copy                - done
 *      - requires at least predefined post_id                                                  - todo
 *      - reduced to three clock-type-based functions:                                          - established
 *          - audio (params, env), fast (params, fadepoint), slow (params)
 *      - parameters either:                                                                    - established
 *          - distribute their values to shared array (mono params)
 *          - or values are copied (poly params)
 *  - detections? (NaN)                                                                         - talked about, for later (including possible MIDI response messages)
 *
 * - notable behavior:
 *  - changing the sample rate requires compilation of the whole program (currently) - (stephan, anton, pascal) !!!
 *      - practiceable? system/program restart possible (with args)?
 *      - or delete and init necessary structures instead?
 *  - voice and param selection currently select_all cases (value == 16383), are they needed (stephan) ???
 *  - parameters of type sync (no transitions) lack post processing (and therefore can't trigger anything)
 *  - param engine should be configured as the current reaktor rendering engine (mostly MONO parameters, except for Key Events)
 *      - !!! however, post processing should spread mono signals into polyphonic shared ARRAY (easy way for audio_engine) !!!
 *  - fade point handling by param_engine is accomplished by fast rendering (reducing cpu while maintaining almost accurate timing)
 *
 * */

/*      CHANGES since 02c:
 *          - Voice Spread added to Param Definition
 *          - Flush removed (realized in audio_engine), only trigger is passed
 *          - Parameter Set expanded to: Env A (+ Gate) (13), Osc A (7 basic), Master (2), Poly (5)
 *      TODO:
 *          - voice spread mechanism (id_lists)                 ->      new id_list format in order to force mono params (fx)
 *          - trigger functions: direct or per voice/clock?     ->      one per clock per voice
 *
 *          - pseudo TCD input (MIDI Notes and CC to several TCD params, events)
 *          - code optimization: dereferencing when possible (copy method)
 *          - implementation in main project
 *
 *      STATUS:
 *          - simple project seems to be working (10 shared signals, updates in post proc, triggers missing)
 * */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <iostream>
#include <voice_manager.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// debug scenarios:

#define tcdKeyDownSequence 1
#define debugStatus 1

// session body: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    // declaration and init
    std::cout << "\nSession Start (Init voice_manager)\n" << std::endl;
    voice_manager host;
    host.init();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    host.tickMain();
    host.m_params.getStatus();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if tcdKeyDownSequence
    // tcd global time update
    std::cout << "\nGlobal Time Update:\n" << std::endl;
    host.evalMidiEvent(0, 127, 127);    // VA
    host.evalMidiEvent(1, 127, 127);    // PA
    host.evalMidiEvent(2, 0, 0);        // T 0
    // tcd prepare parameters (using recall id list)
    std::cout << "\nRecall Sequence:\n" << std::endl;
    host.evalMidiEvent(47, 1, 1);       // PL 1, 1
    host.evalMidiEvent(5, 1, 32);       // EnvA Attack (160 = 0.113 ms = 5.4 samples)
    host.evalMidiEvent(5, 0, 100);      // EnvA Decay1
    host.evalMidiEvent(5, 125, 0);      // EnvA Breakpoint (100%)
    host.evalMidiEvent(5, 50, 100);     // EnvA Decay2
    host.evalMidiEvent(5, 100, 100);    // EnvA Sustain
    //host.evalMidiEvent(5, 125, 0);      // EnvA Release (16000 = highest finite release time)
    host.evalMidiEvent(5, 31, 32);      // EnvA Release (4000 ~2ms)
    host.evalMidiEvent(5, 0, 0);        // EnvA Gain
    host.evalMidiEvent(5, 0, 10);       // EnvA Level Vel
    host.evalMidiEvent(5, 0, 0);        // EnvA Attack Vel
    host.evalMidiEvent(5, 0, 10);       // EnvA Relase Vel
    host.evalMidiEvent(5, 0, 10);       // EnvA Level KT
    host.evalMidiEvent(5, 0, 0);        // EnvA Time KT
    host.evalMidiEvent(5, 0, 0);        // EnvA Attack Curve
    // osc a, master params
    host.evalMidiEvent(5, 62, 64);      // OscA Pitch (60 ST)
    host.evalMidiEvent(5, 78, 16);      // OscA Pitch KT (100 %)
    host.evalMidiEvent(5, 0, 0);        // OscA Fluct
    host.evalMidiEvent(5, 0, 0);        // OscA PM Self
    host.evalMidiEvent(5, 0, 0);        // OscA PM Self Env A
    host.evalMidiEvent(5, 93, 96);      // OscA Chirp (12000 = 140 ST)
    host.evalMidiEvent(5, 62, 64);      // Master Volume (8000 = 0 dB)
    host.evalMidiEvent(5, 9, 48);       // Master Tune (1200 = +12 ST)
    host.evalMidiEvent(47, 0, 2);       // Apply Preloaded Values
    // mono flanger env pre-definition (currently no flanger decay param)
    host.m_params.m_envelopes.setSegmentDx(0, 4, 2, 0.01);  // currently only 'dirty' hack...
    // some time passing
    for(int i = 0; i < 200; i++)
    {
        host.tickMain();
    };
    // tcd keydown sequence (using key event id list)
    std::cout << "\nKey Down Sequence:\n" << std::endl;
    host.evalMidiEvent(47, 2, 1);       // PL 2, 1
    host.evalMidiEvent(0, 0, 2);        // V 2
    host.evalMidiEvent(5, 0, 0);        // steal 0
    host.evalMidiEvent(5, 93, 96);      // pitch (+12 ST)
    host.evalMidiEvent(5, 0, 0);        // voice pan
    host.evalMidiEvent(5, 56, 32);      // phase a (7200 = 0.5)
    host.evalMidiEvent(5, 0, 0);        // phase b
    host.evalMidiEvent(23, 32, 0);      // key down (full range: 4096 - for now)
    /*
    host.evalMidiEvent(0, 0, 3);        // V 3
    host.evalMidiEvent(5, 0, 0);        // steal 0
    host.evalMidiEvent(5, 2, 0);        // pitch
    host.evalMidiEvent(5, 0, 0);        // voice pan
    host.evalMidiEvent(5, 0, 0);        // phase a
    host.evalMidiEvent(5, 0, 0);        // phase b
    host.evalMidiEvent(23, 2, 0);       // key down
    */
    host.evalMidiEvent(47, 0, 2);       // Apply Preloaded Values
    host.m_params.getStatus();
    // some time passing
    for(int i = 0; i < 25; i++)
    {
        host.tickMain();
    };
    //
    host.evalMidiEvent(7, 0, 0);        // key up (no preload currently)
    // some time passing
    for(int i = 0; i < 250; i++)
    {
        host.tickMain();
    };
#endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if debugStatus
    //host.m_params.getStatus();
    host.getStatus();
    /*
    //host.m_params.m_poly_ids.getStatus();     poly ids seem obsolete...
    host.m_params.m_clock_ids.getStatus();
    host.m_params.m_post_ids.getStatus();
    */
#endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    std::cout << "\nSession End (bye bye)\n" << std::endl;
    return 0;
}

// END - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

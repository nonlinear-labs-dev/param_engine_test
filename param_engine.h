#ifndef PARAM_ENGINE_H
#define PARAM_ENGINE_H

#include <utils.h>
#include <global_defines_params.h>
#include <exponentiator.h>
#include <env_engine.h>
#include <key_event.h>

struct param_head
{
    //
    int m_id;
    int m_index;
    int m_size;
    int m_clockType;
    int m_polyType;
    int m_scaleId;
    int m_postId;
    float m_normalize;
    float m_scaleArg;
    //
    void getStatus();
};

struct param_body
{
    //
    int m_state = 0;
    int m_preload = 0;
    float m_signal = 0;
    float m_dx[2] = {0, 0};
    float m_x = 0;
    float m_start = 0;
    float m_diff = 0;
    float m_dest = 0;
    //
    void getStatus();
};

struct param_utility
{
    int m_scaleId;
    float m_normalize;
    float m_scaleArg;
};

struct param_engine
{
    // local variables
    int m_preload;
    //float m_fadepoint;
    // - variable pitch reference frequency for A3 note (init to 440 Hz), fed by utility (range: [400, 480]) (utilities auto-update)
    float m_pitch_reference = dsp_init_pitch_reference;
    // - nyquist frequency (depending on sample rate)
    const float m_nyquist_frequency = static_cast<float>(dsp_sample_rate >> 1);
    const float m_millisecond = dsp_samples_to_ms * static_cast<float>(dsp_sample_rate);
    const float m_highest_finite_time = dsp_upsample_factor * 16000.f;
    // - clock and polyphony definitions
    float m_timeFactors[dsp_clock_types];
    const int m_routePolyphony[2] = {1, dsp_number_of_voices};
    // envelope parameter indeces
    const int m_envIds[sig_number_of_env_events] = {par_envelopeA, par_envelopeB, par_envelopeC};
    // local data structures
    clock_id_list m_clock_ids;                              // sorted param ids for rendering
    dual_clock_id_list m_post_ids;                          // sorted param ids for automatic post processing (including spread capability)
    param_head m_head[sig_number_of_params];                // parameter definition items
    param_body m_body[sig_number_of_param_items];           // parameter rendering items
    exponentiator m_convert;                                // exponential scaling
    param_utility m_utilities[sig_number_of_utilities];     // velocity, referenceTone definition
    env_engine m_envelopes;                                 // envelope performance (rendering in audio post processing)
    poly_key_event m_event;                                 // key event (holding pitches, velocites and env segment factors)
    // proper init
    void init();
    // helper functions
    float scale(const int scaleId, const float scaleArg, float value);  // provided tcd scale functions
    // tcd mechanism
    void setDx(const int voiceId, const int paramId, float value);      // param dx update
    void setDest(const int voiceId, const int paramId, float value);    // param dest update
    void applyPreloaded(const int voiceId, const int paramId);          // param apply preloaded
    void applyDest(const int index);                                    // param apply dest (non-sync types)
    void applySync(const int index);                                    // param apply dest (sync types)
    // rendering
    void tickItem(const int index);                                     // parameter rendering
    // simplified post processing approach (one function per clock)
    void postProcess_slow(float *signal, const int voiceId);            // slow post processing (distribution, copy, env c event signal!)
    void postProcess_fast(float *signal, const int voiceId);            // fast post processing (distribution, copy)
    void postProcess_audio(float *signal, const int voiceId);           // audio post processing (distribution, copy, envelopes, param combination)
    // key events
    void keyDown(const int voiceId, float velocity);                    // key events: key down (note on) mechanism
    void keyUp(const int voiceId, float velocity);                      // key events: key up (note off) mechanism
    void keyApply(const int voiceId);                                   // key events: apply key event
    void keyApplyMono();                                                // key events: apply mono event
    // envelope updates
    void envUpdateStart(const int voiceId, const int envId, const float pitch, const float velocity);
    void envUpdateStop(const int voiceId, const int envId, const float pitch, const float velocity);
    void envUpdateTimes(const int voiceId, const int envId);
    void envUpdateLevels(const int voiceId, const int envId);
    // debug
    void getStatus();
};

#endif // PARAM_ENGINE_H

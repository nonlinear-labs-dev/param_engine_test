#include <math.h>
#include <param_engine.h>

void param_engine::postProcess_slow(float *signal, int voiceId)
{
    // automatic post processing distribution pointers
    int i;
    int p;
    // automatic mono distribution:
    for(i = 0; i < m_post_ids.m_data[3].m_data[0].m_length; i++)
    {
        p = m_head[m_post_ids.m_data[3].m_data[0].m_data[i]].m_postId;
        signal[p] = m_body[m_head[m_post_ids.m_data[3].m_data[0].m_data[i]].m_index].m_signal;
    };
    // automatic poly copy:
    for(i = 0; i < m_post_ids.m_data[3].m_data[1].m_length; i++)
    {
        p = m_head[m_post_ids.m_data[3].m_data[1].m_data[i]].m_postId;
        signal[p] = m_body[m_head[m_post_ids.m_data[3].m_data[1].m_data[i]].m_index + voiceId].m_signal;
    };
    // update envelope times (A, B, C)
    envUpdateTimes(voiceId, 0);     // Envelope A
    // envUpdateTimes(voiceId, 1);     // Envelope B
    // envUpdateTimes(voiceId, 2);     // Envelope C
    // LATER: Envelope C (Event) Signal at slow clock (?)
    // determine oscA_freq (base pitch, master tune, key tracking, osc pitch, envC missing) and update signal:
    float basePitch = m_body[m_head[par_notePitch].m_index + voiceId].m_signal + m_body[m_head[20].m_index].m_signal;
    float keyTracking = m_body[m_head[14].m_index].m_signal;
    float oscPitch = m_body[m_head[13].m_index].m_signal;
    signal[4] = m_pitch_reference * oscPitch * m_convert.eval_lin_pitch(69 + (basePitch * keyTracking));    // OscA Freq (Hz)
    // determine oscA_chirpFreq and update signal:
    signal[8] = m_body[m_head[18].m_index].m_signal * 440.f;    // Osc A Chirp Filter Frequency (Hz)
};

void param_engine::postProcess_fast(float *signal, int voiceId)
{
    // automatic post processing distribution pointers
    int i;
    int p;
    // automatic mono distribution:
    for(i = 0; i < m_post_ids.m_data[2].m_data[0].m_length; i++)
    {
        p = m_head[m_post_ids.m_data[2].m_data[0].m_data[i]].m_postId;
        signal[p] = m_body[m_head[m_post_ids.m_data[2].m_data[0].m_data[i]].m_index].m_signal;
    };
    // automatic poly copy:
    for(i = 0; i < m_post_ids.m_data[2].m_data[1].m_length; i++)
    {
        p = m_head[m_post_ids.m_data[2].m_data[1].m_data[i]].m_postId;
        signal[p] = m_body[m_head[m_post_ids.m_data[2].m_data[1].m_data[i]].m_index + voiceId].m_signal;
    };
    // update envelope levels
    envUpdateLevels(voiceId, 0);    // Envelope A
    // envUpdateLevels(voiceId, 1);
    // envUpdateLevels(voiceId, 2);
    // ??? further calculations on parameters and envelope signals
};

void param_engine::postProcess_audio(float *signal, int voiceId)
{
    // automatic post processing distribution pointers
    int i;
    int p;
    // automatic mono distribution:
    for(i = 0; i < m_post_ids.m_data[1].m_data[0].m_length; i++)
    {
        p = m_head[m_post_ids.m_data[1].m_data[0].m_data[i]].m_postId;
        signal[p] = m_body[m_head[m_post_ids.m_data[1].m_data[0].m_data[i]].m_index].m_signal;
    };
    // automatic poly copy:
    for(i = 0; i < m_post_ids.m_data[1].m_data[1].m_length; i++)
    {
        p = m_head[m_post_ids.m_data[1].m_data[1].m_data[i]].m_postId;
        signal[p] = m_body[m_head[m_post_ids.m_data[1].m_data[1].m_data[i]].m_index + voiceId].m_signal;
    };
    // update envelope-relevant params and pass information to envelopes
    // render envelopes
    if(voiceId == 0)
    {
        m_envelopes.tickMono(); // only voiceId 0 performs mono tick
    };
    m_envelopes.tickPoly(voiceId); // poly envelope tick for all voices
    // envelope distribution:
    signal[0] = m_envelopes.m_body[m_envelopes.m_head[0].m_index + voiceId].m_signal;   // Env A
    signal[1] = m_envelopes.m_body[m_envelopes.m_head[1].m_index + voiceId].m_signal;   // Env B
    signal[2] = m_envelopes.m_body[m_envelopes.m_head[2].m_index + voiceId].m_signal;   // Env C
    signal[3] = m_envelopes.m_body[m_envelopes.m_head[3].m_index + voiceId].m_signal;   // Gate
    // further calculations on parameters and envelope signals
    // osc a pm self (env a)
    float pm_amt = m_body[m_head[16].m_index].m_signal;
    float pm_env = m_body[m_head[17].m_index].m_signal;
    signal[6] = ((signal[0] * pm_env) + (1 - pm_env)) * pm_amt;
};

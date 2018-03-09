#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <global_defines_config.h>

// audio engine dummy
struct audio_engine
{
    // proper init
    void init();                                            // (if needed)
    // rendering
    void renderPolySection(float *signal, int voiceId);     // rendering of the polyphonic synth section
    void renderMonoSection(float *signal);                  // rendering of the monophonic synth (effects and main out) section
    // triggers
    void flushBuffer();                                     // monophonic buffer clearance (all voices)
    void resetOscillatorPhase(float *signal, int voiceId);  // set osc phase according to parameter (signal array)
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // !!! probably vital triggers (previously not discussed)
    /*
    void trigEnvelopeC(int voiceId, float value);           // event-based Envelope C signal (Comb Filter Coeff calc, slow clock)
    void trigFiltCoeffs(int voiceId, float frq, float res); // example of trigger for filter coefficients (usually slow clock)
    */
    void triggerEnvC(float *signal, int voiceId);           // triggers passing array as well
    void triggerFilterCoeffs(float *signal, int voiceId);
    void triggerVoiceSteal(float *signal, int voiceId);
    void triggerNonVoiceSteal(float *signal, int voiceId);
    // instead
    // triggerFast(*signal, voiceId) -> calc osc dx, calc filter coefficients
    // triggerSlow/*signal, voiceId)
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // candidates
    int finalSignalState(int voiceId);                      // NaN detection? (at least of MONO output)
    // debug
    void getStatus();
};

#endif // AUDIO_ENGINE_H

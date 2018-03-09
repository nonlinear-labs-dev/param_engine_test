#include <audio_engine.h>

void audio_engine::init()
{
    //
};

void audio_engine::renderPolySection(float *signal, int voiceId)
{
    // avoid compilation issues by pseudo-using variables
    signal[0] *= 1;
    voiceId *= 1;
};

void audio_engine::renderMonoSection(float *signal)
{
    // avoid compilation issues by pseudo-using variables
    signal[0] *= 1;
};

void audio_engine::flushBuffer()
{
    //
    std::cout << "\nvoice_manager::audio_engine:\tFLUSH\n" << std::endl;
};

void audio_engine::resetOscillatorPhase(float *signal, int voiceId)
{
    //
    std::cout << "\nvoice_manager::audio_engine:\tresetOscPhase(voiceId: " << voiceId << ")\n" << std::endl;
    signal[0] *= 1;
};

void audio_engine::triggerEnvC(float *signal, int voiceId)
{
    // avoid compilation issues by pseudo-using variables
    signal[0] *= 1;
    voiceId *= 1;
};

void audio_engine::triggerFilterCoeffs(float *signal, int voiceId)
{
    // avoid compilation issues by pseudo-using variables
    signal[0] *= 1;
    voiceId *= 1;
};

void audio_engine::triggerNonVoiceSteal(float *signal, int voiceId)
{
    // avoid compilation issues by pseudo-using variables
    signal[0] *= 1;
    voiceId *= 1;
    std::cout << "voice_manager::audio_engine:non-voice-steal(" << voiceId << ")" << std::endl;
};

void audio_engine::triggerVoiceSteal(float *signal, int voiceId)
{
    // avoid compilation issues by pseudo-using variables
    signal[0] *= 1;
    voiceId *= 1;
    std::cout << "voice_manager::audio_engine:voice-steal(" << voiceId << ")" << std::endl;
};

int audio_engine::finalSignalState(int voiceId)
{
    // avoid compilation issues by pseudo-using variables and returning something
    return(0 * voiceId);
};

void audio_engine::getStatus()
{
    //
};

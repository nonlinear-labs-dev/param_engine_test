#ifndef VOICE_MANAGER_H
#define VOICE_MANAGER_H

#include <decoder.h>
#include <param_engine.h>
#include <audio_engine.h>

struct voice_manager
{
    // local variables
    int m_clockPosition[dsp_clock_types] = {0, 0, 0, 0};
    const int m_clockDivision[dsp_clock_types] = {0, 1, dsp_clock_fast_division, dsp_clock_slow_division};
    float m_ms_to_samples;
    float m_signal[dsp_number_of_voices][sig_number_of_signal_items];
    // local data structures
    decoder m_decoder;          // handle MIDI input
    param_engine m_params;      // engine holding rendering items for parameters, utilities, envelopes
    audio_engine m_synth;       // dummy catching triggers
    // public methods
    void init(); // (later: should be transferred to the nlaudio::voice_manager.constructor)
    void evalMidiEvent(int status, int data0, int data1);
    void tickMain();
    // tcd mechanism
    void voiceSelectionUpdate();
    void paramSelectionUpdate();
    void preloadUpdate(int mode, int listId);
    void destinationUpdate(float value);
    void timeUpdate(float value);
    void utilityUpdate(float value);
    void listUpdate(float dest);
    void keyUp(int voiceId, float velocity);
    void keyDown(int voiceId, float velocity);
    void keyApply(int voiceId);
    // debugging
    void getStatus();
};

#endif // VOICE_MANAGER_H

#ifndef EXPONENTIATOR_H
#define EXPONENTIATOR_H

#include <global_defines_config.h>

// exponentiator for all exponential stuff (pitches, levels, times, etc.)

struct exponentiator
{
    // table position variables
    int m_position_step;
    float m_position_fine;
    // constant parameters (offsets, bases, exponent scalings, hyperbolic floor parameters)
    const float m_freqExponent_offset = 69;                         // pitch reference offset (where resulting frequency factor equals 1)
    const float m_freqBase = 2;                                     // base for pitch to frequency conversion
    const float m_scaleFreqExponent = 1.f / 12;                     // exponent normalization for pitch conversion
    const float m_gainBase = 10;                                    // base for level to amplitude and time conversion
    const float m_scaleGainExponent = 1.f / 20;                     // exponent normalization for level and time conversion
    // two parameters needed to generate the hyperbolic floor function of oscillator pitches
    const float m_hyperfloor[2] = {(300.f / 13), (280.f / 13)};
    // constant input value ranges for table access
    const float m_linear_pitch_from = dsp_expon_lin_pitch_from;
    const float m_linear_pitch_to = dsp_expon_lin_pitch_range + dsp_expon_lin_pitch_from;
    const float m_oscillator_pitch_from = dsp_expon_osc_pitch_from;
    const float m_oscillator_pitch_to = dsp_expon_osc_pitch_range + dsp_expon_osc_pitch_from;
    const float m_level_from = dsp_expon_level_from;
    const float m_level_to = dsp_expon_level_range + dsp_expon_level_from;
    const float m_time_from = dsp_expon_time_from;
    const float m_time_to = dsp_expon_time_range + dsp_expon_time_from;
    // conversion tables (constructed on initialization)            // the additional table element (size + 1) prevents interpolation issues
    float m_linear_pitch_table[dsp_expon_lin_pitch_range + 1];      // linear pitch table: [-150, 150] semitones
    float m_oscillator_pitch_table[dsp_expon_osc_pitch_range + 1];  // nonlinear pitch table for oscillators: [-20, 130] semitones
    float m_level_table[dsp_expon_level_range + 1];                 // level conversion table: [-300, 100] decibel (first element is zero)
    float m_time_table[dsp_expon_time_range + 1];                   // time conversion table: [-20, 90] decibel (first element is zero)
    // proper init
    void init();                                                    // facilitate construction of all four conversion tables
    // hyperbolic osc pitch function
    float hyperfloor(float value);                                  // valid for osc pitch - semitones: [-19, 0]
    // value clipping
    float clip(float min, float max, float value);                  // clip value according to min and max parameters
    // update table position (after clipping)
    void setTablePos(float value);                                  // turn a valid (clipped) position into integer and fractional parts
    // interpolation
    float getInterpolated(float fade, float first, float second);   // perform linear interpolation (of two values, using fade parameter)
    // conversion methods
    float eval_lin_pitch(float value);                              // linear pitch conversion (into frequency factor)
    float eval_osc_pitch(float value);                              // oscillator pitch conversion (into frequency factor)
    float eval_level(float value);                                  // gain conversion (into amplitude factor)
    float eval_time(float value);                                   // time conversion (into milliseconds)
    // debuging
    void getStatus_lin_pitch();
    void getStatus_osc_pitch();
    void getStatus_level();
    void getStatus_time();
};

#endif // EXPONENTIATOR_H

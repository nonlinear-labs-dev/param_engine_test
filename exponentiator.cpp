#include <math.h>
#include <exponentiator.h>

void exponentiator::init()
{
    // declare array index and exponent variable for calculations
    int i;
    float exp;
    // construct linear pitch table
    for(i = 0; i <= dsp_expon_lin_pitch_range; i++)
    {
        // determine exponent of equation
        exp = (static_cast<float>(i) + m_linear_pitch_from - m_freqExponent_offset) * m_scaleFreqExponent;
        // determine power: (base^exonent) - neither nyquist nor reference tone are included
        m_linear_pitch_table[i] = pow(m_freqBase, exp);
    };
    // initialize oscillator pitch table floor as zero (first element)
    m_oscillator_pitch_table[0] = 0;
    // construct oscillator pitch table (hyperbolic floor function for second till 20th semitone)
    for(i = 1; i < 20; i++)
    {
        // hyperbolic floor part - determine exponent of equation
        exp = (hyperfloor(static_cast<float>(i) + m_oscillator_pitch_from) - m_freqExponent_offset) * m_scaleFreqExponent;
        // determine power
        m_oscillator_pitch_table[i] = pow(m_freqBase, exp);
    };
    // further construct remaining oscillator pitch table (linear for remaining semitones)
    for(i = 20; i <= dsp_expon_osc_pitch_range; i++)
    {
        // determine exponent of equation
        exp = (static_cast<float>(i) + m_oscillator_pitch_from - m_freqExponent_offset) * m_scaleFreqExponent;
        // determine power
        m_oscillator_pitch_table[i] = pow(m_freqBase, exp);
    };
    // initialize level table floor as zero (first element)
    m_level_table[0] = 0;
    // construct remaining level table
    for(i = 1; i <= dsp_expon_level_range; i++)
    {
        // determine exponent of equation
        exp = (static_cast<float>(i) + m_level_from) * m_scaleGainExponent;
        // determine power
        m_level_table[i] = pow(m_gainBase, exp);
    };
    // initialize time table floor as zero (first element)
    m_time_table[0] = 0;
    // construct remaining time table
    for(i = 1; i <= dsp_expon_time_range; i++)
    {
        // determine exponent of equation
        exp = (static_cast<float>(i) + m_time_from) * m_scaleGainExponent;
        // determine power
        m_time_table[i] = pow(m_gainBase, exp);
    };
};

float exponentiator::hyperfloor(float value)
{
    return((m_hyperfloor[0] * value) / (m_hyperfloor[1] + value));
};

float exponentiator::clip(float min, float max, float value)
{
    // perform clipping (for minimum and maximum)
    return fmax(min, fmin(max, value));
};

void exponentiator::setTablePos(float value)
{
    // convert float position into integer and fractional parts
    m_position_step = static_cast<int>(value);
    m_position_fine = value - static_cast<float>(m_position_step);
};

float exponentiator::getInterpolated(float fade, float first, float second)
{
    // perform a simple crossfade (linear interpolation)
    return(((1 - fade) * first) + (fade * second));
};

float exponentiator::eval_lin_pitch(float value)
{
    // clip, translate value and update table position (affecting _step and _fine)
    setTablePos(clip(m_linear_pitch_from, m_linear_pitch_to, value) - m_linear_pitch_from);
    // perform interpolation and return result (_fine serves as interpolation/fade parameter, _step determines table positions)
    return getInterpolated(m_position_fine, m_linear_pitch_table[m_position_step], m_linear_pitch_table[m_position_step + 1]);
};

float exponentiator::eval_osc_pitch(float value)
{
    // clip, translate value and update table position (affecting _step and _fine)
    setTablePos(clip(m_oscillator_pitch_from, m_oscillator_pitch_to, value) - m_oscillator_pitch_from);
    // perform interpolation and return result (_fine serves as interpolation/fade parameter, _step determines table positions)
    return getInterpolated(m_position_fine, m_oscillator_pitch_table[m_position_step], m_oscillator_pitch_table[m_position_step + 1]);
};

float exponentiator::eval_level(float value)
{
    // clip, translate value and update table position (affecting _step and _fine)
    setTablePos(clip(m_level_from, m_level_to, value) - m_level_from);
    // perform interpolation and return result (_fine serves as interpolation/fade parameter, _step determines table positions)
    return getInterpolated(m_position_fine, m_level_table[m_position_step], m_level_table[m_position_step + 1]);
};

float exponentiator::eval_time(float value)
{
    // clip, translate value and update table position (affecting _step and _fine)
    setTablePos(clip(m_time_from, m_time_to, value) - m_time_from);
    // perform interpolation and return result (_fine serves as interpolation/fade parameter, _step determines table positions)
    return getInterpolated(m_position_fine, m_time_table[m_position_step], m_time_table[m_position_step + 1]);
};

void exponentiator::getStatus_lin_pitch()
{
    // debug linear pitch table
    std::cout << "exponentiator: linear pitch table (index, input, value):" << std::endl;
    for(int i = 0; i <= dsp_expon_lin_pitch_range; i++)
    {
        std::cout << "\t" << i << ",\t" << i + m_linear_pitch_from << ",\t" << m_linear_pitch_table[i] << std::endl;
    };
};

void exponentiator::getStatus_osc_pitch()
{
    // debug oscillator pitch table
    std::cout << "exponentiator: oscillator pitch table (index, input, value):" << std::endl;
    for(int i = 0; i <= dsp_expon_osc_pitch_range; i++)
    {
        std::cout << "\t" << i << ",\t" << i + m_oscillator_pitch_from << ",\t" << m_oscillator_pitch_table[i] << std::endl;
    };
};

void exponentiator::getStatus_level()
{
    // debug level table
    std::cout << "exponentiator: level table (index, input, value):" << std::endl;
    for(int i = 0; i <= dsp_expon_level_range; i++)
    {
        std::cout << "\t" << i << ",\t" << i + m_level_from << ",\t" << m_level_table[i] << std::endl;
    };
};

void exponentiator::getStatus_time()
{
    // debug time table
    std::cout << "exponentiator: oscillator pitch table (index, input, value):" << std::endl;
    for(int i = 0; i <= dsp_expon_time_range; i++)
    {
        std::cout << "\t" << i << ",\t" << i + m_time_from << ",\t" << m_time_table[i] << std::endl;
    };
};

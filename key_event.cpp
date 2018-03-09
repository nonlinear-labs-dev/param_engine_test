#include <key_event.h>

void env_event::init()
{
    for(int v = 0; v < dsp_number_of_voices; v++)
    {
        m_levelFactor[v] = 0;
        for(int i = 0; i < sig_number_of_env_segments; i++)
        {
            m_timeFactor[v][i] = 1;
        };
    };
};

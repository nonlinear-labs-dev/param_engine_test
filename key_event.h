#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#include <global_defines_config.h>

struct key_event
{
    // local variables (hold pitch, velocity)
    int m_preload = 0;
    float m_velocity = 0;       // hold velocity (before apply)
    int m_type = 0;             // hold key event type (0: up, 1: down) (before apply)
};

struct env_event
{
    // local variables (hold time and level factors for each voice and envelope segment)
    float m_timeFactor[dsp_number_of_voices][sig_number_of_env_segments];
    float m_levelFactor[dsp_number_of_voices];
    // trouble when values are not initialized
    void init();
};

struct poly_key_event
{
    // local data structures
    key_event m_mono;                               // track mono event (last event)
    key_event m_poly[dsp_number_of_voices];         // track poly event (by voice)
    env_event m_env[sig_number_of_env_events];      // envelope event (time and level manipulation) probably not needed
    // handling of key event by parent param engine
};

#endif // KEY_EVENT_H

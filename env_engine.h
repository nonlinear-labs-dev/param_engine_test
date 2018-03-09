#ifndef ENV_ENGINE_H
#define ENV_ENGINE_H

#include <global_defines_config.h>
#include <utils.h>
#include <global_defines_envelopes.h>

struct env_head
{
    //
    int m_index;
    int m_type;
    int m_size;
    //
    int m_start;
    int m_stop;
    int m_state[sig_number_of_env_segments + 1];
    int m_next[sig_number_of_env_segments + 1];
    // currently only monophonic attack curvature
    float m_attackCurve;
};

struct env_segment
{
    // segment definition (dx, destination)
    float m_dx = 0;
    float m_dest = 0;
};

struct env_body
{
    // body holds one idle and four valid segments
    env_segment m_segment[sig_number_of_env_segments + 1];
    // state machine variables
    int m_state = 0;
    int m_next = 0;
    int m_index = 0;
    // rendering parameters
    float m_x = 0;
    float m_y = 0;
    float m_start = 0;
    float m_signal = 0;
    //
    void getStatus();
};

struct env_engine
{
    // voice management helper
    const int m_routePolyphony[2] = {1, dsp_number_of_voices};
    // local data structures
    dual_env_id_list m_poly_ids;
    env_head m_head[sig_number_of_envelopes];
    env_body m_body[sig_number_of_env_items];
    // proper init
    void init(const float gateRelease);   // parent param_engine determines/initializes gate release time (1 ms) (as long as it is no param)
    void init_adbdsr(const int envId);
    void init_gate(const int envId, const float gateRelease);
    void init_decay(const int envId);
    // segment updates
    void setSegmentDx(int voiceId, int envId, int segmentId, float value);
    void setSegmentDest(int voiceId, int envId, int segmentId, float value);
    // main tick functions (rendering)
    void tickMono();
    void tickPoly(const int voiceId);
    // individual item rendering
    void tickItem(const int voiceId, const int envId);
    // segment traversal
    void nextSegment(const int voiceId, const int envId);
    // helper functions
    float squaredCurvature(float value, float curvature);
    // triggers (keyDown, keyUp)
    void startEnvelope(const int voiceId, const int envId);
    void stopEnvelope(const int voiceId, const int envId);
    // debugging
    void getStatus(const int envId);
};

#endif // ENV_ENGINE_H

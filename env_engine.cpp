#include <math.h>
#include <env_engine.h>

void env_body::getStatus()
{
    //
    std::cout << "_segment_status:" << std::endl;
    for(int i = 0; i < sig_number_of_env_segments + 1; i++)
    {
        std::cout << "\t" << i << " (dx, dest): " << m_segment[i].m_dx << ", " << m_segment[i].m_dest << std::endl;
    };
};

void env_engine::init(float gateRelease)
{
    // declare pointers
    int i = 0;  // item pointer
    int e;      // envelope pointer
    // traverse definition array
    for(e = 0; e < sig_number_of_envelopes; e++)
    {
        // declare envelope header
        m_head[e].m_index = i;
        m_head[e].m_type = envelope_definition[e][0];
        m_head[e].m_size = m_routePolyphony[envelope_definition[e][1]];
        m_head[e].m_attackCurve = 0;
        // add id according to polyphony
        m_poly_ids.add(envelope_definition[e][1], e);
        // init by type
        switch(m_head[e].m_type)
        {
        case 0:
            // adbdsr
            init_adbdsr(e);
            break;
        case 1:
            // gate
            init_gate(e, gateRelease);
            break;
        case 2:
            // decay
            init_decay(e);
            break;
        };
        // update item pointer
        i += m_head[e].m_size;
    };
};

void env_engine::init_adbdsr(int envId)
{
    // item pointer
    int index;
    // initialize idle segment
    m_head[envId].m_state[0] = 0;
    m_head[envId].m_next[0] = 0;
    // adbdsr type start and stop
    m_head[envId].m_start = 1;
    m_head[envId].m_stop = 4;
    // outer loop: initialize envelope segment
    for(int s = 0; s < sig_number_of_env_segments; s++)
    {
        // segment definition
        m_head[envId].m_state[s + 1] = envelope_types[sig_env_type_adbdsr][s][0];
        m_head[envId].m_next[s + 1] = envelope_types[sig_env_type_adbdsr][s][1];
        // index pointer
        index = m_head[envId].m_index;
        // inner loop: initialize items
        for(int i = 0; i < m_head[envId].m_size; i++)
        {
            // item segment definition
            m_body[index].m_segment[s + 1].m_dx = static_cast<float>(envelope_types[sig_env_type_adbdsr][s][2]);
            m_body[index].m_segment[s + 1].m_dest = static_cast<float>(envelope_types[sig_env_type_adbdsr][s][3]);
            // item state definition
            m_body[index].m_state = 0;
            m_body[index].m_next = 0;
            m_body[index].m_index = 0;
            m_body[index].m_signal = 0;
            // update item pointer
            index++;
        };
    };
};

void env_engine::init_gate(int envId, float gateRelease)
{
    // item pointer
    int index;
    // initialize idle segment
    m_head[envId].m_state[0] = 0;
    m_head[envId].m_next[0] = 0;
    // gate type start and stop
    m_head[envId].m_start = 1;
    m_head[envId].m_stop = 2;
    // outer loop: initialize envelope segment
    for(int s = 0; s < sig_number_of_env_segments; s++)
    {
        // segment definition
        m_head[envId].m_state[s + 1] = envelope_types[sig_env_type_gate][s][0];
        m_head[envId].m_next[s + 1] = envelope_types[sig_env_type_gate][s][1];
        // inner loop: initialize items
        index = m_head[envId].m_index;
        for(int i = 0; i < m_head[envId].m_size; i++)
        {
            // item segment definition (distinct release case)
            if(s != 1)
            {
                // attack, rest unused
                m_body[index].m_segment[s + 1].m_dx = static_cast<float>(envelope_types[sig_env_type_gate][s][2]);
            }
            else
            {
                // release
                m_body[index].m_segment[s + 1].m_dx = gateRelease;
            };
            m_body[index].m_segment[s + 1].m_dest = static_cast<float>(envelope_types[sig_env_type_gate][s][3]);
            // item state definition
            m_body[index].m_state = 0;
            m_body[index].m_next = 0;
            m_body[index].m_index = 0;
            m_body[index].m_signal = 0;
            // update item pointer
            index++;
        };
    };
};

void env_engine::init_decay(int envId)
{
    // item pointer
    int index;
    // initialize idle segment
    m_head[envId].m_state[0] = 0;
    m_head[envId].m_next[0] = 0;
    // decay type start (no stop needed)
    m_head[envId].m_start = 1;
    m_head[envId].m_stop = 0;
    // outer loop: initialize envelope segment
    for(int s = 0; s < sig_number_of_env_segments; s++)
    {
        // segment definition
        m_head[envId].m_state[s+ 1] = envelope_types[sig_env_type_decay][s][0];
        m_head[envId].m_next[s + 1] = envelope_types[sig_env_type_decay][s][1];
        // inner loop: initialize items
        index = m_head[envId].m_index;
        for(int i = 0; i < m_head[envId].m_size; i++)
        {
            // item segment definition
            m_body[index].m_segment[s + 1].m_dx = static_cast<float>(envelope_types[sig_env_type_decay][s][2]);
            m_body[index].m_segment[s + 1].m_dest = static_cast<float>(envelope_types[sig_env_type_decay][s][3]);
            // item state definition
            m_body[index].m_state = 0;
            m_body[index].m_next = 0;
            m_body[index].m_index = 0;
            m_body[index].m_signal = 0;
            // update item pointer
            index++;
        };
    };
};

void env_engine::setSegmentDx(int voiceId, int envId, int segmentId, float value)
{
    // update particular segment dx of selected envelope at a specific voice
    m_body[m_head[envId].m_index + voiceId].m_segment[segmentId].m_dx = value;
};

void env_engine::setSegmentDest(int voiceId, int envId, int segmentId, float value)
{
    // update particular segment destination of selected envelope at a specific voice
    m_body[m_head[envId].m_index + voiceId].m_segment[segmentId].m_dest = value;
};

void env_engine::tickMono()
{
    // for all mono envelopes: perform tick
    for(int e = 0; e < m_poly_ids.m_data[0].m_length; e++)
    {
        tickItem(0, m_poly_ids.m_data[0].m_data[e]);
    };
};

void env_engine::tickPoly(int voiceId)
{
    // for all poly envelopes: perform tick
    for(int e = 0; e < m_poly_ids.m_data[1].m_length; e++)
    {
        tickItem(voiceId, m_poly_ids.m_data[1].m_data[e]);
    };
};

void env_engine::tickItem(int voiceId, int envId)
{
    // für optimalen ablauf: const, auch argumente !!!
    // set pointer, segment position and determine current difference
    const int index = m_head[envId].m_index + voiceId;
    // lokale temporäre variable auf m_body[index]: !!!
    // env_body test = m_body[index];
    // test.getStatus();
    // pointer
    // env_body* test = &m_body[index];
    // test->getStatus();
    // get auch...
    // oder reference
    // env_body& test = m_body[index];
    // test.getStatus();
    // ist wohl nicht wichtig, welches, also erstmal kopieren !
    int pos = m_body[index].m_index;
    float diff = m_body[index].m_segment[pos].m_dest - m_body[index].m_start;
        // (update difference per sample, as segment dest may change at any time, like segment dx)
    // individual envelope item (in particular segment)
    switch(m_body[index].m_state)
    {
    case 0:
        // idle
        break;
    case 1:
        // linear
        if(m_body[index].m_x < 1)
        {
            // create signal and update x
            m_body[index].m_signal = m_body[index].m_start + (diff * m_body[index].m_x);
            m_body[index].m_x += m_body[index].m_segment[pos].m_dx;
        }
        else
        {
            // clip x and create signal
            m_body[index].m_x = 1;
            m_body[index].m_signal = m_body[index].m_start + (diff * m_body[index].m_x);
            // trigger next segment
            nextSegment(voiceId, envId);
        };
#if dbg_env_tick
        // pedantic debugging:
        std::cout << "\tenv tick linear (voiceId, envId, x, signal):\t\t";
        std::cout << voiceId << ", " << envId << ", " << m_body[index].m_x << ", " << m_body[index].m_signal << std::endl;
#endif // dbg_env_tick
        break;
    case 2:
        // exponential
        if(m_body[index].m_y > dsp_render_min)
        {
            // create signal and update y
            m_body[index].m_signal = m_body[index].m_start + (diff * (1 - m_body[index].m_y));
            m_body[index].m_y *= 1 - m_body[index].m_segment[pos].m_dx;
        }
        else
        {
            // null y and create signal
            m_body[index].m_y = 0;
            m_body[index].m_signal = m_body[index].m_start + (diff * (1 - m_body[index].m_y));
            // trigger next segment
            nextSegment(voiceId, envId);
        };
#if dbg_env_tick
        // pedantic debugging:
        std::cout << "\tenv tick exponential (voiceId, envId, x, signal):\t";
        std::cout << voiceId << ", " << envId << ", " << m_body[index].m_y << ", " << m_body[index].m_signal << std::endl;
#endif // dbg_env_tick
        break;
    case 3:
        // polynomial
        if(m_body[index].m_x < 1)
        {
            // apply curvature (16th order polynomial) - maybe, this should be written down explicitly...
            m_body[index].m_y = squaredCurvature(m_body[index].m_x, m_head[envId].m_attackCurve);   // 2nd power
            m_body[index].m_y = squaredCurvature(m_body[index].m_y, m_head[envId].m_attackCurve);   // 4th power
            m_body[index].m_y = squaredCurvature(m_body[index].m_y, m_head[envId].m_attackCurve);   // 8th power
            m_body[index].m_y = squaredCurvature(m_body[index].m_y, m_head[envId].m_attackCurve);   // 16th power
            // create signal and update x
            m_body[index].m_signal = m_body[index].m_start + (diff * m_body[index].m_y);
            m_body[index].m_x += m_body[index].m_segment[pos].m_dx;
        }
        else
        {
            // clip x and create signal (on 100%, no curvature has to be applied, as 1^16 = 1 - ((1 - 1)^16) = 1)
            m_body[index].m_x = 1;
            m_body[index].m_signal = m_body[index].m_start + (diff * m_body[index].m_x);
            // trigger next segment
            nextSegment(voiceId, envId);
        };
#if dbg_env_tick
        // pedantic debugging:
        std::cout << "\tenv tick polynomial (voiceId, envId, x, signal):\t";
        std::cout << voiceId << ", " << envId << ", " << m_body[index].m_y << ", " << m_body[index].m_signal << std::endl;
#endif // dbg_env_tick
        break;
    };
};

void env_engine::nextSegment(int voiceId, int envId)
{
    // segment traversal:
    // set pointer
    int index = m_head[envId].m_index + voiceId;
    // update variables
    m_body[index].m_x = m_body[index].m_segment[m_body[index].m_next].m_dx;         // prepare for linear, polynomial
    m_body[index].m_y = 1 - m_body[index].m_segment[m_body[index].m_next].m_dx;     // prepare for exponential
    m_body[index].m_start = m_body[index].m_signal;                                 // prepare start
    // update item state (state, index, next)
    m_body[index].m_state = m_head[envId].m_state[m_body[index].m_next];
    m_body[index].m_index = m_body[index].m_next;
    m_body[index].m_next = m_head[envId].m_next[m_body[index].m_next];
};

float env_engine::squaredCurvature(float value, float curvature)
{
    // depending on the curvature [-1 .. 1], the resulting x [-1 .. 1] will produce curve set [(1 - (1 - x)²) .. x .. x²]
    // on top, the calculation is sign sensitive, so the squares are of nature: square = x * abs(x)
    return(value * (1 + (curvature * (fabs(value) - 1))));
};

void env_engine::startEnvelope(int voiceId, int envId)
{
    // set pointer
    int index = m_head[envId].m_index + voiceId;
    // update variables
    m_body[index].m_x = m_body[index].m_segment[m_head[envId].m_start].m_dx;        // prepare for linear, polynomial
    m_body[index].m_y = 1 - m_body[index].m_segment[m_head[envId].m_start].m_dx;    // prepare for exponential
    m_body[index].m_start = m_body[index].m_signal;                                 // prepare start
    // update item state (state, index, next)
    m_body[index].m_state = m_head[envId].m_state[m_head[envId].m_start];
    m_body[index].m_index = m_head[envId].m_start;
    m_body[index].m_next =  m_head[envId].m_next[m_head[envId].m_start];
};

void env_engine::stopEnvelope(int voiceId, int envId)
{
    // set pointer
    int index = m_head[envId].m_index + voiceId;
    // update variables
    m_body[index].m_x = m_body[index].m_segment[m_head[envId].m_stop].m_dx;         // prepare for linear, polynomial
    m_body[index].m_y = 1 - m_body[index].m_segment[m_head[envId].m_stop].m_dx;     // prepare for exponential
    m_body[index].m_start = m_body[index].m_signal;                                 // prepare start
    // update item state (state, index, next)
    m_body[index].m_state = m_head[envId].m_state[m_head[envId].m_stop];
    m_body[index].m_index = m_head[envId].m_stop;
    m_body[index].m_next =  m_head[envId].m_next[m_head[envId].m_stop];
};

void env_engine::getStatus(int envId)
{
    //
    int index = m_head[envId].m_index;
    std::cout << "\nparam_engine::env_state(" << envId << "):\t(";
    for(int i = 0; i < m_head[envId].m_size; i++)
    {
        std::cout << m_body[index + i].m_state << ", ";
    };
    std::cout << ")" << std::endl;
    std::cout << "param_engine::env_segment(" << envId << "):\t(";
    for(int i = 0; i < m_head[envId].m_size; i++)
    {
        std::cout << m_body[index + i].m_index << ", ";
    };
    std::cout << ")" << std::endl;
    std::cout << "param_engine::env_signal(" << envId << "):\t(";
    for(int i = 0; i < m_head[envId].m_size; i++)
    {
        std::cout << m_body[index + i].m_signal << ", ";
    };
    std::cout << ")" << std::endl;
};

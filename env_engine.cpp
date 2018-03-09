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

void env_engine::init(const float gateRelease)
{
    // declare indices
    int i = 0;  // item index
    int e;      // envelope index
    // traverse definition array
    for(e = 0; e < sig_number_of_envelopes; e++)
    {
        // create reference
        env_head* obj = &m_head[e];
        // declare envelope header
        obj->m_index = i;
        obj->m_type = envelope_definition[e][0];
        obj->m_size = m_routePolyphony[envelope_definition[e][1]];
        obj->m_attackCurve = 0.f;
        // add id according to polyphony
        m_poly_ids.add(envelope_definition[e][1], e);
        // init by type
        switch(obj->m_type)
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
        i += obj->m_size;
    };
};

void env_engine::init_adbdsr(const int envId)
{
    // item index and reference
    int index;
    env_head* obj = &m_head[envId];
    // initialize idle segment
    obj->m_state[0] = 0;
    obj->m_next[0] = 0;
    // adbdsr type start and stop
    obj->m_start = 1;
    obj->m_stop = 4;
    // outer loop: initialize envelope segment
    for(int s = 0; s < sig_number_of_env_segments; s++)
    {
        // segment definition
        obj->m_state[s + 1] = envelope_types[sig_env_type_adbdsr][s][0];
        obj->m_next[s + 1] = envelope_types[sig_env_type_adbdsr][s][1];
        // item index
        index = obj->m_index;
        // inner loop: initialize items
        for(int i = 0; i < obj->m_size; i++)
        {
            // reference pointer
            env_body* item = &m_body[index];
            // item segment definition
            item->m_segment[s + 1].m_dx = static_cast<float>(envelope_types[sig_env_type_adbdsr][s][2]);
            item->m_segment[s + 1].m_dest = static_cast<float>(envelope_types[sig_env_type_adbdsr][s][3]);
            // item state definition
            item->m_state = 0;
            item->m_next = 0;
            item->m_index = 0;
            item->m_signal = 0;
            // update item index
            index++;
        };
    };
};

void env_engine::init_gate(const int envId, const float gateRelease)
{
    // item index and reference
    int index;
    env_head* obj = &m_head[envId];
    // initialize idle segment
    obj->m_state[0] = 0;
    obj->m_next[0] = 0;
    // gate type start and stop
    obj->m_start = 1;
    obj->m_stop = 2;
    // outer loop: initialize envelope segment
    for(int s = 0; s < sig_number_of_env_segments; s++)
    {
        // segment definition
        obj->m_state[s + 1] = envelope_types[sig_env_type_gate][s][0];
        obj->m_next[s + 1] = envelope_types[sig_env_type_gate][s][1];
        // item index
        index = obj->m_index;
        // inner loop: initialize items
        for(int i = 0; i < obj->m_size; i++)
        {
            // reference pointer
            env_body* item = &m_body[index];
            // item segment definition (distinct release case)
            if(s != 1)
            {
                // attack, rest unused
                item->m_segment[s + 1].m_dx = static_cast<float>(envelope_types[sig_env_type_gate][s][2]);
            }
            else
            {
                // release
                item->m_segment[s + 1].m_dx = gateRelease;
            };
            item->m_segment[s + 1].m_dest = static_cast<float>(envelope_types[sig_env_type_gate][s][3]);
            // item state definition
            item->m_state = 0;
            item->m_next = 0;
            item->m_index = 0;
            item->m_signal = 0;
            // update item index
            index++;
        };
    };
};

void env_engine::init_decay(const int envId)
{
    // item index and reference
    int index;
    env_head* obj = &m_head[envId];
    // initialize idle segment
    obj->m_state[0] = 0;
    obj->m_next[0] = 0;
    // decay type start (no stop needed)
    obj->m_start = 1;
    obj->m_stop = 0;
    // outer loop: initialize envelope segment
    for(int s = 0; s < sig_number_of_env_segments; s++)
    {
        // segment definition
        obj->m_state[s+ 1] = envelope_types[sig_env_type_decay][s][0];
        obj->m_next[s + 1] = envelope_types[sig_env_type_decay][s][1];
        // item index
        index = obj->m_index;
        // inner loop: initialize items
        for(int i = 0; i < obj->m_size; i++)
        {
            // reference pointer
            env_body* item = &m_body[index];
            // item segment definition
            item->m_segment[s + 1].m_dx = static_cast<float>(envelope_types[sig_env_type_decay][s][2]);
            item->m_segment[s + 1].m_dest = static_cast<float>(envelope_types[sig_env_type_decay][s][3]);
            // item state definition
            item->m_state = 0;
            item->m_next = 0;
            item->m_index = 0;
            item->m_signal = 0;
            // update item pointer
            index++;
        };
    };
};

void env_engine::setSegmentDx(int voiceId, int envId, int segmentId, float value)
{
    // update particular segment dx of selected envelope at a specific voice (de-referencing not recommended for one line)
    m_body[m_head[envId].m_index + voiceId].m_segment[segmentId].m_dx = value;
};

void env_engine::setSegmentDest(int voiceId, int envId, int segmentId, float value)
{
    // update particular segment destination of selected envelope at a specific voice (de-referencing not recommended for one line)
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

void env_engine::tickPoly(const int voiceId)
{
    // for all poly envelopes: perform tick
    for(int e = 0; e < m_poly_ids.m_data[1].m_length; e++)
    {
        tickItem(voiceId, m_poly_ids.m_data[1].m_data[e]);
    };
};

void env_engine::tickItem(const int voiceId, const int envId)
{
    // reference to current object/item:
    env_head* obj = &m_head[envId];
    const int index = obj->m_index + voiceId;
    env_body* item = &m_body[index];
    const int pos = item->m_index;
    const float diff = item->m_segment[pos].m_dest - item->m_start;
    // (update difference per sample, as segment dest may change at any time, like segment dx)
    // individual envelope item (in particular segment)
    switch(item->m_state)
    {
    case 0:
        // idle
        break;
    case 1:
        // linear
        if(item->m_x < 1)
        {
            // create signal and update x
            item->m_signal = item->m_start + (diff * item->m_x);
            item->m_x += item->m_segment[pos].m_dx;
        }
        else
        {
            // clip x and create signal
            item->m_x = 1;
            item->m_signal = item->m_start + (diff * item->m_x);
            // trigger next segment
            nextSegment(voiceId, envId);
        };
#if dbg_env_tick
        // pedantic debugging:
        std::cout << "\tenv tick linear (voiceId, envId, x, signal):\t\t";
        std::cout << voiceId << ", " << envId << ", " << item->m_x << ", " << item->m_signal << std::endl;
#endif // dbg_env_tick
        break;
    case 2:
        // exponential
        if(item->m_y > dsp_render_min)
        {
            // create signal and update y
            item->m_signal = item->m_start + (diff * (1 - item->m_y));
            item->m_y *= 1 - item->m_segment[pos].m_dx;
        }
        else
        {
            // null y and create signal
            item->m_y = 0;
            item->m_signal = item->m_start + (diff * (1 - item->m_y));
            // trigger next segment
            nextSegment(voiceId, envId);
        };
#if dbg_env_tick
        // pedantic debugging:
        std::cout << "\tenv tick exponential (voiceId, envId, x, signal):\t";
        std::cout << voiceId << ", " << envId << ", " << item->m_y << ", " << item->m_signal << std::endl;
#endif // dbg_env_tick
        break;
    case 3:
        // polynomial
        if(item->m_x < 1)
        {
            // apply curvature (16th order polynomial) - maybe, this should be written down explicitly...
            item->m_y = squaredCurvature(item->m_x, obj->m_attackCurve);   // 2nd power
            item->m_y = squaredCurvature(item->m_y, obj->m_attackCurve);   // 4th power
            item->m_y = squaredCurvature(item->m_y, obj->m_attackCurve);   // 8th power
            item->m_y = squaredCurvature(item->m_y, obj->m_attackCurve);   // 16th power
            // create signal and update x
            item->m_signal = item->m_start + (diff * item->m_y);
            item->m_x += item->m_segment[pos].m_dx;
        }
        else
        {
            // clip x and create signal (on 100%, no curvature has to be applied, as 1^16 = 1 - ((1 - 1)^16) = 1)
            item->m_x = 1;
            item->m_signal = item->m_start + (diff * item->m_x);
            // trigger next segment
            nextSegment(voiceId, envId);
        };
#if dbg_env_tick
        // pedantic debugging:
        std::cout << "\tenv tick polynomial (voiceId, envId, x, signal):\t";
        std::cout << voiceId << ", " << envId << ", " << item->m_y << ", " << item->m_signal << std::endl;
#endif // dbg_env_tick
        break;
    };
};

void env_engine::nextSegment(const int voiceId, const int envId)
{
    // segment traversal
    // reference to current object/item
    env_head* obj = &m_head[envId];
    const int index = obj->m_index + voiceId;
    env_body* item = &m_body[index];
    // update variables
    item->m_x = item->m_segment[item->m_next].m_dx;         // prepare for linear, polynomial
    item->m_y = 1 - item->m_segment[item->m_next].m_dx;     // prepare for exponential
    item->m_start = item->m_signal;                         // prepare start
    // update item state (state, index, next)
    item->m_state = obj->m_state[item->m_next];
    item->m_index = item->m_next;
    item->m_next = obj->m_next[item->m_next];
};

float env_engine::squaredCurvature(float value, float curvature)
{
    // depending on the curvature [-1 .. 1], the resulting x [-1 .. 1] will produce curve set [(1 - (1 - x)²) .. x .. x²]
    // on top, the calculation is sign sensitive, so the squares are of nature: square = x * abs(x)
    return(value * (1 + (curvature * (fabs(value) - 1))));
};

void env_engine::startEnvelope(const int voiceId, const int envId)
{
    // reference to current object/item
    env_head* obj = &m_head[envId];
    const int index = obj->m_index + voiceId;
    env_body* item = &m_body[index];
    // update variables
    item->m_x = item->m_segment[obj->m_start].m_dx;        // prepare for linear, polynomial
    item->m_y = 1 - item->m_segment[obj->m_start].m_dx;    // prepare for exponential
    item->m_start = item->m_signal;                        // prepare start
    // update item state (state, index, next)
    item->m_state = obj->m_state[obj->m_start];
    item->m_index = obj->m_start;
    item->m_next =  obj->m_next[obj->m_start];
};

void env_engine::stopEnvelope(const int voiceId, const int envId)
{
    // reference to current object/item
    env_head* obj = &m_head[envId];
    const int index = obj->m_index + voiceId;
    env_body* item = &m_body[index];
    // update variables
    item->m_x = item->m_segment[obj->m_stop].m_dx;         // prepare for linear, polynomial
    item->m_y = 1 - item->m_segment[obj->m_stop].m_dx;     // prepare for exponential
    item->m_start = item->m_signal;                        // prepare start
    // update item state (state, index, next)
    item->m_state = obj->m_state[obj->m_stop];
    item->m_index = obj->m_stop;
    item->m_next =  obj->m_next[obj->m_stop];
};

void env_engine::getStatus(const int envId)
{
    //
    env_head* obj = &m_head[envId];
    const int index = m_head[envId].m_index;
    std::cout << "\nparam_engine::env_state(" << envId << "):\t(";
    for(int i = 0; i < obj->m_size; i++)
    {
        std::cout << m_body[index + i].m_state << ", ";
    };
    std::cout << ")" << std::endl;
    std::cout << "param_engine::env_segment(" << envId << "):\t(";
    for(int i = 0; i < obj->m_size; i++)
    {
        std::cout << m_body[index + i].m_index << ", ";
    };
    std::cout << ")" << std::endl;
    std::cout << "param_engine::env_signal(" << envId << "):\t(";
    for(int i = 0; i < obj->m_size; i++)
    {
        std::cout << m_body[index + i].m_signal << ", ";
    };
    std::cout << ")" << std::endl;
};

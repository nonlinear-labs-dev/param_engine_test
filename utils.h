#ifndef UTILS_H
#define UTILS_H

#include <global_defines_config.h>

struct id_list
{
    //
    int m_data[sig_number_of_params];
    int m_length = 0;
    //
    void reset();
    void add(int id);
    void getStatus();
};

struct poly_id_list
{
    //
    id_list m_data[dsp_poly_types];
    //
    void reset();
    void add(int polyType, int id);
    void getStatus();
};

struct clock_id_list
{
    //
    poly_id_list m_data[dsp_clock_types];
    //
    void reset();
    void add(int clockType, int polyType, int id);
    void getStatus();
};

struct dual_clock_id_list
{
    //
    clock_id_list m_data[2];
    //
    void reset();
    void add(int spreadType, int clockType, int polyType, int id);
    void getStatus();
};

struct dual_id_list
{
    //
    id_list m_data[2];
    //
    void reset();
    void add(int listId, int id);
    void getStatus();
};

struct polyDual_id_list
{
    //
    dual_id_list m_data[dsp_poly_types];
    //
    void reset();
    void add(int polyId, int listId, int id);
    void getStatus();
};

struct env_id_list
{
    //
    int m_data[sig_number_of_envelopes];
    int m_length = 0;
    //
    void reset();
    void add(int id);
    void getStatus();
};

struct dual_env_id_list
{
    //
    env_id_list m_data[2];
    //
    void reset();
    void add(int listId, int id);
    void getStatus();
};

#endif // UTILS_H

#include <utils.h>

void id_list::reset()
{
    m_length = 0;
};

void id_list::add(int id)
{
    m_data[m_length] = id;
    m_length++;
};

void id_list::getStatus()
{
    std::cout << "id_list_status: (";
    for(int i = 0; i < m_length; i++)
    {
        std::cout << m_data[i] << ", ";
    };
    std::cout << ")" << std::endl;
};

void poly_id_list::reset()
{
    for(int i = 0; i < dsp_poly_types; i++)
    {
        m_data[i].reset();
    };
};

void poly_id_list::add(int polyType, int id)
{
    m_data[polyType].add(id);
};

void poly_id_list::getStatus()
{
    std::cout << "poly_id_list_status: (" << std::endl;
    for(int i = 0; i < dsp_poly_types; i++)
    {
        m_data[i].getStatus();
    };
    std::cout << ")" << std::endl;
};

void clock_id_list::reset()
{
    for(int i = 0; i < dsp_clock_types; i++)
    {
        m_data[i].reset();
    };
};

void clock_id_list::add(int clockType, int polyType, int id)
{
    m_data[clockType].add(polyType, id);
};

void clock_id_list::getStatus()
{
    std::cout << "clock_id_list_status: (" << std::endl;
    for(int i = 0; i < dsp_clock_types; i++)
    {
        m_data[i].getStatus();
    };
    std::cout << ")" << std::endl;
};

void dual_id_list::reset()
{
    m_data[0].reset();
    m_data[1].reset();
};

void dual_id_list::add(int listId, int id)
{
    m_data[listId].add(id);
};

void dual_id_list::getStatus()
{
     std::cout << "dual_id_list_status: (" << std::endl;
     m_data[0].getStatus();
     m_data[1].getStatus();
     std::cout << ")" << std::endl;
};

void polyDual_id_list::reset()
{
    for(int i = 0; i < dsp_poly_types; i++)
    {
        m_data[i].reset();
    };
};

void polyDual_id_list::add(int polyId, int listId, int id)
{
    m_data[polyId].add(listId, id);
};

void polyDual_id_list::getStatus()
{
    std::cout << "polyDual_id_list_status: (" << std::endl;
    for(int i = 0; i < dsp_poly_types; i++)
    {
        m_data[i].getStatus();
    };
    std::cout << ")" << std::endl;
};

void env_id_list::reset()
{
    m_length = 0;
};

void env_id_list::add(int id)
{
    m_data[m_length] = id;
    m_length++;
};

void env_id_list::getStatus()
{
    std::cout << "env_id_list_status: (";
    for(int i = 0; i < m_length; i++)
    {
        std::cout << m_data[i] << ", ";
    };
    std::cout << ")" << std::endl;
};

void dual_env_id_list::reset()
{
    m_data[0].reset();
    m_data[1].reset();
};

void dual_env_id_list::add(int listId, int id)
{
    m_data[listId].add(id);
};

void dual_env_id_list::getStatus()
{
     std::cout << "dual_env_id_list_status: (" << std::endl;
     m_data[0].getStatus();
     m_data[1].getStatus();
     std::cout << ")" << std::endl;
};

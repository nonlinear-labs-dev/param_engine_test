#include <decoder.h>

void decoder::init()
{
    int i;
    // construct paramId lists (recall, keyEvent)
    for(i = 0; i < lst_recall_length; i++)
    {
        m_listTraversal[0].add(paramIds_recall[i]);
    };
    for(i = 0; i < lst_keyEvent_length; i++)
    {
        m_listTraversal[1].add(paramIds_keyEvent[i]);
    };
};

int decoder::getCommandId(const int status)
{
    return tcd_protocol[status];
};

int decoder::unsigned14(const int data0, const int data1)
{
    return((data0 << 7) + data1);
};

int decoder::signed14(const int data0, const int data1)
{
    return(m_getSign[data0 >> 6] * unsigned14((data0 & 63), data1));
};

void decoder::unsigned28upper(const int data0, const int data1)
{
    m_sign = 1;
    m_value = unsigned14(data0, data1) << 14;
};

void decoder:: signed28upper(const int data0, const int data1)
{
    m_sign = m_getSign[data0 >> 6];
    m_value = unsigned14((data0 & 63), data1) << 14;
};

int decoder::apply28lower(const int data0, const int data1)
{
    m_value += unsigned14(data0, data1);
    return(m_sign * m_value);
};

int decoder::selectionEvent(const int from, const int to, const int id)
{
    // selection core mechanism: check ID against FROM, TO, return result (0, 1)
    m_event[1] = id - from;
    m_event[2] = to - id;
    m_event[3] = m_event[1] | m_event[2];
    m_event[4] = m_event[1] & m_event[2];
    return(0 > m_event[3 + m_event[0]] ? 0 : 1);
};

int decoder::traverseRecall()
{
    // monophonic implementation
    // get current index
    const int id = m_listTraversal[0].m_data[m_listIndex];
    // increase (and wrap) index
    m_listIndex = (m_listIndex + 1) % m_listTraversal[0].m_length;
    // return current
    return id;
};

int decoder::traverseKeyEvent()
{
    // polyphonic implementation (voice selection by sender !!!)
    // get current index
    const int id = m_listTraversal[1].m_data[m_listIndex];
    // increase (and wrap) index
    m_listIndex = (m_listIndex + 1) % m_listTraversal[1].m_length;
    // return current
    return id;
};

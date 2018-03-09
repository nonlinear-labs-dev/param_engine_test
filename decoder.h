#ifndef DECODER_H
#define DECODER_H

#include <utils.h>
#include <global_defines_protocol.h>
#include <global_defines_lists.h>

struct decoder
{
    // local variables
    int m_voiceFrom;
    int m_voiceTo;
    int m_paramFrom;
    int m_paramTo;
    int m_value;
    int m_sign;
    int m_utilityId = 0;
    int m_listId = 0;           // (predefined) lists in order to simplyfy TCD recall or key sequences (recall, key events)
    int m_listIndex = 0;        // list traversal index
    // data structures
    const int m_getSign[2] = {1, -1};
    int m_event[5];
    dual_id_list m_selectedVoices;
    polyDual_id_list m_selectedParams;
    id_list m_listTraversal[lst_number_of_lists];
    // proper init (lists)
    void init();
    // tcd command evaluation
    int getCommandId(const int status);
    // tcd argument parsing
    int unsigned14(const int data0, const int data1);
    int signed14(const int data0, const int data1);
    void unsigned28upper(const int data0, const int data1);
    void signed28upper(const int data0, const int data1);
    int apply28lower(const int data0, const int data1);
    // tcd voice and parameter selection event evaluation
    int selectionEvent(const int from, const int to, const int id);
    // tcd list traversal
    int traverseRecall();
    int traverseKeyEvent();
};

#endif // DECODER_H

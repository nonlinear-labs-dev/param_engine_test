#ifndef GLOBAL_DEFINES_ENVELOPES_H
#define GLOBAL_DEFINES_ENVELOPES_H

#include <global_defines_config.h>

/* envelope types:      (state, next, dx, dest) per segment per envelope type
 *  - state:            0 (idle), 1 (linear), 2 (exponential), 3 (polynomial)
 *  - next:             segment index (0 - idle, 1 ... 4 - segment 0 ... 3)
 *  - dx:               segment dx (in case it needs to be initialized): 0 - infinite, 1 - zero
 *  - dest:             segment destination (in case it needs to be initialized) (0 ... 1) * 100%
 * */

#define sig_env_type_adbdsr 0
#define sig_env_type_gate 1
#define sig_env_type_decay 2

const int envelope_types[sig_number_of_env_types][sig_number_of_env_segments][4] = {
    // define:  state   next    dx      dest
    {   // adbdsr type
        {       3,      2,      0,      1       },                      // 1 polynomial attack phase to 100% (next: 2 - dec1)
        {       1,      3,      0,      0       },                      // 2 linear decay1 phase (next: 3 - dec2)
        {       2,      0,      0,      0       },                      // 3 exponential decay2 phase (next: 0 - idle)
        {       2,      0,      0,      0       }                       // 4 exponential release phase to 0% (next: 0 - idle)
    },
    {   // gate type
        {       1,      0,      1,      1       },                      // 1 linear attack phase to 100% in no time (next: 0)
        {       2,      0,      0,      0       },                      // 2 exponential release phase to 0% (next: 0 - idle)
        {       0,      0,      0,      0       },                      // (remaining unused)
        {       0,      0,      0,      0       }
    },
    {   // decay type
        {       1,      2,      1,      1       },                      // 1 linear attack phase to 100% in no time (next: 2)
        {       2,      0,      0,      0       },                      // 2 exponential release phase to 0% (next: 0 - idle)
        {       0,      0,      0,      0       },                      // (remaining unused)
        {       0,      0,      0,      0       }
    }
};

/* envelope definition:
 *  - env type:         (0 - adbdsr, 1 - gate, 2 - decay)
 *  - poly type:        (0 - mono, 1 - poly)
 * */
const int envelope_definition[sig_number_of_envelopes][2] = {
    {0, 1},                                                             // poly adbdsr envelope env_a
    {0, 1},                                                             // poly adbdsr envelope env_b
    {0, 1},                                                             // poly adbdsr envelope env_c
    {1, 1},                                                             // poly gate envelope gate_env
    {2, 0}                                                              // mono decay envelope flanger_env
};

#endif // GLOBAL_DEFINES_ENVELOPES_H

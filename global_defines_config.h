#ifndef GLOBAL_DEFINES_CONFIG_H
#define GLOBAL_DEFINES_CONFIG_H

#include <iostream>

// main project parameters
#define dsp_sample_rate 48000
#define dsp_upsample_factor 1           // 1 (48k) or 2 (96k)
#define dsp_number_of_voices 8
#define dsp_poly_types 2
#define dsp_clock_types 4
#define dsp_clock_fast_division 5
#define dsp_clock_slow_division 120
#define dsp_render_min 1e-9
#define dsp_flush_wait_fastTicks 30     // flush waits 30 fast ticks (150 samples, 3.125 ms) (variant I)
#define dsp_flush_table_size 480        // 1ms at 48k

// midi operation mode (0 - anton, 1 - tcd), ctrl_ch_id is reserved for pseudo tcd control when in "anton" mode
#define ext_midi_mode 1
#define ext_midi_control_channel_id 15

// project parameter definition
#define sig_number_of_params 26         // 13 ENV_A params, 6 OSC params, 2 MASTER params, 5 POLY params (no EnvC Rate)
#define sig_number_of_param_items 61    // (13 + 6 + 2) MONO params = 21 param itmems; (5 x 8) POLY params = 40 param items; total: 32 + 40 = 61 items
#define sig_number_of_signal_items 10   //
#define sig_number_of_utilities 2
#define sig_number_of_envelopes 5
#define sig_number_of_env_items 33
#define sig_number_of_env_segments 4
#define sig_number_of_env_types 3
#define sig_number_of_env_events 3

// helper values (handling milliseconds, reference tone, exponential scale ranges)
#define dsp_samples_to_ms 1e-3
#define dsp_init_pitch_reference 440
#define dsp_expon_osc_pitch_from -20
#define dsp_expon_osc_pitch_range 150
#define dsp_expon_lin_pitch_from -150
#define dsp_expon_lin_pitch_range 300
#define dsp_expon_level_from -300
#define dsp_expon_level_range 400
#define dsp_expon_time_from -20
#define dsp_expon_time_range 110
#define dsp_expon_time_factor 104.0781

// helper env constants
#define env_norm_peak 0.023766461       // equals 1 / 42.0761 (taken from prototype)
#define env_clip_peak 1.412537545       // equals +3 dB (candidate levelKT clipping)

// tcd lists (see _lists.h)
#define lst_number_of_lists 2           // predefined paramId lists (simplifying recal and key event update TCD sequences)
#define lst_recall_length 21            // declare number of paramIds for recall list (MONO - currently)
#define lst_keyEvent_length 5           // declare number of paramIds for key event list (POLY - currently)

// crucial tcd parameters (internal IDs)
#define par_envelopeA 0                 // item pointer to (consecutive) envelope parameters A (internal ids)
#define par_envelopeB 0                 // item pointer to (consecutive) envelope parameters B (internal ids)
#define par_envelopeC 0                 // item pointer to (consecutive) envelope parameters C (internal ids)
#define par_noteSteal 21                // item pointer to note steal (internal id)
#define par_notePitch 22                // item pointer to note pitch (internal id)

// temporary debugging flags
#define dbg_tcd_in 1                    // pedantic debugging of tcd events
#define dbg_env_tick 0                  // pedantic debugging of envelope sample ticks (use with caution)

#endif // GLOBAL_DEFINES_CONFIG_H

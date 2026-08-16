#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t no_tach_ms;
    uint32_t cutoff_remaining_ms;
    bool fault_latched;
} fan_logic_state_t;

typedef struct {
    bool drive_power;
    uint8_t applied_percent;
    bool newly_faulted;
} fan_logic_output_t;

void fan_logic_init(fan_logic_state_t *state);
void fan_logic_clear_fault(fan_logic_state_t *state);
void fan_logic_step(fan_logic_state_t *state, bool power_requested,
                    uint8_t requested_percent, uint32_t tach_pulses,
                    uint32_t elapsed_ms, uint32_t stall_timeout_ms,
                    uint32_t cutoff_delay_ms, fan_logic_output_t *output);

uint32_t fan_logic_ledc_duty(uint8_t percent, uint32_t max_duty);
uint32_t fan_logic_rpm(uint32_t pulses, uint32_t pulses_per_revolution,
                       uint32_t window_ms);
void fan_logic_restore_requested(bool stored_power_valid, uint8_t stored_power,
                                 bool stored_percent_valid,
                                 uint8_t stored_percent, bool *power,
                                 uint8_t *percent);

#ifdef __cplusplus
}
#endif

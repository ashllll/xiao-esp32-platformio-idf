#include "fan_logic.h"

#include <stddef.h>

void fan_logic_init(fan_logic_state_t *state)
{
    if (state != NULL) {
        *state = (fan_logic_state_t){0};
    }
}

void fan_logic_clear_fault(fan_logic_state_t *state)
{
    fan_logic_init(state);
}

uint32_t fan_logic_ledc_duty(uint8_t percent, uint32_t max_duty)
{
    if (percent > 100U) {
        percent = 100U;
    }
    return ((uint32_t)percent * max_duty + 50U) / 100U;
}

uint32_t fan_logic_rpm(uint32_t pulses, uint32_t pulses_per_revolution,
                       uint32_t window_ms)
{
    if (pulses_per_revolution == 0U || window_ms == 0U) {
        return 0U;
    }
    uint64_t numerator = (uint64_t)pulses * 60000ULL;
    uint64_t denominator = (uint64_t)pulses_per_revolution * window_ms;
    return (uint32_t)((numerator + denominator / 2U) / denominator);
}

void fan_logic_restore_requested(bool stored_power_valid, uint8_t stored_power,
                                 bool stored_percent_valid,
                                 uint8_t stored_percent, bool *power,
                                 uint8_t *percent)
{
    if (power == NULL || percent == NULL) {
        return;
    }
    *power = false;
    *percent = 50U;
    if (stored_power_valid && stored_percent_valid && stored_percent <= 100U) {
        *power = stored_power != 0U;
        *percent = stored_percent;
    }
}

void fan_logic_step(fan_logic_state_t *state, bool power_requested,
                    uint8_t requested_percent, uint32_t tach_pulses,
                    uint32_t elapsed_ms, uint32_t stall_timeout_ms,
                    uint32_t cutoff_delay_ms, fan_logic_output_t *output)
{
    if (state == NULL || output == NULL) {
        return;
    }

    *output = (fan_logic_output_t){0};
    if (requested_percent > 100U) {
        requested_percent = 100U;
    }

    if (state->fault_latched) {
        output->applied_percent = 0U;
        if (state->cutoff_remaining_ms > elapsed_ms) {
            state->cutoff_remaining_ms -= elapsed_ms;
            output->drive_power = true;
        } else {
            state->cutoff_remaining_ms = 0U;
            output->drive_power = false;
        }
        return;
    }

    if (!power_requested) {
        state->no_tach_ms = 0U;
        return;
    }

    output->drive_power = true;
    output->applied_percent = requested_percent;
    if (requested_percent == 0U) {
        state->no_tach_ms = 0U;
        return;
    }

    if (tach_pulses > 0U) {
        state->no_tach_ms = 0U;
        return;
    }

    if (UINT32_MAX - state->no_tach_ms < elapsed_ms) {
        state->no_tach_ms = UINT32_MAX;
    } else {
        state->no_tach_ms += elapsed_ms;
    }

    if (state->no_tach_ms >= stall_timeout_ms) {
        state->fault_latched = true;
        state->cutoff_remaining_ms = cutoff_delay_ms;
        output->applied_percent = 0U;
        output->newly_faulted = true;
    }
}

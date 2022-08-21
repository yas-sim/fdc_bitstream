#pragma once

#include "fdc_vfo_base.h"

/**
 * @brief VFO class using PID control
 * 
 */
class vfo_pid : public vfo_base {
public:
    double m_prev_phase_error;
    double m_prev_freq_error;
    double m_freq_bias;
public:
    vfo_pid() { reset(); }
    void disp_vfo_status(void) override;
    void reset(void) override;
    double calc(double pulse_pos) override;
};

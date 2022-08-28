#pragma once

#include "fdc_vfo_base.h"

/**
 * @brief VFO class using PID control
 * 
 */
class vfo_pid : public vfo_base {
public:
    double m_prev_pulse_pos;
    double m_prev_phase_err;
    double m_phase_err_integral;
    double m_phase_diff_integral;

public:
    vfo_pid() { reset(); }
    void disp_vfo_status(void) override;
    void reset(void) override;
    void soft_reset(void) override;
    double calc(double pulse_pos) override;
};

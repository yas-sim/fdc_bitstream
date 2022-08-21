#include <iostream>

#include "vfo_simple.h"

/**
 * @brief Simple VFO
 * 
 * @param pulse_pos 
 * @return double 
 */
double vfo_simple::calc(double pulse_pos) {

    // Data pulse position adjustment == phase correction
    double phase_error = m_cell_center - pulse_pos;
    double phase_correction = phase_error * 0.2f * m_current_gain;  // P control (no ID element)
    pulse_pos += phase_correction;

    // Cell size adjustment == frequency correction
    double freq_error = m_cell_center - pulse_pos;
    double new_cell_size = m_cell_size_ref - freq_error * 0.05f * m_current_gain;        // 'P' only control
    set_cell_size(new_cell_size);

    return pulse_pos;
}
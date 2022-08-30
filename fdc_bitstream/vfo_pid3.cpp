#include <iostream>
#include <fstream>
#include <string>

#include "vfo_pid3.h"

vfo_pid3::vfo_pid3() : vfo_base(), m_hist_ptr()
{ 
    reset();
    m_pulse_pos_history.resize(c_history_len);
    
    m_coeff_sum = 0.f;
    for(double i=1; i<= c_history_len; i+=1.f) {
        m_coeff_sum += i;
    }
}

/**
 * @brief Read PID coefficient from a file 'vfo_settings.txt'.
 * 
 */
void vfo_pid3::read_coeff(void) {
    std::ifstream ifs("vfo_settings.txt");
    if(ifs.is_open()) {
        std::string tmp;
        ifs >> tmp; m_phase_err_PC = std::stod(tmp);
        ifs >> tmp; m_phase_err_IC = std::stod(tmp);
        ifs >> tmp; m_phase_err_DC = std::stod(tmp);
        ifs.close();
    }
}

void vfo_pid3::disp_vfo_status(void) {
    vfo_base::disp_vfo_status();
    std::cout << "-- vfo_pid3 --" << std::endl;
    std::cout << "Prev pulse pos : " << m_prev_pulse_pos << std::endl;
    std::cout << "Prev phase err : " << m_prev_phase_err << std::endl;
    std::cout << "Phase err  prev     : " << m_prev_phase_err << std::endl;
    std::cout << "Phase err  integral : " << m_phase_err_I << std::endl;
    std::cout << "-- PID coefficients --" << std::endl;
    std::cout << "Phase err  P,I,D coeff  : " << m_phase_err_PC   << ", " << m_phase_err_IC  << ", " << m_phase_err_DC  << std::endl;
}

void vfo_pid3::reset(void) {
    vfo_base::reset();
    soft_reset();
}

void vfo_pid3::soft_reset(void) {
    vfo_base::soft_reset();
    m_prev_pulse_pos = 0.f;
    
    m_prev_phase_err = 0.f;
    m_phase_err_I = 0.f;

    m_prev_phase_diff = 0.f;
    m_phase_diff_I = 0.f;
#if 1
    m_phase_err_PC =1.f/4.f;
    m_phase_err_IC =1.f/128.f;
    m_phase_err_DC =1.f/512.f;
#endif

// 1/4  0.25
// 1/8  0.125
// 1/16 0.0625
// 1/32 0.03125
// 1/64 0.015625
// 1/128 0.0078125
// 1/256 0.00390625
// 1/512 0.001953125

    read_coeff();       // Read PID coefficit from a file.

    for(auto it = m_pulse_pos_history.begin(); it!=m_pulse_pos_history.end(); it++) {
        *it = m_cell_size_ref / 2.f;
    }
}

/**
 * @brief Calculate new pulse position based on the current bit cell parameters. Adjust pulse position, and calculate and optimize bit cell parameters.
 * 
 * @param pulse_pos Pulse position (the distance to the next pulse position in bit unit).
 * @return double Adjusted pulse position.
 *            0123456789abcdef0
 *  Bit cell  |               |
 *  Window    |   WWWWWWWW    |
 *  Center    |       ^       |
 *  Pulse     |     |         |
 *  Error     |     |-|       |
 * 
 *  Phase correction : shift next pulse position to adjust the phase.
 *  Freq. correction : change bit cell size by accumlated (integrated) error.
 */
double vfo_pid3::calc(double pulse_pos) {
    //LPF
    m_hist_ptr = (++m_hist_ptr) & (c_history_len-1);
    m_pulse_pos_history[m_hist_ptr] = pulse_pos;
    double sum = 0.f;
    for(size_t i=1; i <= c_history_len; i++) {
        sum += m_pulse_pos_history[ (m_hist_ptr+i) & (c_history_len-1) ] * i;
    }
    double avg = sum / m_coeff_sum;

    // Phase error from the center of the bit cell
    double phase_err_P  = m_cell_center - avg;
    double phase_err_D  = phase_err_P - m_prev_phase_err;
    m_phase_err_I      += phase_err_P;
    m_prev_phase_err    = phase_err_P;

    // Cell size adjustment == frequency correction + phase adjust
    double new_cell_size = m_cell_size_ref 
                - (phase_err_P  * m_phase_err_PC  - phase_err_D  * m_phase_err_DC  + m_phase_err_I  * m_phase_err_IC ) * m_current_gain;   // Phase control (PID)

    // FDD spindle variation (long term) = 2-2.5%
    // FDD spindle wow/flutter variation (short term)= +-2-2.5%
    // fast-slow combination = (2.5+2.5) *2 = 10%
    // VFO oscillation center frequency variation by temp and power supply voltage variation = 5%
    // Varition in total = 15% 
    //constexpr double tolerance = 0.5f;
    constexpr double tolerance = 0.8f;
    new_cell_size = limit(new_cell_size, m_cell_size_ref * (1.f/(1.f + tolerance)) , m_cell_size_ref * (1.0f + tolerance));

    set_cell_size(new_cell_size);
    m_prev_pulse_pos = pulse_pos;

    return pulse_pos;
}
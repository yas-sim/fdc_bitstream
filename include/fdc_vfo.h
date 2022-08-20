#pragma once

#define DLL_BODY
#include "dll_export.h"

class vfo_base {
public:
    double  m_cell_size;
    double  m_cell_size_ref;


    double  m_window_ratio;    // data window width ratio to the data cell width (0.75 means window = cell*0.75 )
    double  m_window_size;
    double  m_window_ofst;
    double  m_cell_center;

    double  m_gain_l;
    double  m_gain_h;
    double  m_current_gain;
    enum gain_state {
        low = 0,
        high = 1
    };
public:
    vfo_base() {};

    virtual void disp_vfo_status(void) {
        std::cout << "-- vfo_base --" << std::endl;
        std::cout << "Cell_size : " << m_cell_size << std::endl;
        std::cout << "Cell_size_ref : " << m_cell_size_ref << std::endl;
        std::cout << "Window ratio  : " << m_window_ratio << std::endl;
        std::cout << "Window size   : " << m_window_size << std::endl;
        std::cout << "Windoe offset : " << m_window_ofst << std::endl;
        std::cout << "Gain (Low)    : " << m_gain_l << std::endl;
        std::cout << "Gain (High)   : " << m_gain_h << std::endl;
        std::cout << "Current gain  : " << m_current_gain << std::endl;
    }

    virtual void reset(void) {
        set_params(4e6, 500e3);
        set_gain_val(1.f, 2.f);
    };

    inline double limit(double val, double lower_limit, double upper_limit) {
        if(val < lower_limit) val = lower_limit;
        if(val > upper_limit) val = upper_limit;
        return val;
    }

    virtual void set_params(size_t sampling_rate, size_t fdc_bit_rate, double data_window_ratio = 0.75f) {
        m_cell_size_ref = static_cast<double>(sampling_rate) / static_cast<double>(fdc_bit_rate);
        m_cell_size = m_cell_size_ref;

        data_window_ratio = limit(data_window_ratio, 0.3f, 0.9f);
        m_window_ratio = data_window_ratio;
        update_cell_params();
    }

    virtual void set_cell_size(double cell_size) {
        constexpr double tolerance = 0.4f;
        cell_size = limit(cell_size, m_cell_size_ref * (1.f - tolerance), m_cell_size_ref * (1.f + tolerance));
        m_cell_size = cell_size;
        update_cell_params();
    }

    /**
     * @brief Update cell related parameters.
     *   012345678
     *   | WWWW  |
     *     <-->    Window size (4)
     *   <->       Window ofst (2)
     *   <------>  Cell size   (8)
     */
     virtual void update_cell_params(void) {
        m_window_size = m_cell_size * m_window_ratio;               // typical window_ratio for actual drive is 0.5 but uses 0.75 as default in this SW
        m_window_size = m_window_size < 1.f ? 1.f : m_window_size;  // Avoid too narrow m_data_window_size situation
        m_window_ofst = (m_cell_size - m_window_size) / 2.f;
        m_cell_center = m_cell_size / 2.f; 
    }

    void set_gain_val(double gain_l, double gain_h) { m_gain_l = gain_l; m_gain_h = gain_h; }

    void set_gain_mode(gain_state state) {
        switch(state) {
        case gain_state::low:
        default:
            m_current_gain = m_gain_l;
            break;
        case gain_state::high:
            m_current_gain = m_gain_h;
            break;
        }
    }

    /**
     * @brief Construct a new calc object
     * 
     * @param pulse_pos Pulse position in the data cell
     */
    virtual double calc(double pulse_pos) = 0;
};



class fdc_vfo1 : public vfo_base {
public:
    double m_prev_phase_error;
    double m_prev_freq_error;
    double m_freq_bias;

public:
    fdc_vfo1() {
        reset();
    }

    virtual void disp_vfo_status(void) override {
        vfo_base::disp_vfo_status();
        std::cout << "-- fdc_vfo1 --" << std::endl;
        std::cout << "Prev phase err : " << m_prev_phase_error << std::endl;
        std::cout << "Prev freq err  : " << m_prev_freq_error << std::endl;
        std::cout << "Freq bias      : " << m_freq_bias << std::endl;
    }

    void reset(void) override {
        vfo_base::reset();
        m_prev_phase_error = 0.f;
        m_prev_freq_error = 0.f;
        m_freq_bias = 0.f;
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
    double calc(double pulse_pos) override {

        // Data pulse position adjustment == phase correction
        double phase_error = m_cell_center - pulse_pos;
        constexpr double phase_error_limitter = 0.25f;
        phase_error = limit(phase_error, -m_cell_size_ref * phase_error_limitter, 
                                          m_cell_size_ref * phase_error_limitter);
        double phase_err_diff =  phase_error - m_prev_phase_error;
        double phase_correction = (phase_error * 0.02f /*- phase_err_diff * 0.002f*/) * m_current_gain;  // PD control (no I element)
        constexpr double phase_correction_limitter = 0.125f;
        phase_correction = limit(phase_correction, -m_cell_size_ref * phase_correction_limitter, 
                                                    m_cell_size_ref * phase_correction_limitter);
        pulse_pos += phase_correction;
        m_prev_phase_error = phase_error;

        // Cell size adjustment == frequency correction
        double freq_error = m_cell_center - pulse_pos;
        m_freq_bias += freq_error;                                                              // 'I' element
        double new_cell_size = m_cell_size_ref - (m_freq_bias * 0.01f) * m_current_gain;        // 'I' only control
        set_cell_size(new_cell_size);

        return pulse_pos;
    }
};

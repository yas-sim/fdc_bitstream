/**
 * @brief VFO for data separator in mfm_codec
 * 
 */

/**
 * @brief Base class of VFO (virtual).
 * 
 */
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
public:
    enum gain_state {
        low = 0,
        high = 1
    };
public:
    vfo_base() {};
    virtual void disp_vfo_status(void);
    virtual void reset(void);
    inline double limit(double val, double lower_limit, double upper_limit) {
        if(val < lower_limit) val = lower_limit;
        if(val > upper_limit) val = upper_limit;
        return val;
    }
    virtual void set_params(size_t sampling_rate, size_t fdc_bit_rate, double data_window_ratio = 0.75f);
    virtual void set_cell_size(double cell_size);
    virtual void update_cell_params(void);
    void set_gain_val(double gain_l, double gain_h);
    void set_gain_mode(gain_state state);
    virtual double calc(double pulse_pos) = 0;
};



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



/**
 * @brief VFO class using PID control (2)
 * 
 */
class vfo_pid2 : public vfo_base {
public:
    double m_prev_phase_error;
    double m_phase_integral;
    double m_prev_freq_error;
    double m_freq_integral;
public:
    void disp_vfo_status(void) override;
    void reset(void) override;
    double calc(double pulse_pos) override;
};



/**
 * @brief Fixed VFO class (no VFO)
 * 
 */
class vfo_fixed : public vfo_base {
public:
    void disp_vfo_status(void) override {
        vfo_base::disp_vfo_status();
        std::cout << "-- vfo_fixed --" << std::endl;
    }
    double calc(double pulse_pos) override { return m_cell_center; }
};

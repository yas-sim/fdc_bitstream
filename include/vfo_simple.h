#pragma once

#include "fdc_vfo_base.h"

/**
 * @brief VFO class using simple proportional error correction (P)
 * 
 */
class vfo_simple : public vfo_base {
public:
    void disp_vfo_status(void) override {
        vfo_base::disp_vfo_status();
        std::cout << "-- vfo_simple --" << std::endl;
    }
    double calc(double pulse_pos) override;
};

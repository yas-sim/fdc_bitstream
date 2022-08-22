#pragma once

#define VFO_TYPE_SIMPLE         (0)
#define VFO_TYPE_FIXED          (1)
#define VFO_TYPE_PID            (2)
#define VFO_TYPE_PID2           (3)
#define VFO_TYPE_EXPERIMANTAL   (9)

/**
 * @brief Default VFD type.
 * 
 */
#define VFO_TYPE_DEFAULT        VFO_TYPE_PID

#define VFO_TYPE_DESC_STR  "0:vfo_simple, 1:vfo_fixed, 2:vfo_pid, 3:vfo_pid2, 9:vfo_experimental"

#define VFO_GAIN_L_DEFAULT  (1.f)
#define VFO_GAIN_H_DEFAULT  (2.f)

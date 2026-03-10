#include "balance.h"

balance_state_t mm_balance_update(float accel_x_g, float accel_y_g, float threshold_g) {
    if (accel_x_g < -threshold_g) {
        return BALANCE_STATE_TILT_LEFT;
    } else if (accel_x_g > threshold_g) {
        return BALANCE_STATE_TILT_RIGHT;
    }

    if (accel_y_g > threshold_g) {
        return BALANCE_STATE_TILT_FORWARD;
    } else if (accel_y_g < -threshold_g) {
        return BALANCE_STATE_TILT_BACKWARD;
    }

    return BALANCE_STATE_LEVEL;
}

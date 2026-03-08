#include <string.h>
#include "motion_processor.h"
#include "../filter/motion_filter.h"
#include "../fsm/motion_fsm.h"
#include "../balance/motion_balance.h"

#define TILT_THRESHOLD_G 0.05f

void mm_processor_init(motion_processor_t *proc, float alpha) {
    if (!proc) return;
    memset(proc, 0, sizeof(motion_processor_t));
    proc->alpha = alpha;
    proc->gravity_offset_z = 1.0f; // Default baseline
}

void mm_processor_run(motion_processor_t *proc, const mpu6050_scaled_data_t *raw, motion_metrics_t *metrics) {
    if (!proc || !raw || !metrics) return;

    // 1. Filter
    mm_filter_apply(raw, &proc->filtered_raw, proc->alpha);

    // 2. Metrics calculation
    mm_filter_calculate_metrics(&proc->filtered_raw, proc->gravity_offset_z, 
                                &metrics->lin_accel_z, &metrics->shake_mag);
    
    metrics->gyro_roll_dps = proc->filtered_raw.gyro_x_dps;
    metrics->gyro_pitch_dps = proc->filtered_raw.gyro_y_dps;

    // 3. Motion State (FSM)
    metrics->state = mm_fsm_update(metrics->state, metrics->lin_accel_z, 
                                   &proc->fsm_timer_ms, &proc->fsm_pending);

    // 4. Balance State (Relative to calibrated baseline)
    float relative_x = proc->filtered_raw.accel_x_g - proc->offset_x;
    float relative_y = proc->filtered_raw.accel_y_g - proc->offset_y;

    metrics->balance = mm_balance_update(relative_x, relative_y, TILT_THRESHOLD_G);
}

void mm_processor_set_baseline(motion_processor_t *proc, float x, float y, float z) {
    if (proc) {
        proc->offset_x = x;
        proc->offset_y = y;
        proc->gravity_offset_z = z;
    }
}

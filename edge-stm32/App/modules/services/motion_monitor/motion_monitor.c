/**
 * @file motion_monitor.c
 * @brief Professional Motion & Safety Monitor for ESMU Edge Node
 */

#include "motion_monitor.h"
#include "core/motion_core.h"
#include "system_registry.h"
#include "edge_logger.h"

// ─────────────────────────────────────────────
// Global Functions
// ─────────────────────────────────────────────

bool motion_monitor_init(void) {
    return motion_core_init();
}

bool motion_monitor_start(void) {
    return motion_core_start();
}

void motion_monitor_calibrate(void) {
    motion_core_calibrate();
}

void motion_monitor_get_metrics(motion_metrics_t *metrics) {
    if (!metrics) return;
    
    kinematics_data_t kin;
    detector_result_t det;
    motion_core_get_results(&kin, &det);
    
    metrics->state = kin.state;
    metrics->health_status = det.current_health;
    // Vibration/Score data can be mapped back from registry if needed
}

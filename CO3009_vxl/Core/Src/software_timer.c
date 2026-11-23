
#include "software_timer.h"

/*
 * CO3009 - Software Timer Module
 *
 * Four independent software timers
 * with 10 ms resolution.
 */

// Timer expiration flags
int timer_flag[MAX_SW_TIMERS] = {0};

/*
 * Functions to implement:
 *
 * setTimer(index, duration_ms)
 * clearTimer(index)
 * timerRun()
 *
 * timerRun() will be registered as a
 * periodic scheduler task with period
 * of 1 tick (10 ms).
 */

/*
 * Timer implementation will be added
 * after the scheduler is operational.
 */

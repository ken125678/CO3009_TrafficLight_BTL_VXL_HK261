
#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

// Number of software timers
#define MAX_SW_TIMERS 4

// Scheduler tick duration in milliseconds
#define TICK_MS 10

// Timer expiration flags
extern int timer_flag[MAX_SW_TIMERS];

// Set a software timer
void setTimer(int index, int duration_ms);

// Stop and reset a software timer
void clearTimer(int index);

// Update all software timers
void timerRun(void);

#endif /* INC_SOFTWARE_TIMER_H_ */

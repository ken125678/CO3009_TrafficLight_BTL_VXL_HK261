
#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

// Maximum number of registered tasks
#define SCH_MAX_TASKS 15

// Invalid task ID
#define NO_TASK_ID 0

// Scheduler task structure
typedef struct {
    void (*pTask)(void);

    // Relative delay in ticks
    uint32_t Delay;

    // Task period in ticks (0 = one-shot)
    uint32_t Period;

    // Execution flag
    uint8_t RunMe;

    // Unique task identifier
    uint32_t TaskID;

} sTask;

// Initialize the scheduler
void SCH_Init(void);

// Register a new task
uint32_t SCH_Add_Task(
    void (*pFunction)(void),
    uint32_t DELAY_TICKS,
    uint32_t PERIOD_TICKS
);

// Delete a task by its ID
uint8_t SCH_Delete_Task(uint32_t taskID);

// Update scheduler at each timer tick
void SCH_Update(void);

// Execute ready tasks in the main loop
void SCH_Dispatch_Tasks(void);

#endif /* INC_SCHEDULER_H_ */

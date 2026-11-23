
#include "scheduler.h"
#include "stm32f1xx_hal.h"
#include <stddef.h>

/* ===== Scheduler Data ===== */

// Relative-delay queue
volatile sTask SCH_tasks_G[SCH_MAX_TASKS];

// Number of registered tasks
volatile uint8_t SCH_count = 0;

// Next available task ID
static uint32_t SCH_next_task_id = 1U;


/* ===== SCH_Init ===== */

void SCH_Init(void)
{
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();

    SCH_count = 0U;
    SCH_next_task_id = 1U;

    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++)
    {
        SCH_tasks_G[i].pTask = NULL;
        SCH_tasks_G[i].Delay = 0U;
        SCH_tasks_G[i].Period = 0U;
        SCH_tasks_G[i].RunMe = 0U;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }

    __set_PRIMASK(irq_state);
}


/* ===== SCH_Add_Task ===== */

uint32_t SCH_Add_Task(
    void (*pFunction)(void),
    uint32_t DELAY_TICKS,
    uint32_t PERIOD_TICKS)
{
    // Reject a NULL callback
    if (pFunction == NULL)
    {
        return NO_TASK_ID;
    }

    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();

    // Check queue capacity
    if (SCH_count >= SCH_MAX_TASKS)
    {
        __set_PRIMASK(irq_state);
        return NO_TASK_ID;
    }

    uint32_t remaining = DELAY_TICKS;
    uint32_t position = 0U;

    // Find the correct insertion position
    while (position < SCH_count &&
           remaining >= SCH_tasks_G[position].Delay)
    {
        remaining -= SCH_tasks_G[position].Delay;
        position++;
    }

    // Shift tasks to the right
    for (uint32_t j = SCH_count; j > position; j--)
    {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1U];
    }

    // Generate a nonzero task ID
    uint32_t new_id = SCH_next_task_id++;

    if (SCH_next_task_id == NO_TASK_ID)
    {
        SCH_next_task_id = 1U;
    }

    // Insert the new task
    SCH_tasks_G[position].pTask = pFunction;
    SCH_tasks_G[position].Delay = remaining;
    SCH_tasks_G[position].Period = PERIOD_TICKS;
    SCH_tasks_G[position].RunMe = 0U;
    SCH_tasks_G[position].TaskID = new_id;

    // Preserve the next task's execution time
    if (position < SCH_count)
    {
        SCH_tasks_G[position + 1U].Delay -= remaining;
    }

    SCH_count++;

    // Restore previous interrupt state
    __set_PRIMASK(irq_state);

    return new_id;
}

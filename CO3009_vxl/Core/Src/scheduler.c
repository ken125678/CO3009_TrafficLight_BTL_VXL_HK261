#include "scheduler.h"
#include "stm32f1xx_hal.h"
#include <stddef.h>

/* Sorted relative-delay queue. TIM2 ISR only updates queue head. */
volatile sTask SCH_tasks_G[SCH_MAX_TASKS];
volatile uint8_t SCH_count = 0U;
static uint32_t SCH_next_task_id = 1U;

/* Call with interrupts already masked. */
static uint8_t SCH_InsertRelative(void (*function)(void),
                                  uint32_t delay,
                                  uint32_t period,
                                  uint32_t id)
{
    if (function == NULL || SCH_count >= SCH_MAX_TASKS || id == NO_TASK_ID)
        return 0U;

    uint32_t position = 0U;
    while (position < SCH_count && delay >= SCH_tasks_G[position].Delay)
    {
        delay -= SCH_tasks_G[position].Delay;
        position++;
    }

    for (uint32_t j = SCH_count; j > position; --j)
        SCH_tasks_G[j] = SCH_tasks_G[j - 1U];

    SCH_tasks_G[position].pTask = function;
    SCH_tasks_G[position].Delay = delay;
    SCH_tasks_G[position].Period = period;
    SCH_tasks_G[position].RunMe = 0U;
    SCH_tasks_G[position].TaskID = id;

    if (position < SCH_count)
        SCH_tasks_G[position + 1U].Delay -= delay;

    SCH_count++;
    return 1U;
}

/* Call with interrupts already masked, and valid index < SCH_count. */
static void SCH_RemoveAt(uint32_t index)
{
    if (index + 1U < SCH_count)
        SCH_tasks_G[index + 1U].Delay += SCH_tasks_G[index].Delay;

    for (uint32_t i = index; i + 1U < SCH_count; ++i)
        SCH_tasks_G[i] = SCH_tasks_G[i + 1U];

    SCH_count--;
    SCH_tasks_G[SCH_count].pTask = NULL;
    SCH_tasks_G[SCH_count].Delay = 0U;
    SCH_tasks_G[SCH_count].Period = 0U;
    SCH_tasks_G[SCH_count].RunMe = 0U;
    SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;
}

void SCH_Init(void)
{
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();
    SCH_count = 0U;
    SCH_next_task_id = 1U;
    for (uint32_t i = 0U; i < SCH_MAX_TASKS; ++i)
    {
        SCH_tasks_G[i].pTask = NULL;
        SCH_tasks_G[i].Delay = 0U;
        SCH_tasks_G[i].Period = 0U;
        SCH_tasks_G[i].RunMe = 0U;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }
    __set_PRIMASK(irq_state);
}

uint32_t SCH_Add_Task(void (*pFunction)(void),
                       uint32_t DELAY_TICKS,
                       uint32_t PERIOD_TICKS)
{
    if (pFunction == NULL)
        return NO_TASK_ID;

    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();

    if (SCH_count >= SCH_MAX_TASKS)
    {
        __set_PRIMASK(irq_state);
        return NO_TASK_ID;
    }

    uint32_t id = SCH_next_task_id++;
    if (SCH_next_task_id == NO_TASK_ID)
        SCH_next_task_id = 1U;

    if (!SCH_InsertRelative(pFunction, DELAY_TICKS, PERIOD_TICKS, id))
        id = NO_TASK_ID;

    __set_PRIMASK(irq_state);
    return id;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == NO_TASK_ID)
        return 0U;

    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();

    for (uint32_t i = 0U; i < SCH_count; ++i)
    {
        if (SCH_tasks_G[i].TaskID == taskID)
        {
            SCH_RemoveAt(i);
            __set_PRIMASK(irq_state);
            return 1U;
        }
    }

    __set_PRIMASK(irq_state);
    return 0U;
}

void SCH_Update(void)
{
    if (SCH_count == 0U)
        return;

    if (SCH_tasks_G[0].Delay > 0U)
        --SCH_tasks_G[0].Delay;

    if (SCH_tasks_G[0].Delay == 0U)
        SCH_tasks_G[0].RunMe = 1U;
}

void SCH_Dispatch_Tasks(void)
{
    for (;;)
    {
        uint32_t irq_state = __get_PRIMASK();
        __disable_irq();

        if (SCH_count == 0U || SCH_tasks_G[0].Delay != 0U)
        {
            __set_PRIMASK(irq_state);
            break;
        }

        /* A zero-delay task exposed behind another due task is due too. */
        SCH_tasks_G[0].RunMe = 1U;
        sTask due = SCH_tasks_G[0];
        SCH_RemoveAt(0U);

        /* Keep the TaskID stable for deleting a periodic task later. */
        if (due.Period > 0U)
            (void)SCH_InsertRelative(due.pTask, due.Period,
                                     due.Period, due.TaskID);

        __set_PRIMASK(irq_state);

        /* Callback runs only in main context with interrupts enabled. */
        due.pTask();
    }
}


#include "software_timer.h"

/* Four independent software timers */

/* Remaining time in scheduler ticks */
static int timer_counter[MAX_SW_TIMERS] = {0};

/* Expiration flags */
int timer_flag[MAX_SW_TIMERS] = {0};


/* Check whether timer index is valid */
static int isValidTimer(int index)
{
    return (index >= 0 && index < MAX_SW_TIMERS);
}


/* Start or restart a software timer */
void setTimer(int index, int duration_ms)
{
    if (!isValidTimer(index))
    {
        return;
    }

    /* Clear the previous expiration flag */
    timer_flag[index] = 0;

    /* Non-positive duration: expire immediately */
    if (duration_ms <= 0)
    {
        timer_counter[index] = 0;
        timer_flag[index] = 1;
        return;
    }

    /* Convert milliseconds to ticks, rounding up */
    int ticks = duration_ms / TICK_MS;

    if ((duration_ms % TICK_MS) != 0)
    {
        ticks++;
    }

    timer_counter[index] = ticks;
}


/* Cancel an active timer */
void clearTimer(int index)
{
    if (!isValidTimer(index))
    {
        return;
    }

    timer_counter[index] = 0;
    timer_flag[index] = 0;
}


/* Called periodically every 10 ms */
void timerRun(void)
{
    for (int i = 0; i < MAX_SW_TIMERS; i++)
    {
        if (timer_counter[i] > 0)
        {
            timer_counter[i]--;

            if (timer_counter[i] == 0)
            {
                timer_flag[i] = 1;
            }
        }
    }
}

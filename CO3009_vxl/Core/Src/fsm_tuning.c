#include "fsm_tuning.h"
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"

static void enter_auto_from_tuning(void) {
    clearTimer(TIMER_BLINK);
    traffic_set(ROAD_1, LIGHT_RED);
    traffic_set(ROAD_2, LIGHT_GREEN);
    count_road1 = time_red;
    count_road2 = time_green;
    setTimer(TIMER_PHASE, time_green * 1000);
    setTimer(TIMER_1SEC, 1000);
    status = AUTO_RED1_GREEN2;
    clearAllButtonFlags();
}

static void validate_and_commit_durations(void) {
    /* Enforce time_red == time_green + time_amber with all durations in [1, 99]. */
    if (temp_red == (temp_green + temp_amber) &&
        temp_red >= 2 && temp_red <= 99 &&
        temp_green >= 1 && temp_amber >= 1) {
        time_red = temp_red;
        time_green = temp_green;
        time_amber = temp_amber;
    } else {
        int g = (temp_green < 1) ? 1 : (temp_green > 98 ? 98 : temp_green);
        int a = (temp_amber < 1) ? 1 : (temp_amber > 98 ? 98 : temp_amber);
        if (g + a > 99) {
            g = 99 - a;
        }
        time_green = g;
        time_amber = a;
        time_red = g + a;
    }
}

void fsm_tuning_run(void) {
    int *target = 0;
    LightColor_t blink_color = LIGHT_OFF;

    switch (status) {
        case TUNE_RED:
            target = &temp_red;
            blink_color = LIGHT_RED;
            break;
        case TUNE_GREEN:
            target = &temp_green;
            blink_color = LIGHT_GREEN;
            break;
        case TUNE_AMBER:
            target = &temp_amber;
            blink_color = LIGHT_AMBER;
            break;
        default:
            break;
    }

    if (target == 0) {
        return;
    }

    if (timer_flag[TIMER_BLINK]) {
        traffic_toggle(ROAD_1, blink_color);
        traffic_toggle(ROAD_2, blink_color);
        setTimer(TIMER_BLINK, 250);
    }

    if (isButtonPressed(BTN_CONFIRM)) {
        validate_and_commit_durations();
        enter_auto_from_tuning();
        return;
    } else if (isButtonPressed(BTN_MODE)) {
        if (status == TUNE_RED) {
            status = TUNE_GREEN;
            traffic_set(ROAD_1, LIGHT_GREEN);
            traffic_set(ROAD_2, LIGHT_GREEN);
            setTimer(TIMER_BLINK, 250);
            clearAllButtonFlags();
        } else if (status == TUNE_GREEN) {
            status = TUNE_AMBER;
            traffic_set(ROAD_1, LIGHT_AMBER);
            traffic_set(ROAD_2, LIGHT_AMBER);
            setTimer(TIMER_BLINK, 250);
            clearAllButtonFlags();
        } else {
            enter_auto_from_tuning();
        }
        return;
    } else if (isButtonPressed(BTN_USER_IDX)) {
        temp_red = 5;
        temp_green = 3;
        temp_amber = 2;
        clearAllButtonFlags();
        return;
    }

    if (isButtonPressed(BTN_INC_SWITCH) || isButtonLongPressed(BTN_INC_SWITCH)) {
        if (*target < 99) {
            (*target)++;
        }
    }
    if (isButtonPressed(BTN_DEC) || isButtonLongPressed(BTN_DEC)) {
        if (*target > 1) {
            (*target)--;
        }
    }
}

#include "fsm_manual.h"
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"

static void enter_tuning_from_manual(void) {
    clearTimer(TIMER_PHASE);
    clearTimer(TIMER_1SEC);
    temp_red = time_red;
    temp_green = time_green;
    temp_amber = time_amber;
    traffic_set(ROAD_1, LIGHT_RED);
    traffic_set(ROAD_2, LIGHT_RED);
    setTimer(TIMER_BLINK, 250);
    status = TUNE_RED;
    clearAllButtonFlags();
}

static void enter_auto_from_manual(void) {
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

void fsm_manual_run(void) {
    switch (status) {
        case MAN_RED1_GREEN2:
            if (isButtonPressed(BTN_MODE)) {
                enter_tuning_from_manual();
                break;
            }
            if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_from_manual();
                break;
            }
            if (isButtonPressed(BTN_INC_SWITCH)) {
                /* Mandatory amber transition prevents simultaneous green on both roads. */
                status = MAN_RED1_AMBER2;
                traffic_set(ROAD_1, LIGHT_RED);
                traffic_set(ROAD_2, LIGHT_AMBER);
                setTimer(TIMER_PHASE, time_amber * 1000);
                clearAllButtonFlags();
            }
            break;

        case MAN_GREEN1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                enter_tuning_from_manual();
                break;
            }
            if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_from_manual();
                break;
            }
            if (isButtonPressed(BTN_INC_SWITCH)) {
                status = MAN_AMBER1_RED2;
                traffic_set(ROAD_1, LIGHT_AMBER);
                traffic_set(ROAD_2, LIGHT_RED);
                setTimer(TIMER_PHASE, time_amber * 1000);
                clearAllButtonFlags();
            }
            break;

        case MAN_RED1_AMBER2:
            if (isButtonPressed(BTN_MODE)) {
                enter_tuning_from_manual();
                break;
            }
            if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_from_manual();
                break;
            }
            /* Safety interlock: discard switch requests during mandatory amber clearance. */
            (void)isButtonPressed(BTN_INC_SWITCH);
            if (timer_flag[TIMER_PHASE]) {
                status = MAN_GREEN1_RED2;
                traffic_set(ROAD_1, LIGHT_GREEN);
                traffic_set(ROAD_2, LIGHT_RED);
                clearTimer(TIMER_PHASE);
                clearAllButtonFlags();
            }
            break;

        case MAN_AMBER1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                enter_tuning_from_manual();
                break;
            }
            if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_from_manual();
                break;
            }
            (void)isButtonPressed(BTN_INC_SWITCH);
            if (timer_flag[TIMER_PHASE]) {
                status = MAN_RED1_GREEN2;
                traffic_set(ROAD_1, LIGHT_RED);
                traffic_set(ROAD_2, LIGHT_GREEN);
                clearTimer(TIMER_PHASE);
                clearAllButtonFlags();
            }
            break;

        default:
            break;
    }
}

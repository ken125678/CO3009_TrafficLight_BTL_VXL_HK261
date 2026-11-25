#include "fsm_automatic.h"
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"

static void enter_auto_red1_green2(void) {
    status = AUTO_RED1_GREEN2;
    traffic_set(ROAD_1, LIGHT_RED);
    traffic_set(ROAD_2, LIGHT_GREEN);
    count_road1 = time_red;
    count_road2 = time_green;
    clearTimer(TIMER_BLINK);
    setTimer(TIMER_PHASE, time_green * 1000);
    setTimer(TIMER_1SEC, 1000);
}

static void enter_auto_red1_amber2(void) {
    status = AUTO_RED1_AMBER2;
    traffic_set(ROAD_1, LIGHT_RED);
    traffic_set(ROAD_2, LIGHT_AMBER);
    count_road1 = time_amber;
    count_road2 = time_amber;
    setTimer(TIMER_PHASE, time_amber * 1000);
    setTimer(TIMER_1SEC, 1000);
}

static void enter_auto_green1_red2(void) {
    status = AUTO_GREEN1_RED2;
    traffic_set(ROAD_1, LIGHT_GREEN);
    traffic_set(ROAD_2, LIGHT_RED);
    count_road1 = time_green;
    count_road2 = time_red;
    setTimer(TIMER_PHASE, time_green * 1000);
    setTimer(TIMER_1SEC, 1000);
}

static void enter_auto_amber1_red2(void) {
    status = AUTO_AMBER1_RED2;
    traffic_set(ROAD_1, LIGHT_AMBER);
    traffic_set(ROAD_2, LIGHT_RED);
    count_road1 = time_amber;
    count_road2 = time_amber;
    setTimer(TIMER_PHASE, time_amber * 1000);
    setTimer(TIMER_1SEC, 1000);
}

static void enter_manual_from_auto(SystemState_t next_state, LightColor_t road1_color, LightColor_t road2_color) {
    status = next_state;
    traffic_set(ROAD_1, road1_color);
    traffic_set(ROAD_2, road2_color);
    clearTimer(TIMER_PHASE);
    clearTimer(TIMER_1SEC);
    clearAllButtonFlags();
}

static void tick_countdown_1sec(void) {
    if (count_road1 > 0) {
        count_road1--;
    }
    if (count_road2 > 0) {
        count_road2--;
    }
    setTimer(TIMER_1SEC, 1000);
}

void fsm_automatic_run(void) {
    switch (status) {
        case INIT_SYSTEM:
            enter_auto_red1_green2();
            clearAllButtonFlags();
            break;

        case AUTO_RED1_GREEN2:
            if (isButtonPressed(BTN_MODE)) {
                /* Preserve active priority direction when entering manual mode. */
                enter_manual_from_auto(MAN_RED1_GREEN2, LIGHT_RED, LIGHT_GREEN);
                return;
            } else if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_red1_green2();
                clearAllButtonFlags();
                return;
            }

            /* Check phase timer before 1s tick to avoid decrementing on a phase transition. */
            if (timer_flag[TIMER_PHASE]) {
                enter_auto_red1_amber2();
            } else if (timer_flag[TIMER_1SEC]) {
                tick_countdown_1sec();
            }
            break;

        case AUTO_RED1_AMBER2:
            if (isButtonPressed(BTN_MODE)) {
                enter_manual_from_auto(MAN_RED1_GREEN2, LIGHT_RED, LIGHT_GREEN);
                return;
            } else if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_red1_green2();
                clearAllButtonFlags();
                return;
            }

            if (timer_flag[TIMER_PHASE]) {
                enter_auto_green1_red2();
            } else if (timer_flag[TIMER_1SEC]) {
                tick_countdown_1sec();
            }
            break;

        case AUTO_GREEN1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                enter_manual_from_auto(MAN_GREEN1_RED2, LIGHT_GREEN, LIGHT_RED);
                return;
            } else if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_red1_green2();
                clearAllButtonFlags();
                return;
            }

            if (timer_flag[TIMER_PHASE]) {
                enter_auto_amber1_red2();
            } else if (timer_flag[TIMER_1SEC]) {
                tick_countdown_1sec();
            }
            break;

        case AUTO_AMBER1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                enter_manual_from_auto(MAN_GREEN1_RED2, LIGHT_GREEN, LIGHT_RED);
                return;
            } else if (isButtonPressed(BTN_USER_IDX)) {
                enter_auto_red1_green2();
                clearAllButtonFlags();
                return;
            }

            if (timer_flag[TIMER_PHASE]) {
                enter_auto_red1_green2();
            } else if (timer_flag[TIMER_1SEC]) {
                tick_countdown_1sec();
            }
            break;

        default:
            break;
    }
}

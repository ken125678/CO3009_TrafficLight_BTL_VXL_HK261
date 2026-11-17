#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

typedef enum {
    INIT_SYSTEM = 0,
    AUTO_RED1_GREEN2,
    AUTO_RED1_AMBER2,
    AUTO_GREEN1_RED2,
    AUTO_AMBER1_RED2,
    MAN_RED1_GREEN2,
    MAN_RED1_AMBER2,
    MAN_GREEN1_RED2,
    MAN_AMBER1_RED2,
    TUNE_RED,
    TUNE_GREEN,
    TUNE_AMBER
} SystemState_t;

typedef enum {
    LIGHT_OFF = 0,
    LIGHT_RED,
    LIGHT_AMBER,
    LIGHT_GREEN
} LightColor_t;

#define BTN_MODE         0
#define BTN_INC_SWITCH   1
#define BTN_DEC          2
#define BTN_CONFIRM      3
#define BTN_USER_IDX     4

#define TIMER_PHASE      0
#define TIMER_1SEC       1
#define TIMER_BLINK      2
#define TIMER_MSG        3

/* Declared here; Member 3 will define these in global.c. */
extern SystemState_t status;
extern int time_red;
extern int time_green;
extern int time_amber;
extern int temp_red;
extern int temp_green;
extern int temp_amber;
extern int count_road1;
extern int count_road2;
extern float dht20_temp;
extern float dht20_humi;

void global_init(void);

#endif /* INC_GLOBAL_H_ */

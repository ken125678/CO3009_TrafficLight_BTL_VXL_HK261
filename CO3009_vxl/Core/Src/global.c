#include "global.h"

SystemState_t status = INIT_SYSTEM;
int time_red = 5;
int time_green = 3;
int time_amber = 2;
int temp_red = 5;
int temp_green = 3;
int temp_amber = 2;
int count_road1 = 0;
int count_road2 = 0;
float dht20_temp = 0.0f;
float dht20_humi = 0.0f;

void global_init(void) {
    status = INIT_SYSTEM;
    time_red = 5;
    time_green = 3;
    time_amber = 2;
    temp_red = 5;
    temp_green = 3;
    temp_amber = 2;
    count_road1 = 0;
    count_road2 = 0;
    dht20_temp = 0.0f;
    dht20_humi = 0.0f;
}

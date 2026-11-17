#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include "global.h"

#define ROAD_1 1
#define ROAD_2 2

void traffic_init(void);
void traffic_set(uint8_t road, LightColor_t color);
void traffic_toggle(uint8_t road, LightColor_t color);

#endif /* INC_TRAFFIC_LIGHT_H_ */

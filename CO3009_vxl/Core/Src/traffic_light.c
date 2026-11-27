#include "traffic_light.h"
#include "main.h"

#define SIGNAL_RED    0b00
#define SIGNAL_AMBER  0b01
#define SIGNAL_GREEN  0b10
#define SIGNAL_OFF    0b11

typedef struct {
    GPIO_TypeDef *port_a;
    uint16_t pin_a;
    GPIO_TypeDef *port_b;
    uint16_t pin_b;
} TrafficLight_t;

static TrafficLight_t road1 = {GPIOB, GPIO_PIN_3, GPIOB, GPIO_PIN_5};
static TrafficLight_t road2 = {GPIOB, GPIO_PIN_4, GPIOB, GPIO_PIN_10};

static LightColor_t road1_current = LIGHT_OFF;
static LightColor_t road2_current = LIGHT_OFF;

static TrafficLight_t* get_traffic_light(uint8_t road)
{
    if (road == ROAD_1) return &road1;
    if (road == ROAD_2) return &road2;
    return NULL;
}

static uint8_t get_color_signal(LightColor_t color)
{
    switch (color) {
        case LIGHT_RED:   return SIGNAL_RED;
        case LIGHT_AMBER: return SIGNAL_AMBER;
        case LIGHT_GREEN: return SIGNAL_GREEN;
        case LIGHT_OFF:  return SIGNAL_OFF;
        default:         return SIGNAL_OFF;
    }
}

static void write_traffic_signal(TrafficLight_t *light, uint8_t signal)
{
    GPIO_PinState state_a = (signal & 0b10) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState state_b = (signal & 0b01) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(light->port_a, light->pin_a, state_a);
    HAL_GPIO_WritePin(light->port_b, light->pin_b, state_b);
}

void traffic_set(uint8_t road, LightColor_t color)
{
    if (color < LIGHT_OFF || color > LIGHT_GREEN) return;

    TrafficLight_t *light = get_traffic_light(road);
    if (light == NULL) return;

    write_traffic_signal(light, get_color_signal(color));

    if (road == ROAD_1)
        road1_current = color;
    else
        road2_current = color;
}

void traffic_toggle(uint8_t road, LightColor_t color)
{
    if (road == ROAD_1) {
        traffic_set(ROAD_1, road1_current == color ? LIGHT_OFF : color);
    }
    else if (road == ROAD_2) {
        traffic_set(ROAD_2, road2_current == color ? LIGHT_OFF : color);
    }
}

void traffic_init(void)
{
    traffic_set(ROAD_1, LIGHT_OFF);
    traffic_set(ROAD_2, LIGHT_OFF);
}

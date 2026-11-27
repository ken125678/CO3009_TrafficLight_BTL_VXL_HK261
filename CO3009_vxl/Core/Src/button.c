#include "button.h"
#include "main.h"

#define DEBOUNCE_THRESHOLD  3
#define SAMPLE_TIME_MS     10
#define LONG_PRESS_MS      1000
#define REPEAT_INTERVAL    250

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;

    uint8_t stable_state;
    uint8_t sample_count;

    uint16_t hold_time;
    uint16_t repeat_time;

    uint8_t pressed_event;
    uint8_t long_press_event;
} Button_t;

static Button_t buttons[N_BUTTONS] = {
    {GPIOA, GPIO_PIN_8},   // BTN1
    {GPIOA, GPIO_PIN_9},   // BTN2
    {GPIOC, GPIO_PIN_7},   // BTN3
    {GPIOB, GPIO_PIN_6},   // BTN4
    {GPIOC, GPIO_PIN_13}   // USER
};

void button_init(void)
{
    for (uint8_t i = 0; i < N_BUTTONS; i++) {
        buttons[i].stable_state = 1;  // Released
        buttons[i].sample_count = 0;
        buttons[i].hold_time = 0;
        buttons[i].repeat_time = 0;
        buttons[i].pressed_event = 0;
        buttons[i].long_press_event = 0;
    }
}

static uint8_t button_read(int index)
{
    return HAL_GPIO_ReadPin(buttons[index].port,
                            buttons[index].pin) == GPIO_PIN_SET ? 1 : 0;
}

void button_reading(void)
{
    for (uint8_t i = 0; i < N_BUTTONS; i++) {
        uint8_t reading = button_read(i);

        // Debouncing
        if (reading != buttons[i].stable_state) {
            buttons[i].sample_count++;

            if (buttons[i].sample_count >= DEBOUNCE_THRESHOLD) {
                buttons[i].stable_state = reading;
                buttons[i].sample_count = 0;

                if (reading == 0) {
                    buttons[i].pressed_event = 1;
                }
            }
        }
        else {
            buttons[i].sample_count = 0;
        }

        // Long-press handling
        if (buttons[i].stable_state == 0) {
            if (buttons[i].hold_time < LONG_PRESS_MS) {
                buttons[i].hold_time += SAMPLE_TIME_MS;

                if (buttons[i].hold_time >= LONG_PRESS_MS) {
                    buttons[i].long_press_event = 1;
                    buttons[i].repeat_time = 0;
                }
            }
            else {
                buttons[i].repeat_time += SAMPLE_TIME_MS;

                if (buttons[i].repeat_time >= REPEAT_INTERVAL) {
                    buttons[i].long_press_event = 1;
                    buttons[i].repeat_time = 0;
                }
            }
        }
        else {
            buttons[i].hold_time = 0;
            buttons[i].repeat_time = 0;
        }
    }
}

int isButtonPressed(int index)
{
    if (index < 0 || index >= N_BUTTONS) return 0;

    if (buttons[index].pressed_event) {
        buttons[index].pressed_event = 0;
        return 1;
    }

    return 0;
}

int isButtonLongPressed(int index)
{
    if (index < 0 || index >= N_BUTTONS) return 0;

    if (buttons[index].long_press_event) {
        buttons[index].long_press_event = 0;
        return 1;
    }

    return 0;
}

void clearAllButtonFlags(void)
{
    for (uint8_t i = 0; i < N_BUTTONS; i++) {
        buttons[i].pressed_event = 0;
        buttons[i].long_press_event = 0;
    }
}

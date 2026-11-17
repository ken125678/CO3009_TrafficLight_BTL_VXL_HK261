#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#define N_BUTTONS      5
/* Confirm active-low behavior on the real hardware. */
#define PRESSED_STATE  0
#define RELEASED_STATE 1

void button_init(void);
void button_reading(void);
int isButtonPressed(int index);
int isButtonLongPressed(int index);
void clearAllButtonFlags(void);

#endif /* INC_BUTTON_H_ */

#ifndef JOYSPAD_H
#define JOYSPAD_H

#include <stdbool.h>
#include <psxpad.h>

void joypad_init(void);
void joypad_deinit(void);
bool joypad_is_pressed(PadButton button_to_check);
bool joypad_is_released(PadButton button_to_check);

#endif

#include "joypad.h"

#include <assert.h>
#include <stdbool.h>
#include <psxapi.h>
#include <sys/types.h>
#include <psxpad.h>

#define PAD_COUNT 2
#define PAD_BUFFER_SIZE 34

u_char pad_buffer[PAD_COUNT][PAD_BUFFER_SIZE];

void joypad_init(void) {
    assert(PAD_COUNT == 2);
    InitPAD(pad_buffer[0], PAD_BUFFER_SIZE, pad_buffer[1], PAD_BUFFER_SIZE);

    // Set to 0xFF, because 0x00 means "ready" or something like that
    pad_buffer[0][0] = pad_buffer[0][1] = 0xFF;
    pad_buffer[1][0] = pad_buffer[1][1] = 0xFF;

    StartPAD();
    ChangeClearPAD(1);
}

void joypad_deinit(void) {
    StopPAD();
}

bool joypad_is_pressed(PadButton button_to_check) {
    u_short pressed_button = *((u_short*) (pad_buffer[0] + 2));

    return !(pressed_button & button_to_check);
}

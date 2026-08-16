#include "mouse.h"
#include "matrix.h"
#include <string.h>

// defines
#define MOUSE_MOVEMENT_DELTA    (4)
#define MOUSE_WHEEL_DELTA       (4)

// typedefs
typedef struct mouse_states_t {
    bool left_click;
    bool middle_click;
    bool right_click;

    bool move_left;
    bool move_right;
    bool move_up;
    bool move_down;
} mouse_states_t;

// statics
static mouse_report_t* mouse_report = NULL;
static mouse_states_t mouse_states = {0};

// private functions
static void mouse_init(void* init_data) {
    mouse_report = (mouse_report_t*)init_data;
}

static void mouse_reset(void) {
    memset(&mouse_states, 0, sizeof(mouse_states));
}

static bool mouse_update(void) {
    if (mouse_states.left_click)   mouse_report->buttons |= MOUSE_BUTTON_LEFT;
    if (mouse_states.middle_click) mouse_report->buttons |= MOUSE_BUTTON_MIDDLE;
    if (mouse_states.right_click)  mouse_report->buttons |= MOUSE_BUTTON_RIGHT;
    if (mouse_states.move_left)    mouse_report->x = -MOUSE_MOVEMENT_DELTA;
    if (mouse_states.move_right)   mouse_report->x = MOUSE_MOVEMENT_DELTA;
    if (mouse_states.move_up)      mouse_report->y = -MOUSE_MOVEMENT_DELTA;
    if (mouse_states.move_down)    mouse_report->y = MOUSE_MOVEMENT_DELTA;
    return false;
}

static bool mouse_on_release(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) != ENTRY_TYPE_MOUSE) return false;

    switch (key & MOUSE_ACTION_MASK) {
        case MOUSE_ACTION_LEFT_CLICK:   mouse_states.left_click = false;    break;
        case MOUSE_ACTION_MIDDLE_CLICK: mouse_states.middle_click = false;  break;
        case MOUSE_ACTION_RIGHT_CLICK:  mouse_states.right_click = false;   break;
        case MOUSE_ACTION_MOVE_LEFT:    mouse_states.move_left = false;     break;
        case MOUSE_ACTION_MOVE_RIGHT:   mouse_states.move_right = false;    break;
        case MOUSE_ACTION_MOVE_UP:      mouse_states.move_up = false;       break;
        case MOUSE_ACTION_MOVE_DOWN:    mouse_states.move_down = false;     break;
        default: {
            // Ignore for now
        }
    }
}

static bool mouse_on_press(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) != ENTRY_TYPE_MOUSE) return false;

    switch (key & MOUSE_ACTION_MASK) {
        case MOUSE_ACTION_LEFT_CLICK:   mouse_states.left_click = true;     break;
        case MOUSE_ACTION_MIDDLE_CLICK: mouse_states.middle_click = true;   break;
        case MOUSE_ACTION_RIGHT_CLICK:  mouse_states.right_click = true;    break;
        case MOUSE_ACTION_MOVE_LEFT:    mouse_states.move_left = true;      break;
        case MOUSE_ACTION_MOVE_RIGHT:   mouse_states.move_right = true;     break;
        case MOUSE_ACTION_MOVE_UP:      mouse_states.move_up = true;        break;
        case MOUSE_ACTION_MOVE_DOWN:    mouse_states.move_down = true;      break;
        default: {
            // Ignore for now
        }
    }
}

static bool mouse_on_virtual_press(keymap_entry_t key) {
    mouse_on_press(0xff, 0xff, key);
}

// public functions
const keyboard_system_t* mouse_system(void) {
    static const keyboard_system_t system = {
        .name = "mouse",
        .init = mouse_init,
        .reset = mouse_reset,
        .update = mouse_update,
        .on_press = mouse_on_press,
        .on_virtual_press = mouse_on_virtual_press,
        .on_release = mouse_on_release,
    };

    return &system;
}

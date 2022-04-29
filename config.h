#pragma once

// Lets you roll mod-tap keys
#define IGNORE_MOD_TAP_INTERRUPT
#define PERMISSIVE_HOLD_PER_KEY
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define TAPPING_FORCE_HOLD_PER_KEY

#ifdef TAPPING_TERM
#undef TAPPING_TERM
#endif
#define TAPPING_TERM 199
//#define PERMISSIVE_HOLD
#undef MOUSEKEY_INTERVAL
#define MOUSEKEY_INTERVAL 15
// The default is 19. Since we made the mouse about 3 times faster with the previous setting,
// give it more time to accelerate to max speed to retain precise control over short distances.
#undef MOUSEKEY_TIME_TO_MAX
#define MOUSEKEY_TIME_TO_MAX 39
// The default is 299. Let's try and make this as low as possible while keeping the cursor responsive
#undef MOUSEKEY_DELAY
#define MOUSEKEY_DELAY 99
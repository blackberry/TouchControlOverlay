/*
 * emulate.h
 *
 *  Created on: Jul 27, 2011
 *      Author: jnicholl
 */

#ifndef _EMULATE_EMULATE_H_INCLUDED
#define _EMULATE_EMULATE_H_INCLUDED

#include <sys/platform.h>
#include <screen/screen.h>

__BEGIN_DECLS

/**
 * emulate library functions that may fail
 * return \c EMU_SUCCESS on success.
 */
#define EMU_SUCCESS (0)

/**
 * emulate library functions that may fail return
 * \c EMU_FAILURE on error and
 * may set errno.
 */
#define EMU_FAILURE (-1)

#define EMU_UNHANDLED (2)

/**
 * File version number
 */
#define EMU_FILE_VERSION 1

struct emu_callbacks {
	int (*handleKeyFunc)(int sym, int mod, int scancode, uint16_t unicode, int event);
	int (*handleDPadFunc)(int angle, int event);
	int (*handleTouchFunc)(int dx, int dy);
	int (*handleMouseButtonFunc)(int button, int mask, int event); // TODO: Unify keyboard mod with mouse mask
	int (*handlePassThruButtonFunc)(int x, int y);
};

enum EmuKeyButtonState {
	EMU_KB_DOWN = 0,
	EMU_KB_UP = 1
};

enum EmuMouseButton {
	EMU_MOUSE_LEFT_BUTTON = 0,
	EMU_MOUSE_RIGHT_BUTTON = 1,
	EMU_MOUSE_MIDDLE_BUTTON = 2
};

enum EmuKeyMask {
	EMU_SHIFT = 0x1,
	EMU_CTRL = 0x2,
	EMU_ALT = 0x4
};

enum EmuMouseButtonState {
	EMU_MOUSE_BUTTON_DOWN = 0,
	EMU_MOUSE_BUTTON_UP = 1
};

typedef void * emu_context_t;
/**
 * Initialize the emulation layer and context.
 */
int emulate_initialize(emu_context_t *context, screen_context_t screenContext, struct emu_callbacks callbacks);

/**
 * Load the controls from a file.
 */
int emulate_loadcontrols(emu_context_t context, const char* filename);

/**
 * Show configuration window
 * NOTE: the window MUST have a window group set already.
 */
int emulate_swipedown(emu_context_t context, screen_window_t window);

/**
 * Provide touch events
 */
int emulate_touch(emu_context_t context, screen_event_t event);

/**
 * Cleanup and shutdown
 */
void emulate_shutdown(emu_context_t context);

__END_DECLS

#endif /* _EMULATE_EMULATE_H_INCLUDED */

/*
 * control.h
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <screen/screen.h>

class EventDispatcher;

class Control
{
public:
	enum ControlType {
		KEY,              // Used to provide keyboard input
		DPAD,             // Provides angle and magnitude from center (0 east, 90 north, 180 west, 270 south)
		TOUCHAREA,        // Used to provide relative mouse motion
		MOUSEBUTTON,      // Used to provide mouse button state
		PASSTHRUBUTTON    // Used for left-clicking visible buttons on screen
	};

	Control(screen_context_t context, ControlType type, int x, int y, unsigned width, unsigned height, EventDispatcher *dispatcher);
	~Control();

	void draw(screen_buffer_t buffer) const;
	bool handleTouch(int type, int contactId, int pos[]);

private:
	bool inBounds(int pos[]) const;

	ControlType m_type;
	int m_x;
	int m_y;
	unsigned m_width;
	unsigned m_height;
	EventDispatcher *m_dispatcher;

	screen_context_t m_context;
	screen_pixmap_t m_pixmap;
	screen_buffer_t m_buffer;

	int m_contactId;
	int m_lastPos[2];
};
#endif /* CONTROL_H_ */

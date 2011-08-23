/*
 * control.h
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <screen/screen.h>
#include <vector>

class EventDispatcher;
class Label;

class Control
{
public:
	enum ControlType {
		KEY,              // Used to provide keyboard input
		DPAD,             // Provides angle and magnitude from center (0 east, 90 north, 180 west, 270 south)
		TOUCHAREA,        // Used to provide relative mouse motion
		MOUSEBUTTON,      // Used to provide mouse button state
	};

	Control(screen_context_t context, ControlType type,
			int x, int y, unsigned width, unsigned height,
			EventDispatcher *dispatcher, EventDispatcher *tapDispatcher=0);
	~Control();
	void fill();
	bool loadFromPNG(const char *filename);

	void draw(screen_buffer_t buffer) const;
	bool handleTouch(int type, int contactId, const int pos[], long long timestamp);
	bool handleTap(int contactId, const int pos[]);
	bool handlesTap() const { return m_tapDispatcher != 0; }
	bool inBounds(const int pos[]) const;
	void move(int dx, int dy, unsigned maxDimensions[]);

	void showLabel(screen_window_t window);
	void addLabel(Label *label);

private:

	ControlType m_type;
	int m_x;
	int m_y;
	unsigned m_width;
	unsigned m_height;
	unsigned m_srcWidth;
	unsigned m_srcHeight;
	EventDispatcher *m_dispatcher;
	EventDispatcher *m_tapDispatcher;

	screen_context_t m_context;
	screen_pixmap_t m_pixmap;
	screen_buffer_t m_buffer;

	int m_contactId;
	int m_lastPos[2];
	long long m_touchDownTime;

	std::vector<Label *> m_labels;
};
#endif /* CONTROL_H_ */

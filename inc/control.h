/*
 * Copyright (c) 2011 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
		TOUCHSCREEN		  // Provides: mouse move, left click tap and right click tap-hold
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

	// For touch areas
	int m_lastPos[2];
	long long m_touchDownTime;

	// For touch screens
	int m_startPos[2];
	long long m_touchScreenStartTime;
	bool m_touchScreenInMoveEvent;
	bool m_touchScreenInHoldEvent;

	std::vector<Label *> m_labels;
};
#endif /* CONTROL_H_ */

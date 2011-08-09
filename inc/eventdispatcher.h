/*
 * eventdispatcher.h
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#ifndef EVENTDISPATCHER_H_
#define EVENTDISPATCHER_H_

#include "emulate.h"

class EventDispatcher
{
public:
	EventDispatcher() {}
	virtual ~EventDispatcher() {}

	virtual int runCallback(void *params) = 0;
};

class KeyEventDispatcher : public EventDispatcher
{
public:
	enum KeyEventType {
		KEY_DOWN = EMU_KB_DOWN,
		KEY_UP = EMU_KB_UP
	};

	struct KeyEvent {
		KeyEventType event;
	};

	KeyEventDispatcher(int (*callback)(int sym, int mod, int scancode, uint16_t unicode, int event),
			int sym, int mod, int scancode, uint16_t unicode)
		: m_callback(callback)
		, m_sym(sym)
		, m_mod(mod)
		, m_scancode(scancode)
		, m_unicode(unicode)
	{}

	int runCallback(void *params);

private:
	int (*m_callback)(int sym, int mod, int scancode, uint16_t unicode, int event);
	int m_sym;
	int m_mod;
	int m_scancode;
	uint16_t m_unicode;
};

class DPadEventDispatcher : public EventDispatcher
{
public:
	enum KeyEventType {
		DPAD_DOWN = EMU_KB_DOWN,
		DPAD_UP = EMU_KB_UP
	};
	struct DPadEvent {
		int angle;
		int event;
	};
	DPadEventDispatcher(int (*callback)(int angle, int event))
		: m_callback(callback)
	{}

	int runCallback(void *params);

private:
	int (*m_callback)(int angle, int event);
};

class TouchAreaEventDispatcher : public EventDispatcher
{
public:
	struct TouchAreaEvent {
		int dx;
		int dy;
	};
	TouchAreaEventDispatcher(int (*callback)(int dx, int dy))
		: m_callback(callback)
	{}

	int runCallback(void *params);

private:
	int (*m_callback)(int dx, int dy);
};

class MouseButtonEventDispatcher : public EventDispatcher
{
public:
	enum KeyEventType {
		MOUSE_DOWN = EMU_MOUSE_BUTTON_DOWN,
		MOUSE_UP = EMU_MOUSE_BUTTON_UP
	};

	struct MouseButtonEvent {
		int event;
	};

	MouseButtonEventDispatcher(int (*callback)(int button, int mask, int event), int mask, int button)
		: m_callback(callback)
		, m_mask(mask)
		, m_button(button)
	{}

	int runCallback(void *params);

private:
	int (*m_callback)(int button, int mask, int event);
	int m_mask;
	int m_button;
};

class PassThruEventDispatcher : public EventDispatcher
{
public:
	struct PassThruEvent {
		int x;
		int y;
	};
	PassThruEventDispatcher(int (*callback)(int x, int y))
		: m_callback(callback)
	{}

	int runCallback(void *params);

private:
	int (*m_callback)(int x, int y);
};

#endif /* EVENTDISPATCHER_H_ */

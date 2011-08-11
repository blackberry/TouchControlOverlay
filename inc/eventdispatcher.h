/*
 * eventdispatcher.h
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#ifndef EVENTDISPATCHER_H_
#define EVENTDISPATCHER_H_

#include "emulate.h"
#include "emulate_priv.h"

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
	typedef EmulationContext::HandleKeyFunc Callback;

	enum KeyEventType {
		KEY_DOWN = EMU_KB_DOWN,
		KEY_UP = EMU_KB_UP
	};

	struct KeyEvent {
		KeyEventType event;
	};

	KeyEventDispatcher(Callback callback,
			int sym, int mod, int scancode, uint16_t unicode)
		: m_callback(callback)
		, m_sym(sym)
		, m_mod(mod)
		, m_scancode(scancode)
		, m_unicode(unicode)
	{}

	int runCallback(void *params);

private:
	Callback m_callback;
	int m_sym;
	int m_mod;
	int m_scancode;
	uint16_t m_unicode;
};

class DPadEventDispatcher : public EventDispatcher
{
public:
	typedef EmulationContext::HandleDPadFunc Callback;

	enum KeyEventType {
		DPAD_DOWN = EMU_KB_DOWN,
		DPAD_UP = EMU_KB_UP
	};
	struct DPadEvent {
		int angle;
		int event;
	};
	DPadEventDispatcher(Callback callback)
		: m_callback(callback)
	{}

	int runCallback(void *params);

private:
	Callback m_callback;
};

class TouchAreaEventDispatcher : public EventDispatcher
{
public:
	typedef EmulationContext::HandleTouchFunc Callback;

	struct TouchAreaEvent {
		int dx;
		int dy;
	};
	TouchAreaEventDispatcher(Callback callback)
		: m_callback(callback)
	{}

	int runCallback(void *params);

private:
	Callback m_callback;
};

class MouseButtonEventDispatcher : public EventDispatcher
{
public:
	typedef EmulationContext::HandleMouseButtonFunc Callback;

	enum KeyEventType {
		MOUSE_DOWN = EMU_MOUSE_BUTTON_DOWN,
		MOUSE_UP = EMU_MOUSE_BUTTON_UP
	};

	struct MouseButtonEvent {
		int event;
	};

	MouseButtonEventDispatcher(Callback callback, int mask, int button)
		: m_callback(callback)
		, m_mask(mask)
		, m_button(button)
	{}

	int runCallback(void *params);

private:
	Callback m_callback;
	int m_mask;
	int m_button;
};

class PassThruEventDispatcher : public EventDispatcher
{
public:
	typedef EmulationContext::HandlePassThruButtonFunc Callback;

	struct PassThruEvent {
		int x;
		int y;
	};
	PassThruEventDispatcher(Callback callback)
		: m_callback(callback)
	{}

	int runCallback(void *params);

private:
	Callback m_callback;
};

#endif /* EVENTDISPATCHER_H_ */

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

#ifndef EVENTDISPATCHER_H_
#define EVENTDISPATCHER_H_

#include "touchcontroloverlay.h"
#include "touchcontroloverlay_priv.h"

class EventDispatcher
{
public:
	EventDispatcher() {}
	virtual ~EventDispatcher() {}

	virtual int runCallback(void *params) = 0;
};

class TapDispatcher : public EventDispatcher
{
public:
	typedef TCOContext::HandleTapFunc Callback;

	TapDispatcher(Callback callback)
		: m_callback(callback)
	{}
	int runCallback(void *params);

private:
	Callback m_callback;
};

class KeyEventDispatcher : public EventDispatcher
{
public:
	typedef TCOContext::HandleKeyFunc Callback;

	enum KeyEventType {
		KEY_DOWN = TCO_KB_DOWN,
		KEY_UP = TCO_KB_UP
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
	typedef TCOContext::HandleDPadFunc Callback;

	enum KeyEventType {
		DPAD_DOWN = TCO_KB_DOWN,
		DPAD_UP = TCO_KB_UP
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
	typedef TCOContext::HandleTouchFunc Callback;

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
	typedef TCOContext::HandleMouseButtonFunc Callback;

	enum KeyEventType {
		MOUSE_DOWN = TCO_MOUSE_BUTTON_DOWN,
		MOUSE_UP = TCO_MOUSE_BUTTON_UP
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

class TouchScreenEventDispatcher : public EventDispatcher
{
public:
	typedef TCOContext::HandleTouchScreenFunc Callback;

	TouchScreenEventDispatcher(Callback callback)
		: m_callback(callback)
	{}

	struct TouchScreenEvent {
		int x;
		int y;
		int tap;
		int hold;
	};

	int runCallback(void *params);

private:
	Callback m_callback;
};

#endif /* EVENTDISPATCHER_H_ */

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

#include "eventdispatcher.h"

int TapDispatcher::runCallback(void *params)
{
	return (*m_callback)();
}

int KeyEventDispatcher::runCallback(void *params)
{
	KeyEvent *event = static_cast<KeyEvent *>(params);
	return (*m_callback)(m_sym, m_mod, m_scancode, m_unicode, event->event);
}

int DPadEventDispatcher::runCallback(void * params)
{
	DPadEvent *event = static_cast<DPadEvent *>(params);
	return (*m_callback)(event->angle, event->event);
}

int TouchAreaEventDispatcher::runCallback(void *params)
{
	TouchAreaEvent *event = static_cast<TouchAreaEvent *>(params);
	return (*m_callback)(event->dx, event->dy);
}

int MouseButtonEventDispatcher::runCallback(void *params)
{
	MouseButtonEvent *event = static_cast<MouseButtonEvent *>(params);
	return (*m_callback)(m_button, m_mask, event->event);
}

int TouchScreenEventDispatcher::runCallback(void *params)
{
	TouchScreenEvent *event = static_cast<TouchScreenEvent *>(params);
	return (*m_callback)(event->x, event->y, event->tap, event->hold);
}

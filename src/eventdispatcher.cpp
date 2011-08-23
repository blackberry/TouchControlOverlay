/*
 * eventdispatcher.cpp
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
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

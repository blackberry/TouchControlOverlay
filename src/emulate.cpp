/*
 * emulate.cpp
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#include "emulate.h"
#include "emulate_priv.h"
#include "eventdispatcher.h"
#include <bps/bps.h>
#include <bps/screen.h>
#include <bps/event.h>
#include <bps/navigator.h>
#include "control.h"
#include "configwindow.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define EXTERNAL_API extern "C"

EXTERNAL_API int emulate_initialize(emu_context_t *context, screen_context_t screenContext, emu_callbacks callbacks)
{
	EmulationContext *emuContext = new EmulationContext(screenContext, callbacks);
	*context = (void*)emuContext;
	return EMU_SUCCESS;
}

EXTERNAL_API int emulate_loadcontrols(emu_context_t context, const char* filename)
{
	EmulationContext *ctx = static_cast<EmulationContext *>(context);
	return ctx->loadControls(filename);
}

EXTERNAL_API int emulate_swipedown(emu_context_t context, screen_window_t window)
{
	EmulationContext *ctx = static_cast<EmulationContext *>(context);
	return ctx->showConfig(window);
}

EXTERNAL_API int emulate_touch(emu_context_t context, screen_event_t event)
{
	EmulationContext *ctx = static_cast<EmulationContext *>(context);
	bool handled = ctx->touchEvent(event);
	if (handled)
		return EMU_SUCCESS;
	else
		return EMU_UNHANDLED;
}

EXTERNAL_API void emulate_shutdown(emu_context_t context)
{
	EmulationContext *ctx = static_cast<EmulationContext *>(context);
	delete ctx;
	ctx = 0;
}

EmulationContext::EmulationContext(screen_context_t screenContext, emu_callbacks callbacks)
{
	m_screenContext = screenContext;
	m_appWindow = 0;
	m_configWindow = 0;

	m_handleKeyFunc = callbacks.handleKeyFunc;
	m_handleDPadFunc = callbacks.handleDPadFunc;
	m_handleTouchFunc = callbacks.handleTouchFunc;
	m_handleMouseButtonFunc = callbacks.handleMouseButtonFunc;
	m_handlePassThruButtonFunc = callbacks.handlePassThruButtonFunc;
}

int EmulationContext::showConfig(screen_window_t window)
{
	m_appWindow = window;

	if (!m_configWindow) {
		m_configWindow = ConfigWindow::createConfigWindow(m_screenContext, window);
		if (!m_configWindow) {
			return EMU_FAILURE;
		}
		m_configWindow->runEventLoop(this);
		delete m_configWindow;
		m_configWindow = 0;
	}
	return EMU_SUCCESS;
}

int EmulationContext::loadControls(const char *filename)
{
#define FILE_VERSION 1
	std::ifstream inFile(filename);
	std::string line;
	std::stringstream ss;
	Control *control = 0;
	int type, x, y, w, h, extra[3];
	uint16_t unicode;
	int version = 0;

	while (inFile)
	{
		if (getline(inFile, line)) {
			ss.clear();
			ss.str("");
			ss << line;
			if (version == 0) {
				ss >> version;
				if (version == 0 || version > FILE_VERSION) {
					fprintf(stderr, "Version mismatch: read %d\n", version);
					return EMU_FAILURE;
				} else {
					continue;
				}
			}
			ss >> type >> x >> y >> w >> h;
			switch (type) {
			case Control::KEY:
				ss >> extra[0] >> extra[1] >> extra[2] >> unicode;
				control = new Control(m_screenContext, static_cast<Control::ControlType>(type),
						x, y, w, h,
						new KeyEventDispatcher(m_handleKeyFunc, extra[0], extra[1], extra[2], unicode));
				break;
			case Control::DPAD:
				control = new Control(m_screenContext, static_cast<Control::ControlType>(type),
						x, y, w, h, new DPadEventDispatcher(m_handleDPadFunc));
				break;
			case Control::TOUCHAREA:
				control = new Control(m_screenContext, static_cast<Control::ControlType>(type),
						x, y, w, h, new TouchAreaEventDispatcher(m_handleTouchFunc));
				break;
			case Control::MOUSEBUTTON:
				ss >> extra[0] >> extra[1];
				control = new Control(m_screenContext, static_cast<Control::ControlType>(type),
						x, y, w, h, new MouseButtonEventDispatcher(m_handleMouseButtonFunc, extra[0], extra[1]));
				break;
			case Control::PASSTHRUBUTTON:
				control = new Control(m_screenContext, static_cast<Control::ControlType>(type),
						x, y, w, h, new PassThruEventDispatcher(m_handlePassThruButtonFunc));
				break;
			}
			m_controls.push_back(control);
		}
	}

	return EMU_SUCCESS;
}

bool EmulationContext::touchEvent(screen_event_t event)
{
	int type;
    int contactId;
    int pos[2];
    int screenPos[2];
    int orientation;
    long long timestamp;
    int sequenceId;
    bool handled = false;

    screen_get_event_property_iv(event, SCREEN_PROPERTY_TYPE, &type);
    screen_get_event_property_iv(event, SCREEN_PROPERTY_TOUCH_ID, (int*)&contactId);
    screen_get_event_property_iv(event, SCREEN_PROPERTY_SOURCE_POSITION, pos);
    screen_get_event_property_iv(event, SCREEN_PROPERTY_POSITION, screenPos);
    screen_get_event_property_iv(event, SCREEN_PROPERTY_TOUCH_ORIENTATION, (int*)&orientation);
    screen_get_event_property_llv(event, SCREEN_PROPERTY_TIMESTAMP, (long long*)&timestamp);
    screen_get_event_property_iv(event, SCREEN_PROPERTY_SEQUENCE_ID, (int*)&sequenceId);

    Control * touchPointOwner = m_controlMap[contactId];
    if (touchPointOwner) {
    	handled = touchPointOwner->handleTouch(type, contactId, pos);
    	if (!handled)
    		m_controlMap[contactId] = 0;
    }

    if (!handled) {
		std::vector<Control *>::iterator iter = m_controls.begin();
		for (; iter != m_controls.end(); iter++)
		{
			if (*iter == touchPointOwner)
				continue; // already checked

			handled |= (*iter)->handleTouch(type, contactId, pos);
			if (handled) {
				m_controlMap[contactId] = (*iter);
				break; // Only allow the first control to handle the touch.
			}
		}
    }
	return handled;
}

void EmulationContext::drawControls(screen_buffer_t buffer)
{
	std::vector<Control *>::iterator iter = m_controls.begin();
	while (iter != m_controls.end())
	{
		(*iter)->draw(buffer);
		iter++;
	}
}

EmulationContext::~EmulationContext()
{
	m_appWindow = 0;
	delete m_configWindow;
	m_configWindow = 0;

	std::vector<Control *>::iterator iter = m_controls.begin();
	while (iter != m_controls.end())
	{
		delete *iter;
		iter++;
	}
	m_controls.clear();
	m_controlMap.clear();

	m_handleKeyFunc = 0;
	m_handleDPadFunc = 0;
	m_handleTouchFunc = 0;
	m_handleMouseButtonFunc = 0;
	m_handlePassThruButtonFunc = 0;
}

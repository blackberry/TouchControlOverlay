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

#include "emulate.h"
#include "emulate_priv.h"
#include "eventdispatcher.h"
#include <bps/bps.h>
#include <bps/screen.h>
#include <bps/event.h>
#include <bps/navigator.h>
#include "control.h"
#include "controlfactory.h"
#include "configwindow.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

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

EXTERNAL_API int emulate_showlabels(emu_context_t context, screen_window_t window)
{
	EmulationContext *ctx = static_cast<EmulationContext *>(context);
	return ctx->showLabels(window);
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
	m_handleTapFunc = callbacks.handleTapFunc;
	m_handleTouchScreenFunc = callbacks.handleTouchScreenFunc;
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
	struct TrackedXML
	{
		TrackedXML()
			: m_doc(0)
		{}

		~TrackedXML()
		{
			if (m_doc)
				xmlFreeDoc(m_doc);
			m_doc = 0;
			xmlCleanupParser();
		}
		xmlDoc *m_doc;
	} xml;
	std::stringstream ss;

	xml.m_doc = xmlReadFile(filename, NULL, 0);
	if (xml.m_doc == NULL) {
		fprintf(stderr, "Unable to parse control file %s\n", filename);
		return EMU_FAILURE;
	}

	xmlNode *root = xmlDocGetRootElement(xml.m_doc);

	// Check version
	bool versionMatch = false;
	if (root && root->properties) {
		xmlAttr *properties = root->properties;
		while (properties) {
			if (!xmlStrncasecmp(properties->name, BAD_CAST "version", strlen("version"))) {
				if (properties->children && properties->children->content) {
					int version = 0;
					ss.clear();
					ss.str("");
					ss << properties->children->content;
					ss >> version;
					if (version != 0 && version <= EMU_FILE_VERSION) {
						versionMatch = true;
					}
				}
				break;
			}
			properties = properties->next;
		}
	}

	if (!versionMatch) {
		fprintf(stderr, "Version mismatch\n");
		return EMU_FAILURE;
	}

	xmlNode *cur = root->children;
	Control *control = 0;
	while (cur) {
		control = ControlFactory::createControl(this, cur);
		if (control) {
			m_controls.push_back(control);
		}
		cur = cur->next;
	}

	return EMU_SUCCESS;
}

Control *EmulationContext::controlAt(int pos[]) const
{
	std::vector<Control *>::const_iterator iter = m_controls.begin();
	for (; iter != m_controls.end(); iter++)
	{
		if ((*iter)->inBounds(pos)) {
			return *iter;
		}
	}
	return NULL;
}

int EmulationContext::showLabels(screen_window_t window)
{
	std::vector<Control *>::const_iterator iter = m_controls.begin();
	for (; iter != m_controls.end(); iter++)
	{
		(*iter)->showLabel(window);
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

#if 0
    if (type == SCREEN_EVENT_MTOUCH_TOUCH)
    	m_touchMap[contactId].downThisFrame = true;
    if (type == SCREEN_EVENT_MTOUCH_RELEASE && m_touchMap[contactId].downThisFrame)
    	m_touchMap[contactId].tap = true;
    m_touchMap[contactId].valid = true;
    m_touchMap[contactId].pos[0] = pos[0];
    m_touchMap[contactId].pos[1] = pos[1];
    m_touchMap[contactId].type = type;
    m_touchMap[contactId].timestamp = timestamp;

    return true;
#else
    Control * touchPointOwner = m_controlMap[contactId];
    if (touchPointOwner) {
    	handled = touchPointOwner->handleTouch(type, contactId, pos, timestamp);
    	if (!handled)
    		m_controlMap[contactId] = 0;
    }

    if (!handled) {
		std::vector<Control *>::iterator iter = m_controls.begin();
		for (; iter != m_controls.end(); iter++)
		{
			if (*iter == touchPointOwner)
				continue; // already checked

			handled |= (*iter)->handleTouch(type, contactId, pos, timestamp);
			if (handled) {
				m_controlMap[contactId] = (*iter);
				break; // Only allow the first control to handle the touch.
			}
		}
    }

	return handled;
#endif
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
	m_handleTapFunc = 0;
}

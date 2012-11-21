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

#include "configwindow.h"
#include "touchcontroloverlay_priv.h"
#include "control.h"
#include <bps/bps.h>
#include <bps/screen.h>
#include <bps/event.h>
#include <bps/navigator.h>

ConfigWindow* ConfigWindow::createConfigWindow(screen_context_t context, screen_window_t parent)
{
	const int zOrder = 10; // FIXME: hardcoded
	ConfigWindow *window = new ConfigWindow(context, parent);
	if (!window->m_valid) {
		delete window;
		return 0;
	}

	if (!window->setZOrder(zOrder) ||
			!window->setTouchSensitivity(true)) {
		delete window;
		return 0;
	}

	return window;
}

screen_buffer_t ConfigWindow::draw(TCOContext *emuContext)
{
	screen_buffer_t buffer;
	unsigned char *pixels;
	int stride;
	if (!getPixels(&buffer, &pixels, &stride)) {
		return 0;
	}

	{
		// Fill pixels
		int i=0,j=0;
		for (i=0; i<m_size[1]; i++) {
			for (j=0; j<m_size[0]; j++) {
				pixels[i*stride+j*4] = 0xa0;
				pixels[i*stride+j*4+1] = 0xa0;
				pixels[i*stride+j*4+2] = 0xa0;
				pixels[i*stride+j*4+3] = 0xa0;
			}
		}
	}
	emuContext->drawControls(buffer);
	return buffer;
}

void ConfigWindow::runEventLoop(TCOContext *emuContext)
{
	screen_buffer_t buffer = draw(emuContext);

	bool showingWindow = true;
	bps_initialize();
	screen_request_events(m_context);
	bps_event_t *event; // FIXME: How do we verify they ran bps_initialize?
	screen_event_t se;
	int eventType;
	int contactId;
	bool touching = false;
	bool releasedThisRound = false;
	int startPos[2] = {0,0};
	int endPos[2] = {0,0};
	bool scaling = false;

	while (showingWindow)
	{
		const int dirtyRects[4] = {0, 0, m_size[0], m_size[1]};

		releasedThisRound = false;
		bps_get_event(&event, 0);
		while (showingWindow && event)
		{
			int domain = bps_event_get_domain(event);
			if (domain == navigator_get_domain())
			{
				if (bps_event_get_code(event) == NAVIGATOR_SWIPE_DOWN)
					showingWindow = false;
				else if (bps_event_get_code(event) == NAVIGATOR_EXIT) {
					showingWindow = false;
				}
			} else if (domain == screen_get_domain()) {
				se = screen_event_get_event(event);
				screen_get_event_property_iv(se, SCREEN_PROPERTY_TYPE, &eventType);
				screen_get_event_property_iv(se, SCREEN_PROPERTY_TOUCH_ID, &contactId);
				switch (eventType)
				{
				case SCREEN_EVENT_MTOUCH_TOUCH:
					screen_get_event_property_iv(se, SCREEN_PROPERTY_TOUCH_ID, &contactId);
					if (contactId == 0 && !touching && !releasedThisRound) {
						touching = true;
						screen_get_event_property_iv(se, SCREEN_PROPERTY_SOURCE_POSITION, startPos);
						endPos[0] = startPos[0];
						endPos[1] = startPos[1];
					}
					break;
				case SCREEN_EVENT_MTOUCH_MOVE:
					screen_get_event_property_iv(se, SCREEN_PROPERTY_TOUCH_ID, &contactId);
					if (contactId == 0 && touching) {
						screen_get_event_property_iv(se, SCREEN_PROPERTY_SOURCE_POSITION, endPos);
					}
					break;
				case SCREEN_EVENT_MTOUCH_RELEASE:
					screen_get_event_property_iv(se, SCREEN_PROPERTY_TOUCH_ID, &contactId);
					if (contactId == 0 && touching) {
						touching = false;
						releasedThisRound = true;
						screen_get_event_property_iv(se, SCREEN_PROPERTY_SOURCE_POSITION, endPos);
					}
					break;
				default:
					fprintf(stderr, "Unknown screen event: %d\n", eventType);
					break;
				}
			}

			bps_get_event(&event, 0);
		}

		if (releasedThisRound) {
			m_selected = 0;
		} else if (touching) {
			if (!m_selected)
				m_selected = emuContext->controlAt(startPos);
			if (m_selected && (endPos[0] != startPos[0] || endPos[1] != startPos[1])) {
				m_selected->move(endPos[0] - startPos[0], endPos[1] - startPos[1], (unsigned*)m_size);
				buffer = draw(emuContext);
				startPos[0] = endPos[0];
				startPos[1] = endPos[1];
			}
		}

		if (buffer)
			post(buffer);
	}
}

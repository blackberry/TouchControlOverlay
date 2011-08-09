/*
 * configwindow.cpp
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#include "configwindow.h"
#include "emulate_priv.h"
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

void ConfigWindow::runEventLoop(EmulationContext *emuContext)
{
	screen_buffer_t buffer;
	unsigned char *pixels;
	int stride;
	if (!getPixels(&buffer, &pixels, &stride)) {
		return;
	}

	{
		// Fill pixels
		int i=0,j=0;
		for (i=0; i<m_size[1]; i++) {
			for (j=0; j<m_size[0]; j++) {
				pixels[i*stride+j*4] = 0;
				pixels[i*stride+j*4+1] = (i+j) * 0xff / (m_size[0]+m_size[1]);
				pixels[i*stride+j*4+2] = (i * 0xff / (m_size[1]));
				pixels[i*stride+j*4+3] = 0xff;
			}
		}
	}
	emuContext->drawControls(buffer);

	bool showingWindow = true;
	bps_initialize(NULL, NULL);
	screen_request_events(m_context);
	bps_event_t *event; // FIXME: How do we verify they ran bps_initialize?
	screen_event_t se;
	int eventType;
	while (showingWindow)
	{
		const int dirtyRects[4] = {0, 0, m_size[0], m_size[1]};

		bps_get_event(&event, 0);
		while (event)
		{
			switch (bps_event_get_domain(event))
			{
			case BPS_EVENT_DOMAIN_NAVIGATOR:
				if (bps_event_get_code(event) == NAV_SWIPE_DOWN)
					showingWindow = false;
				break;
			case BPS_EVENT_DOMAIN_SCREEN:
				se = screen_event_get_event(event);
				screen_get_event_property_iv(se, SCREEN_PROPERTY_TYPE, &eventType);
				switch (eventType)
				{
				case SCREEN_EVENT_MTOUCH_TOUCH:
				case SCREEN_EVENT_MTOUCH_MOVE:
				case SCREEN_EVENT_MTOUCH_RELEASE:
					showingWindow = false;
					break;
				default:
					fprintf(stderr, "Unknown screen event: %d\n", eventType);
					break;
				}
				break;
			default:
				fprintf(stderr, "Unknown event domain: %d\n", bps_event_get_domain(event));
				break;
			}
			bps_event_destroy(event);
			bps_get_event(&event, 0);
		}

		post(buffer);
	}
}

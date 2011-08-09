/*
 * control.cpp
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#include "control.h"
#include "eventdispatcher.h"
#include <math.h>

Control::Control(screen_context_t context, ControlType type, int x, int y, unsigned width, unsigned height, EventDispatcher *dispatcher)
	: m_type(type)
	, m_x(x)
	, m_y(y)
	, m_width(width)
	, m_height(height)
	, m_dispatcher(dispatcher)
	, m_context(context)
	, m_contactId(-1)
{
	static unsigned controlNum = 0;
	static uint32_t controlColors[] = { 0xaaff0000, 0xaa00ff00, 0xaa0000ff, 0xaaffff00, 0xaaff00ff, 0xaa00ffff };

	m_lastPos[0] = 0;
	m_lastPos[1] = 0;

	int format = SCREEN_FORMAT_RGBA8888;
	int size[2] = {width, height};
	unsigned char *pixels;
	int stride;

	int rc = screen_create_pixmap(&m_pixmap, context); // FIXME: Check failure
	rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_FORMAT, &format);
	rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
	rc = screen_create_pixmap_buffer(m_pixmap);
	rc = screen_get_pixmap_property_pv(m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&m_buffer);
	rc = screen_get_buffer_property_pv(m_buffer, SCREEN_PROPERTY_POINTER, (void **)&pixels);
	rc = screen_get_buffer_property_iv(m_buffer, SCREEN_PROPERTY_STRIDE, &stride);

	{
		// BGRA format
		for (unsigned i=0; i<height; i++) {
			for (unsigned j=0; j<width; j++) {
				uint32_t *ptr = (uint32_t*)&(pixels[i*stride+j*4]);
				*ptr = controlColors[controlNum];
			}
		}
	}
	controlNum++;
	if (controlNum > 5)
		controlNum = 0;
}

Control::~Control()
{
	screen_destroy_pixmap(m_pixmap);
	delete m_dispatcher;
}

void Control::draw(screen_buffer_t buffer) const
{
	int attribs[] = {
			SCREEN_BLIT_SOURCE_X, 0,
			SCREEN_BLIT_SOURCE_Y, 0,
			SCREEN_BLIT_SOURCE_WIDTH, m_width,
			SCREEN_BLIT_SOURCE_HEIGHT, m_height,
			SCREEN_BLIT_DESTINATION_X, m_x,
			SCREEN_BLIT_DESTINATION_Y, m_y,
			SCREEN_BLIT_DESTINATION_WIDTH, m_width,
			SCREEN_BLIT_DESTINATION_HEIGHT, m_height,
			SCREEN_BLIT_GLOBAL_ALPHA, 128,
			SCREEN_BLIT_END
	};
	screen_blit(m_context, buffer, m_buffer, attribs);
}

bool Control::inBounds(int pos[]) const
{
	return (pos[0] >= m_x && pos[0] <= static_cast<int>(m_x + m_width) &&
			pos[1] >= m_y && pos[1] <= static_cast<int>(m_y + m_height));
}

bool Control::handleTouch(int type, int contactId, int pos[])
{
	if (m_contactId != -1 && m_contactId != contactId) {
		// We have a contact point set and this isn't it.
		return false;
	}

	if (m_contactId == -1) {
		// Don't handle orphaned release events.
		if (type == SCREEN_EVENT_MTOUCH_RELEASE)
			return false;
		if (!inBounds(pos))
			return false;

		// This is a new touch point that we should start handling
		m_contactId = contactId;

		switch (m_type)
		{
		case KEY:
			{
				KeyEventDispatcher::KeyEvent event;
				event.event = KeyEventDispatcher::KEY_DOWN;
				m_dispatcher->runCallback(&event);
			}
			break;
		case DPAD:
			{
				DPadEventDispatcher::DPadEvent event;
				event.angle = atan2((pos[1] - m_y - m_height / 2.0f), (pos[0] - m_x - m_width / 2.0f)) * 180 / M_PI;
				event.event = DPadEventDispatcher::DPAD_DOWN;
			}
			break;
		case TOUCHAREA:
			m_lastPos[0] = pos[0];
			m_lastPos[1] = pos[1];
			break;
		case MOUSEBUTTON:
			{
				MouseButtonEventDispatcher::MouseButtonEvent event;
				event.event = MouseButtonEventDispatcher::MOUSE_DOWN;
				m_dispatcher->runCallback(&event);
			}
			break;
		case PASSTHRUBUTTON:
			break;
		default:
			break;
		}
	} else {
		if (!inBounds(pos)) {
			// Act as if we received a key up
			switch (m_type)
			{
			case KEY:
				{
					KeyEventDispatcher::KeyEvent event;
					event.event = KeyEventDispatcher::KEY_UP;
					m_dispatcher->runCallback(&event);
				}
				break;
			case DPAD:
				{
					DPadEventDispatcher::DPadEvent event;
					event.angle = atan2((pos[1] - m_y - m_height / 2.0f), (pos[0] - m_x - m_width / 2.0f)) * 180 / M_PI;
					event.event = DPadEventDispatcher::DPAD_UP;
				}
				break;
			case TOUCHAREA:
				{
					TouchAreaEventDispatcher::TouchAreaEvent event;
					event.dx = pos[0] - m_lastPos[0];
					event.dy = pos[1] - m_lastPos[1];
					if (event.dx != 0 || event.dy != 0) {
						m_dispatcher->runCallback(&event);
						m_lastPos[0] = pos[0];
						m_lastPos[1] = pos[1];
					}
				}
				break;
			case MOUSEBUTTON:
				{
					MouseButtonEventDispatcher::MouseButtonEvent event;
					event.event = MouseButtonEventDispatcher::MOUSE_UP;
					m_dispatcher->runCallback(&event);
				}
				break;
			case PASSTHRUBUTTON:
				// TODO: Consider if we should send the button press?
				break;
			default:
				break;
			}
			m_contactId = -1;
			return false;
		}

		// We have had a previous touch point from this contact and this point is in bounds
		switch (m_type)
		{
		case KEY:
			if (type == SCREEN_EVENT_MTOUCH_RELEASE)
			{
				KeyEventDispatcher::KeyEvent event;
				event.event = KeyEventDispatcher::KEY_UP;
				m_dispatcher->runCallback(&event);
			}
			break;
		case DPAD:
			{
				DPadEventDispatcher::DPadEvent event;
				event.angle = atan2((pos[1] - m_y - m_height / 2.0f), (pos[0] - m_x - m_width / 2.0f)) * 180 / M_PI;
				if (type == SCREEN_EVENT_MTOUCH_RELEASE)
					event.event = DPadEventDispatcher::DPAD_UP;
				else
					event.event = DPadEventDispatcher::DPAD_DOWN;
			}
			break;
		case TOUCHAREA:
			{
				TouchAreaEventDispatcher::TouchAreaEvent event;
				event.dx = pos[0] - m_lastPos[0];
				event.dy = pos[1] - m_lastPos[1];
				if (event.dx != 0 || event.dy != 0) {
					m_dispatcher->runCallback(&event);
					m_lastPos[0] = pos[0];
					m_lastPos[1] = pos[1];
				}
			}
			break;
		case MOUSEBUTTON:
			if (type == SCREEN_EVENT_MTOUCH_RELEASE)
			{
				MouseButtonEventDispatcher::MouseButtonEvent event;
				event.event = MouseButtonEventDispatcher::MOUSE_UP;
				m_dispatcher->runCallback(&event);
			}
			break;
		case PASSTHRUBUTTON:
			if (type == SCREEN_EVENT_MTOUCH_RELEASE)
			{
				PassThruEventDispatcher::PassThruEvent event;
				event.x = pos[0];
				event.y = pos[1];
				m_dispatcher->runCallback(&event);
			}
			break;
		default:
			break;
		}

		if (type == SCREEN_EVENT_MTOUCH_RELEASE) {
			m_contactId = -1;
			return false;
		}
	}

	return true;
}

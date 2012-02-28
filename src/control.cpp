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

#include "control.h"
#include "eventdispatcher.h"
#include "label.h"
#include "pngreader.h"
#include <math.h>
#include <stdlib.h>

#include <screen/screen.h>

const static int TAP_THRESHOLD = 150000000L;
const static int JITTER_THRESHOLD = 10;
const static int pngRed = 0;
const static int pngGreen = 1;
const static int pngBlue = 2;
const static int pngAlpha = 3;

//struct TrackedPNG
//{
//	TrackedPNG(FILE *file)
//		: m_read(0)
//		, m_info(0)
//		, m_data(0)
//		, m_rows(0)
//		, m_buffer(0)
//		, m_width(0)
//		, m_height(0)
//		, m_stride(0)
//		, m_file(file)
//	{}
//
//	~TrackedPNG()
//	{
//		delete [] m_rows;
//		delete [] m_data;
//
//		if (m_read)
//			png_destroy_read_struct(&m_read, m_info ? &m_info : (png_infopp) 0, (png_infopp) 0);
//		if (m_buffer)
//			screen_destroy_buffer(m_buffer);
//		if (m_file)
//			fclose(m_file);
//	}
//
//	png_structp m_read;
//	png_infop m_info;
//	unsigned char* m_data;
//	png_bytep* m_rows;
//	screen_buffer_t m_buffer;
//	int m_width;
//	int m_height;
//	int m_stride;
//	FILE *m_file;
//};

Control::Control(screen_context_t context, ControlType type,
		int x, int y, unsigned width, unsigned height,
		EventDispatcher *dispatcher, EventDispatcher *tapDispatcher)
	: m_type(type)
	, m_x(x)
	, m_y(y)
	, m_width(width)
	, m_height(height)
	, m_srcWidth(width)
	, m_srcHeight(height)
	, m_dispatcher(dispatcher)
	, m_tapDispatcher(tapDispatcher)
	, m_context(context)
	, m_contactId(-1)
	, m_touchDownTime(0)
	, m_touchScreenStartTime(0)
	, m_touchScreenInMoveEvent(false)
	, m_touchScreenInHoldEvent(false)
{
	m_lastPos[0] = 0;
	m_lastPos[1] = 0;
	m_startPos[0] = 0;
	m_startPos[1] = 0;
}

Control::~Control()
{
	std::vector<Label *>::iterator iter = m_labels.begin();
	while (iter != m_labels.end())
	{
		delete *iter;
		iter++;
	}
	m_labels.clear();
	screen_destroy_pixmap(m_pixmap);
	delete m_dispatcher;
	delete m_tapDispatcher;
}

void Control::fill()
{
	static unsigned controlNum = 0;
	static uint32_t controlColors[] = { 0xaaff0000, 0xaa00ff00, 0xaa0000ff, 0xaaffff00, 0xaaff00ff, 0xaa00ffff };

	int format = SCREEN_FORMAT_RGBA8888;
	int size[2] = {m_width, m_height};
	unsigned char *pixels;
	int stride;

	int rc = screen_create_pixmap(&m_pixmap, m_context); // FIXME: Check failure
	rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_FORMAT, &format);
	rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
	rc = screen_create_pixmap_buffer(m_pixmap);
	rc = screen_get_pixmap_property_pv(m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&m_buffer);
//	rc = screen_get_buffer_property_pv(m_buffer, SCREEN_PROPERTY_POINTER, (void **)&pixels);
//	rc = screen_get_buffer_property_iv(m_buffer, SCREEN_PROPERTY_STRIDE, &stride);
	int attribs[] = {
		SCREEN_BLIT_COLOR, (int)controlColors[controlNum],
		SCREEN_BLIT_END
	};
	rc = screen_fill(m_context, m_buffer, attribs);
	controlNum++;
	if (controlNum > 5)
		controlNum = 0;
}

bool Control::loadFromPNG(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Unable to open file %s\n", filename);
		return false;
	}

	PNGReader png(file, m_context);
	if (!png.doRead())
		return false;

	m_srcWidth = png.m_width;
	m_srcHeight = png.m_height;
	m_pixmap = png.m_pixmap;
	png.m_pixmap = 0;
	m_buffer = png.m_buffer;
	png.m_buffer = 0;
//	int rc;
//	{
//		int format = SCREEN_FORMAT_RGBA8888;
//		int size[2] = {png.m_width, png.m_height};

//
//		rc = screen_create_pixmap(&m_pixmap, m_context); // FIXME: Check failure
//		rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_FORMAT, &format);
//		rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
//		rc = screen_create_pixmap_buffer(m_pixmap);
//
//		unsigned char *realPixels;
//		int realStride;
//		rc = screen_get_pixmap_property_pv(m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&m_buffer);
//		rc = screen_get_buffer_property_pv(m_buffer, SCREEN_PROPERTY_POINTER, (void **)&realPixels);
//		rc = screen_get_buffer_property_iv(m_buffer, SCREEN_PROPERTY_STRIDE, &realStride);
//		memcpy(realPixels, png.m_data, realStride * m_srcHeight);
//	}

	return true;
}

void Control::showLabel(screen_window_t window)
{
	std::vector<Label *>::iterator iter = m_labels.begin();
	while (iter != m_labels.end())
	{
		(*iter)->draw(window, m_x, m_y);
		iter++;
	}
}

void Control::addLabel(Label *label)
{
	m_labels.push_back(label);
	label->setControl(this);
}

void Control::move(int dx, int dy, unsigned maxDimensions[])
{
	m_x += dx;
	m_y += dy;
	if (m_x <= 0)
		m_x = 0;
	if (m_y <= 0)
		m_y = 0;
	if (m_x + m_width >= maxDimensions[0])
		m_x = maxDimensions[0] - m_width;
	if (m_y + m_height >= maxDimensions[1])
		m_y = maxDimensions[1] - m_height;
	std::vector<Label *>::iterator iter = m_labels.begin();
	while (iter != m_labels.end())
	{
		(*iter)->move(m_x, m_y);
		iter++;
	}
}

void Control::draw(screen_buffer_t buffer) const
{
	screen_get_pixmap_property_pv(m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&m_buffer);
	int attribs[] = {
			SCREEN_BLIT_SOURCE_X, 0,
			SCREEN_BLIT_SOURCE_Y, 0,
			SCREEN_BLIT_SOURCE_WIDTH, m_srcWidth,
			SCREEN_BLIT_SOURCE_HEIGHT, m_srcHeight,
			SCREEN_BLIT_DESTINATION_X, m_x,
			SCREEN_BLIT_DESTINATION_Y, m_y,
			SCREEN_BLIT_DESTINATION_WIDTH, m_width,
			SCREEN_BLIT_DESTINATION_HEIGHT, m_height,
			SCREEN_BLIT_TRANSPARENCY, SCREEN_TRANSPARENCY_NONE,
			SCREEN_BLIT_GLOBAL_ALPHA, 192,
			SCREEN_BLIT_END
	};
	screen_blit(m_context, buffer, m_buffer, attribs);
}

bool Control::inBounds(const int pos[]) const
{
	return (pos[0] >= m_x && pos[0] <= static_cast<int>(m_x + m_width) &&
			pos[1] >= m_y && pos[1] <= static_cast<int>(m_y + m_height));
}

bool Control::handleTap(int contactId, const int pos[])
{
	if (!m_tapDispatcher)
		return false;

	if (m_contactId != -1) {
		// We have a contact point set already. No taps allowed.
		return false;
	}
	if (!inBounds(pos))
		return false;

	return m_tapDispatcher->runCallback(0);

}

bool Control::handleTouch(int type, int contactId, const int pos[], long long timestamp)
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
				m_dispatcher->runCallback(&event);
			}
			break;
		case TOUCHAREA:
			//fprintf(stderr, "Toucharea: new touch %d,%d\n", pos[0],pos[1]);
			m_touchDownTime = timestamp;
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
		case TOUCHSCREEN:
			m_startPos[0] = pos[0];
			m_startPos[1] = pos[1];
			m_touchScreenStartTime = timestamp;
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
					m_dispatcher->runCallback(&event);
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
			case TOUCHSCREEN:
				m_touchScreenInHoldEvent = false;
				m_touchScreenInMoveEvent = false;
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
				m_dispatcher->runCallback(&event);
			}
			break;
		case TOUCHAREA:
			{
				if (m_tapDispatcher && type == SCREEN_EVENT_MTOUCH_RELEASE
						&& (timestamp - m_touchDownTime) < TAP_THRESHOLD) {
					m_tapDispatcher->runCallback(0);
				} else {
					if (type == SCREEN_EVENT_MTOUCH_TOUCH)
						m_touchDownTime = timestamp;

					TouchAreaEventDispatcher::TouchAreaEvent event;
					event.dx = pos[0] - m_lastPos[0];
					event.dy = pos[1] - m_lastPos[1];
					if (event.dx != 0 || event.dy != 0) {
						m_dispatcher->runCallback(&event);
						m_lastPos[0] = pos[0];
						m_lastPos[1] = pos[1];
					}
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
		case TOUCHSCREEN:
			{
				TouchScreenEventDispatcher::TouchScreenEvent event;
				event.x = pos[0];
				event.y = pos[1];
				event.tap = 0;
				event.hold = 0;
				int distance = abs(pos[0] - m_startPos[0]) + abs(pos[1] - m_startPos[1]);
				if (!m_touchScreenInHoldEvent) {
					if ((type == SCREEN_EVENT_MTOUCH_RELEASE)
							&& (timestamp - m_touchScreenStartTime) < TAP_THRESHOLD
							&& distance < JITTER_THRESHOLD) {
						event.tap = 1;
						m_dispatcher->runCallback(&event);
					} else if ((type == SCREEN_EVENT_MTOUCH_MOVE)
							&& (m_touchScreenInMoveEvent
									|| (distance > JITTER_THRESHOLD))) {
						m_touchScreenInMoveEvent = true;
						m_dispatcher->runCallback(&event);
					} else if ((type == SCREEN_EVENT_MTOUCH_MOVE)
							&& (!m_touchScreenInMoveEvent)
							&& (timestamp - m_touchScreenStartTime) > 2*TAP_THRESHOLD) {
						event.hold = 1;
						m_touchScreenInHoldEvent = true;
						m_dispatcher->runCallback(&event);
					}
				}
			}
			break;
		default:
			break;
		}

		if (type == SCREEN_EVENT_MTOUCH_RELEASE) {
			//fprintf(stderr, "Update touch - release\n");
			m_contactId = -1;
			m_touchScreenInHoldEvent = false;
			m_touchScreenInMoveEvent = false;
			return false;
		}
	}

	return true;
}

/*
 * control.cpp
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#include "control.h"
#include "eventdispatcher.h"
#include <math.h>

#include "png.h"
#include <screen/screen.h>

const static int pngRed = 0;
const static int pngGreen = 1;
const static int pngBlue = 2;
const static int pngAlpha = 3;

struct TrackedPNG
{
	TrackedPNG(FILE *file)
		: m_read(0)
		, m_info(0)
		, m_data(0)
		, m_rows(0)
		, m_buffer(0)
		, m_width(0)
		, m_height(0)
		, m_stride(0)
		, m_file(file)
	{}

	~TrackedPNG()
	{
		delete [] m_rows;
		delete [] m_data;

		if (m_read)
			png_destroy_read_struct(&m_read, m_info ? &m_info : (png_infopp) 0, (png_infopp) 0);
		if (m_buffer)
			screen_destroy_buffer(m_buffer);
		if (m_file)
			fclose(m_file);
	}

	png_structp m_read;
	png_infop m_info;
	unsigned char* m_data;
	png_bytep* m_rows;
	screen_buffer_t m_buffer;
	int m_width;
	int m_height;
	int m_stride;
	FILE *m_file;
};

Control::Control(screen_context_t context, ControlType type, int x, int y, unsigned width, unsigned height, EventDispatcher *dispatcher)
	: m_type(type)
	, m_x(x)
	, m_y(y)
	, m_width(width)
	, m_height(height)
	, m_srcWidth(width)
	, m_srcHeight(height)
	, m_dispatcher(dispatcher)
	, m_context(context)
	, m_contactId(-1)
{
	m_lastPos[0] = 0;
	m_lastPos[1] = 0;
}

Control::~Control()
{
	screen_destroy_pixmap(m_pixmap);
	delete m_dispatcher;
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

	TrackedPNG png(file);

	png.m_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png.m_read) {
		fprintf(stderr, "Failed to create PNG read struct\n");
		return false;
	}

	png.m_info = png_create_info_struct(png.m_read);
	if (!png.m_info) {
		fprintf(stderr, "Failed to create PNG info struct\n");
		return false;
	}

	// Exception handling
	if (setjmp(png_jmpbuf(png.m_read))) {
		fprintf(stderr, "PNG jumped to failure\n");
		return false;
	}

	png_init_io(png.m_read, file);
	png_read_info(png.m_read, png.m_info);

	png.m_width = png_get_image_width(png.m_read, png.m_info);
	if (png.m_width <= 0) {
		fprintf(stderr, "Invalid PNG width: %d\n", png.m_width);
		return false;
	}

	png.m_height = png_get_image_height(png.m_read, png.m_info);
	if (png.m_height <= 0) {
		fprintf(stderr, "Invalid PNG height: %d\n", png.m_height);
		return false;
	}

	const int channels = 4;
	png_set_palette_to_rgb(png.m_read);
	png_set_tRNS_to_alpha(png.m_read);
	if (png_get_channels(png.m_read, png.m_info) < channels)
		png_set_filler(png.m_read, 0xff, PNG_FILLER_AFTER);

	png.m_data = new unsigned char[png.m_width * png.m_height * channels];
	png.m_stride = png.m_width * channels;
	png.m_rows = new png_bytep[png.m_height];

	for (int i=png.m_height - 1; i >= 0; --i) {
		png.m_rows[i] = (png_bytep)(png.m_data + i * png.m_stride);
	}
	png_read_image(png.m_read, png.m_rows);

	int rc;
	{
		int format = SCREEN_FORMAT_RGBA8888;
		int size[2] = {png.m_width, png.m_height};
		m_srcWidth = png.m_width;
		m_srcHeight = png.m_height;

		rc = screen_create_pixmap(&m_pixmap, m_context); // FIXME: Check failure
		rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_FORMAT, &format);
		rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
		rc = screen_create_pixmap_buffer(m_pixmap);

		unsigned char *realPixels;
		int realStride;
		rc = screen_get_pixmap_property_pv(m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&m_buffer);
		rc = screen_get_buffer_property_pv(m_buffer, SCREEN_PROPERTY_POINTER, (void **)&realPixels);
		rc = screen_get_buffer_property_iv(m_buffer, SCREEN_PROPERTY_STRIDE, &realStride);
		memcpy(realPixels, png.m_data, realStride * m_srcHeight);
	}

	return true;
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
				m_dispatcher->runCallback(&event);
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

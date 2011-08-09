/*
 * window.cpp
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#include "window.h"
#include <errno.h>
#include <string.h>
#include <screen/screen.h>

EmulationWindow::EmulationWindow(screen_context_t screenContext, screen_window_t parent)
	: m_context(screenContext)
{
	m_valid = false;
	int rc;
	int format = SCREEN_FORMAT_RGBA8888;
	int usage = SCREEN_USAGE_NATIVE;
	int size[2] = {1024, 600};

	rc = screen_get_window_property_iv(parent, SCREEN_PROPERTY_SIZE, size);
	if (rc) {
		perror("screen_get_window_property_iv(size)");
		return;
	}
	m_size[0] = size[0];
	m_size[1] = size[1];

	rc = screen_create_window_type(&m_window, screenContext, SCREEN_CHILD_WINDOW);
	if (rc) {
		perror("screen_create_window");
		return;
	}

	rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_FORMAT, &format);
	if (rc) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
		screen_destroy_window(m_window);
		m_window = 0;
		return;
	}

	rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_USAGE, &usage);
	if (rc) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
		screen_destroy_window(m_window);
		m_window = 0;
		return;
	}

	rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_SIZE, size);
	if (rc) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE)");
		screen_destroy_window(m_window);
		m_window = 0;
		return;
	}

	rc = screen_create_window_buffers(m_window, 2);
	if (rc) {
		perror("screen_create_window_buffers");
		screen_destroy_window(m_window);
		m_window = 0;
		return;
	}

	if (parent != 0) {
		char buffer[256] = {0};

		rc = screen_get_window_property_cv(parent, SCREEN_PROPERTY_GROUP, 256, buffer);
		if (rc) {
			perror("screen_get_window_property_cv(SCREEN_PROPERTY_GROUP)");
			screen_destroy_window(m_window);
			m_window = 0;
			return;
		}

		rc = screen_join_window_group(m_window, buffer);
		if (rc) {
			perror("screen_join_window_group");
			screen_destroy_window(m_window);
			m_window = 0;
			return;
		}
	}

	m_valid = true;
}

bool EmulationWindow::getPixels(screen_buffer_t *buffer, unsigned char **pixels, int *stride) const
{
	screen_buffer_t buffers[2];
	int rc = screen_get_window_property_pv(m_window,
			SCREEN_PROPERTY_RENDER_BUFFERS, (void**)buffers);
	if (rc) {
		fprintf(stderr, "Cannot get window render buffers: %s", strerror(errno));
		return false;
	}
	*buffer = buffers[0];

	rc = screen_get_buffer_property_pv(*buffer, SCREEN_PROPERTY_POINTER, (void **)pixels);
	if (rc) {
		fprintf(stderr, "Cannot get buffer pointer: %s", strerror(errno));
		return false;
	}

	rc = screen_get_buffer_property_iv(*buffer, SCREEN_PROPERTY_STRIDE, stride);
	if (rc) {
		fprintf(stderr, "Cannot get stride: %s", strerror(errno));
		return false;
	}
	return true;
}

bool EmulationWindow::setZOrder(int zOrder) const
{
	int rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_ZORDER, &zOrder);
	if (rc) {
		fprintf(stderr, "Cannot set z-order: %s", strerror(errno));
		return false;
	}
	return true;
}

bool EmulationWindow::setTouchSensitivity(bool isSensitive) const
{
	int sensitivity = (isSensitive)?SCREEN_SENSITIVITY_ALWAYS:SCREEN_SENSITIVITY_NEVER;
	int rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_SENSITIVITY, &sensitivity);
	if (rc) {
		fprintf(stderr, "Cannot set screen sensitivity: %s", strerror(errno));
		return false;
	}
	return true;
}

void EmulationWindow::post(screen_buffer_t buffer) const
{
	int dirtyRects[4] = {0, 0, m_size[0], m_size[1]};
	screen_post_window(m_window, buffer, 1, dirtyRects, 0);
}

EmulationWindow::~EmulationWindow()
{
	if (m_window) {
		screen_destroy_window(m_window);
		m_window = 0;
	}
}

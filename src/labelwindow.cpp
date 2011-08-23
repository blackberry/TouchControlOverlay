/*
 * labelwindow.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: jnicholl
 */

#include "labelwindow.h"
#include "pngreader.h"

LabelWindow *LabelWindow::create(screen_context_t context, int width, int height)
{
	const int zOrder = 6; // FIXME: hardcoded
	LabelWindow *window = new LabelWindow(context, width, height);
	if (!window->m_valid) {
		delete window;
		return 0;
	}

	if (!window->setZOrder(zOrder) ||
			!window->setTouchSensitivity(false)) {
		delete window;
		return 0;
	}

	return window;
}

void LabelWindow::draw(PNGReader &reader)
{
	screen_buffer_t buffer;
	unsigned char *pixels;
	int stride;
	if (!getPixels(&buffer, &pixels, &stride)) {
		fprintf(stderr, "Unable to get label window buffer\n");
		return;
	}
//
//	if (reader.m_stride != stride || reader.m_height != m_size[1]) {
//		fprintf(stderr, "Memcpy is unsafe!!!\n");
//	}
//	memcpy(pixels, reader.m_data, reader.m_stride * reader.m_height);
	screen_buffer_t pixmapBuffer;
	screen_get_pixmap_property_pv(reader.m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&pixmapBuffer);

	int attribs[] = {
			SCREEN_BLIT_SOURCE_X, 0,
			SCREEN_BLIT_SOURCE_Y, 0,
			SCREEN_BLIT_SOURCE_WIDTH, reader.m_width,
			SCREEN_BLIT_SOURCE_HEIGHT, reader.m_height,
			SCREEN_BLIT_DESTINATION_X, 0,
			SCREEN_BLIT_DESTINATION_Y, 0,
			SCREEN_BLIT_DESTINATION_WIDTH, m_size[0],
			SCREEN_BLIT_DESTINATION_HEIGHT, m_size[1],
			SCREEN_BLIT_END
	};
	screen_blit(m_context, buffer, pixmapBuffer, attribs);
	this->post(buffer);
	int visible = 0;
	int rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_VISIBLE, &visible);
}

void LabelWindow::showAt(screen_window_t parent, int x, int y)
{
	move(x, y);
	if (!setParent(parent))
		return;

	int visible = 1;
	int rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_VISIBLE, &visible);
	if (rc) {
		perror("set label window visible: ");
	}
}

void LabelWindow::move(int x, int y)
{
	int position[] = {x, y};
	int rc = screen_set_window_property_iv(m_window, SCREEN_PROPERTY_POSITION, position);
	if (rc) {
		perror("LabelWindow set position: ");
		return;
	}
}

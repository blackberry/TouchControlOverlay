/*
 * label.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: jnicholl
 */

#include "label.h"
#include "labelwindow.h"
#include "control.h"
#include "pngreader.h"

Label::Label(screen_context_t context, int x, int y, unsigned width, unsigned height, char *imageFile)
	: m_x(x)
	, m_y(y)
	, m_width(width)
	, m_height(height)
	, m_window(0)
{
	FILE *file = 0;

	if (imageFile) {
		file = fopen(imageFile, "rb");
	}

	if (file) {
		// We have an image
		PNGReader png(file, context);
		if (png.doRead()) {
			m_window = LabelWindow::create(context, width, height);
			m_window->draw(png);
		}
	}
}

Label::~Label()
{
	m_control = 0;
	delete m_window;
	m_window = 0;
}

void Label::draw(screen_window_t window, int x, int y)
{
	if (!m_window)
		return;
	m_window->showAt(window, m_x+x, m_y+y);
}

void Label::move(int x, int y)
{
	if (!m_window)
		return;
	m_window->move(m_x+x, m_y+y);
}

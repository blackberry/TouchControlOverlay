/*
 * labelwindow.h
 *
 *  Created on: Aug 12, 2011
 *      Author: jnicholl
 */

#ifndef LABELWINDOW_H_
#define LABELWINDOW_H_

#include <screen/screen.h>
#include "window.h"

class PNGReader;

class LabelWindow : public EmulationWindow
{
public:
	static LabelWindow *create(screen_context_t context, int width, int height);

	void draw(PNGReader &reader);
	void showAt(screen_window_t parent, int x, int y);
	void move(int x, int y);

protected:
	LabelWindow(screen_context_t screenContext, int width, int height)
		: EmulationWindow(screenContext, width, height)
	{}
};

#endif /* LABELWINDOW_H_ */

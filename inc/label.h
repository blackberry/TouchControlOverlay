/*
 * label.h
 *
 *  Created on: Aug 12, 2011
 *      Author: jnicholl
 */

#ifndef LABEL_H_
#define LABEL_H_

#include <screen/screen.h>

class Control;
class LabelWindow;

class Label
{
public:
	Label(screen_context_t context, int x, int y, unsigned width, unsigned height, char *imageFile);
	~Label();

	void setControl(Control *control) { m_control = control; }
	void draw(screen_window_t window, int x, int y);
	void move(int x, int y);
private:
	int m_x;
	int m_y;
	unsigned m_width;
	unsigned m_height;
	Control *m_control;
	LabelWindow *m_window;
};

#endif /* LABEL_H_ */

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

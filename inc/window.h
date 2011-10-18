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

#ifndef WINDOW_H_
#define WINDOW_H_

#include <screen/screen.h>

class EmulationWindow
{
public:
	virtual ~EmulationWindow();

protected:
	EmulationWindow(screen_context_t screenContext, screen_window_t parent=0);
	EmulationWindow(screen_context_t screenContext, int width, int height, screen_window_t parent=0);

	bool setZOrder(int zOrder) const;
	bool setTouchSensitivity(bool isSensitive) const;
	bool getPixels(screen_buffer_t *buffer, unsigned char **pixels, int *stride) const;
	void post(screen_buffer_t buffer) const;

	bool setParent(screen_window_t parent);

	bool m_valid;
	screen_context_t m_context;
	screen_window_t m_window;
	screen_window_t m_parent;
	int m_size[2];

private:
	void init(screen_window_t parent);
};

#endif /* WINDOW_H_ */

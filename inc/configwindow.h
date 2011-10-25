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

#ifndef CONFIGWINDOW_H_
#define CONFIGWINDOW_H_

#include <screen/screen.h>
#include "window.h"

class Control;
class TCOContext;

class ConfigWindow : public EmulationWindow
{
public:
	static ConfigWindow *createConfigWindow(screen_context_t context, screen_window_t parent=0);

	void runEventLoop(TCOContext *emuContext);

protected:
	ConfigWindow(screen_context_t screenContext, screen_window_t parent=0)
		: EmulationWindow(screenContext, parent)
		, m_selected(0)
	{}

private:
	screen_buffer_t draw(TCOContext *emuContext);

	Control *m_selected;
};

#endif /* CONFIGWINDOW_H_ */

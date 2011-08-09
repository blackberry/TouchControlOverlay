/*
 * configwindow.h
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#ifndef CONFIGWINDOW_H_
#define CONFIGWINDOW_H_

#include <screen/screen.h>
#include "window.h"

class EmulationContext;

class ConfigWindow : public EmulationWindow
{
public:
	static ConfigWindow *createConfigWindow(screen_context_t context, screen_window_t parent=0);

	void runEventLoop(EmulationContext *emuContext);
protected:
	ConfigWindow(screen_context_t screenContext, screen_window_t parent=0)
		: EmulationWindow(screenContext, parent)
	{}
};

#endif /* CONFIGWINDOW_H_ */

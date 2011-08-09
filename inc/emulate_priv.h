/*
 * emulate_priv.h
 *
 *  Created on: Aug 5, 2011
 *      Author: jnicholl
 */

#ifndef EMULATE_PRIV_H_
#define EMULATE_PRIV_H_

#include <map>
#include <vector>

#include <screen/screen.h>

class Control;
class ConfigWindow;

struct emu_callbacks;

class EmulationContext
{
public:
	EmulationContext(screen_context_t context, emu_callbacks callbacks);
	~EmulationContext();

	int showConfig(screen_window_t window);
	bool touchEvent(screen_event_t event);

	int loadControls(const char *filename);

	void drawControls(screen_buffer_t buffer);

private:
	screen_context_t m_screenContext;
	screen_window_t m_appWindow;
	ConfigWindow *m_configWindow;

	std::vector<Control*> m_controls;
	std::map<int, Control *> m_controlMap;

	int (*m_handleKeyFunc)(int sym, int mod, int scancode, uint16_t unicode, int event);
	int (*m_handleDPadFunc)(int angle, int event);
	int (*m_handleTouchFunc)(int dx, int dy);
	int (*m_handleMouseButtonFunc)(int button, int mask, int event);
	int (*m_handlePassThruButtonFunc)(int x, int y);
};

#endif /* EMULATE_PRIV_H_ */

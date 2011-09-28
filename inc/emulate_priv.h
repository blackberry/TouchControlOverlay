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
	typedef int(*HandleKeyFunc)(int sym, int mod, int scancode, uint16_t unicode, int event);
	typedef int(*HandleDPadFunc)(int angle, int event);
	typedef int(*HandleTouchFunc)(int dx, int dy);
	typedef int(*HandleMouseButtonFunc)(int button, int mask, int event);
	typedef int(*HandleTapFunc)();
	typedef int(*HandleTouchScreenFunc)(int x, int y, int tap, int hold);

	EmulationContext(screen_context_t context, emu_callbacks callbacks);
	~EmulationContext();

	int showLabels(screen_window_t window);

	int showConfig(screen_window_t window);
	bool touchEvent(screen_event_t event);
	void frameStart();
	void frameEnd();
	Control *controlAt(int pos[]) const;

	int loadControls(const char *filename);
	void drawControls(screen_buffer_t buffer);

	HandleKeyFunc handleKeyFunc() const { return m_handleKeyFunc; }
	HandleDPadFunc handleDPadFunc() const { return m_handleDPadFunc; }
	HandleTouchFunc handleTouchFunc() const { return m_handleTouchFunc; }
	HandleMouseButtonFunc handleMouseButtonFunc() const { return m_handleMouseButtonFunc; }
	HandleTapFunc handleTapFunc() const { return m_handleTapFunc; }
	HandleTouchScreenFunc handleTouchScreenFunc() const { return m_handleTouchScreenFunc; }

	screen_context_t screenContext() const { return m_screenContext; }
private:
	screen_context_t m_screenContext;
	screen_window_t m_appWindow;
	ConfigWindow *m_configWindow;

	std::vector<Control*> m_controls;
	std::map<int, Control *> m_controlMap;

	HandleKeyFunc m_handleKeyFunc;
	HandleDPadFunc m_handleDPadFunc;
	HandleTouchFunc m_handleTouchFunc;
	HandleMouseButtonFunc m_handleMouseButtonFunc;
	HandleTapFunc m_handleTapFunc;
	HandleTouchScreenFunc m_handleTouchScreenFunc;
};

#endif /* EMULATE_PRIV_H_ */

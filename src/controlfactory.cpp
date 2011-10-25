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

#include "controlfactory.h"
#include "control.h"
#include "eventdispatcher.h"
#include "touchcontroloverlay_priv.h"
#include "label.h"
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

template<typename T>
bool getProperty(xmlAttr *properties, const char *propertyName, T& value) {
	std::stringstream ss;
	if (!xmlStrncasecmp(properties->name, BAD_CAST propertyName, strlen(propertyName))) {
		if (properties->children && properties->children->content) {
			ss.clear();
			ss.str("");
			ss << properties->children->content;
			ss >> value;
			return true;
		}
	}
	return false;
}

bool getButtonProperty(xmlAttr *properties, const char *propertyName, int& value) {
	if (!xmlStrncasecmp(properties->name, BAD_CAST propertyName, strlen(propertyName))) {
		if (properties->children && properties->children->content) {
			if (!xmlStrncasecmp(properties->children->content, BAD_CAST "left", strlen("left"))) {
				value = TCO_MOUSE_LEFT_BUTTON;
			} else if (!xmlStrncasecmp(properties->children->content, BAD_CAST "right", strlen("right"))) {
				value = TCO_MOUSE_RIGHT_BUTTON;
			} else if (!xmlStrncasecmp(properties->children->content, BAD_CAST "middle", strlen("middle"))) {
				value = TCO_MOUSE_MIDDLE_BUTTON;
			} else
				return false;
			return true;
		}
	}
	return false;
}

Control *ControlFactory::createControl(TCOContext *context, xmlNode *node)
{
	Control::ControlType type = Control::KEY;
	int x = 0, y = 0;
	unsigned w = 100, h = 100;
	int sym = 0, mod = 0, scancode = 0;
	uint16_t unicode = 0;
	int mask = 0, button = 0;
	xmlChar *imageFile = 0;
	int tapSensitive = 0;

	if (!xmlStrncasecmp(node->name, BAD_CAST "key", strlen("key"))) {
		type = Control::KEY;
	} else if (!xmlStrncasecmp(node->name, BAD_CAST "dpad", strlen("dpad"))) {
		type = Control::DPAD;
	} else if (!xmlStrncasecmp(node->name, BAD_CAST "toucharea", strlen("toucharea"))) {
		type = Control::TOUCHAREA;
	} else if (!xmlStrncasecmp(node->name, BAD_CAST "mousebutton", strlen("mousebutton"))) {
		type = Control::MOUSEBUTTON;
	} else if (!xmlStrncasecmp(node->name, BAD_CAST "touchscreen", strlen("touchscreen"))) {
		type = Control::TOUCHSCREEN;
	} else {
		type = (Control::ControlType)-1;
	}

	xmlAttr *properties = node->properties;
	while (properties)
	{
		getProperty(properties, "x", x);
		getProperty(properties, "y", y);
		getProperty(properties, "width", w);
		getProperty(properties, "height", h);
		if (!xmlStrncasecmp(properties->name, BAD_CAST "image", strlen("image"))) {
			if (properties->children && properties->children->content)
				imageFile = properties->children->content;
		}

		switch (type) {
		case Control::KEY:
			getProperty(properties, "sym", sym);
			getProperty(properties, "mod", mod);
			getProperty(properties, "scancode", scancode);
			getProperty(properties, "unicode", unicode);
			break;
		case Control::TOUCHAREA:
			getProperty(properties, "tapSensitive", tapSensitive);
			break;
		case Control::MOUSEBUTTON:
			getProperty(properties, "mask", mask);
			getButtonProperty(properties, "button", button);
			break;
		}
		properties = properties->next;
	}

	Control *control = 0;
	switch (type) {
	case Control::KEY:
		control = new Control(context->screenContext(), type, x, y, w, h,
				new KeyEventDispatcher(context->handleKeyFunc(), sym, mod, scancode, unicode));
		break;
	case Control::DPAD:
		control = new Control(context->screenContext(), type, x, y, w, h,
				new DPadEventDispatcher(context->handleDPadFunc()));
		break;
	case Control::TOUCHAREA:
		if (tapSensitive > 0) {
			control = new Control(context->screenContext(), type, x, y, w, h,
					new TouchAreaEventDispatcher(context->handleTouchFunc()),
					new TapDispatcher(context->handleTapFunc()));
		} else {
			control = new Control(context->screenContext(), type, x, y, w, h,
					new TouchAreaEventDispatcher(context->handleTouchFunc()));
		}
		break;
	case Control::MOUSEBUTTON:
		control = new Control(context->screenContext(), type, x, y, w, h,
				new MouseButtonEventDispatcher(context->handleMouseButtonFunc(), mask, button));
		break;
	case Control::TOUCHSCREEN:
		control = new Control(context->screenContext(), type, x, y, w, h,
				new TouchScreenEventDispatcher(context->handleTouchScreenFunc()));
		break;
	default:
		return 0;
	}

	xmlNode *child = node->children;
	xmlChar *childImage;
	Label *label;
	while (child) {
		if (!xmlStrncasecmp(child->name, BAD_CAST "label", strlen("label"))) {
			xmlAttr *properties = child->properties;
			childImage = 0;
			while (properties)
			{
				getProperty(properties, "x", x);
				getProperty(properties, "y", y);
				getProperty(properties, "width", w);
				getProperty(properties, "height", h);
				if (!xmlStrncasecmp(properties->name, BAD_CAST "image", strlen("image"))) {
					if (properties->children && properties->children->content)
						childImage = properties->children->content;
				}

				properties = properties->next;
			}
			label = new Label(context->screenContext(), x, y, w, h, (char*)childImage);
			control->addLabel(label);
		}
		child = child->next;
	}

	if (imageFile) {
		if (!control->loadFromPNG((char*)(imageFile)))
			fprintf(stderr, "Failed to load from PNG\n");
	} else {
		control->fill();
	}
	return control;
}

Control *ControlFactory::createControl(TCOContext *context, int controlType, int x, int y, int w, int h, std::stringstream &ss)
{
	int extra[3];
	uint16_t unicode;
	Control *control = 0;

	Control::ControlType type = static_cast<Control::ControlType>(controlType);
	switch (type) {
	case Control::KEY:
		ss >> extra[0] >> extra[1] >> extra[2] >> unicode;
		control = new Control(context->screenContext(), type, x, y, w, h,
				new KeyEventDispatcher(context->handleKeyFunc(), extra[0], extra[1], extra[2], unicode));
		break;
	case Control::DPAD:
		control = new Control(context->screenContext(), type, x, y, w, h,
				new DPadEventDispatcher(context->handleDPadFunc()));
		break;
	case Control::TOUCHAREA:
		control = new Control(context->screenContext(), type, x, y, w, h,
				new TouchAreaEventDispatcher(context->handleTouchFunc()));
		break;
	case Control::MOUSEBUTTON:
		ss >> extra[0] >> extra[1];
		control = new Control(context->screenContext(), type, x, y, w, h,
				new MouseButtonEventDispatcher(context->handleMouseButtonFunc(), extra[0], extra[1]));
		break;
	default:
		return 0;
	}
	return control;
}

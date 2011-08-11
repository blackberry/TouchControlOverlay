/*
 * controlfactory.h
 *
 *  Created on: Aug 9, 2011
 *      Author: jnicholl
 */

#ifndef CONTROLFACTORY_H_
#define CONTROLFACTORY_H_

#include <sstream>
#include <screen/screen.h>
#include <libxml/tree.h>

class Control;
class EmulationContext;

class ControlFactory
{
public:
	static Control *createControl(EmulationContext *context, int type, int x, int y, int w, int h, std::stringstream &ss);
	static Control *createControl(EmulationContext *context, xmlNode *node);
};

#endif /* CONTROLFACTORY_H_ */

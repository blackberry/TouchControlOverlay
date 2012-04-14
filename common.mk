ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Library that is used to convert touch screen input into other forms of input
endef

NAME=TouchControlOverlay

PUBLIC_INCVPATH = $(PROJECT_ROOT)/public
EXTRA_INCVPATH = $(PROJECT_ROOT)/inc
EXTRA_SRCVPATH = $(PROJECT_ROOT)/src

LIBS=xml2 png14 screen bps m

include $(MKFILES_ROOT)/qtargets.mk

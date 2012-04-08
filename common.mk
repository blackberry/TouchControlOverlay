ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Library for manipulating touchscreen
endef

NAME=TouchControlOverlay

PUBLIC_INCVPATH = $(PROJECT_ROOT)/public
EXTRA_INCVPATH = $(PROJECT_ROOT)/inc
EXTRA_SRCVPATH = $(PROJECT_ROOT)/src

include $(MKFILES_ROOT)/qtargets.mk

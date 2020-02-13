.SUFFIXES: .c .cpp .cxx .cc .s .S .asm .h .hpp .d

TARGET      ?= $(notdir $(shell pwd)).a

CFLAGS	    ?=
CXXFLAGS    ?=
ASFLAGS	    ?=
ARFLAGS	    ?= -rcs

DEFINES     ?=
INCLUDES    ?= include
LIBS        ?= c m

SRCDIRS     ?= .
DESTDIR     ?= $(SYSROOT)/usr/lib



include $(ROOTDIR)/extra/build/build-sources.mk
include $(ROOTDIR)/extra/build/build-flags.mk

$(TARGET): $(OBJS) $(HDRS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

include $(ROOTDIR)/extra/build/build-objects.mk
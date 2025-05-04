# ----------------------------------------------------------- -*- mode: makefile -*-
# List of targets to build
# ----------------------------------------------------------------------------

include ../dbus-gmain.mk

NAME = keepdisplayonplugin.a
ROOT     ?= /tmp/test/$(NAME)
SOURCES = keep-display-on-plugin.c
OBJECTS=$(SOURCES:.c=.o)


# ----------------------------------------------------------------------------
# Standard install directories
# ----------------------------------------------------------------------------

_PREFIX         ?= /usr#                         # /usr
_INCLUDEDIR     ?= $(_PREFIX)/include#           # /usr/include
_EXEC_PREFIX    ?= $(_PREFIX)#                   # /usr
_BINDIR         ?= $(_EXEC_PREFIX)/bin#          # /usr/bin
_SBINDIR        ?= $(_EXEC_PREFIX)/sbin#         # /usr/sbin
_LIBEXECDIR     ?= $(_EXEC_PREFIX)/libexec#      # /usr/libexec
_LIBDIR         ?= $(_EXEC_PREFIX)/lib#          # /usr/lib
_SYSCONFDIR     ?= /etc#                         # /etc
_DATADIR        ?= $(_PREFIX)/share#             # /usr/share
_MANDIR         ?= $(_DATADIR)/man#              # /usr/share/man
_INFODIR        ?= $(_DATADIR)/info#             # /usr/share/info
_DEFAULTDOCDIR  ?= $(_DATADIR)/doc#              # /usr/share/doc
_LOCALSTATEDIR  ?= /var#                         # /var
_UNITDIR        ?= /lib/systemd/system#
_TESTSDIR       ?= /opt/tests#                   # /opt/tests

#
CC = gcc  # C compiler
# ----------------------------------------------------------------------------
# Default flags
# ----------------------------------------------------------------------------

CPPFLAGS += -D_GNU_SOURCE
CPPFLAGS += -D_FILE_OFFSET_BITS=64

CFLAGS   += -Wall
CFLAGS   += -Os
#CFLAGS   += -Wextra
CFLAGS   += -std=c99
CFLAGS   += -g
CFLAGS   += -fPIC


LDFLAGS  += -static
LDLIBS   += -Wl,--as-needed

# ----------------------------------------------------------------------------
# Flags from pkg-config
# ----------------------------------------------------------------------------

PKG_NAMES  += glib-2.0
PKG_NAMES  += dbus-1
PKG_NAMES  += keepalive-glib
#PKG_NAMES  += mpv

PKG_CFLAGS := $(shell pkg-config --cflags $(PKG_NAMES))
PKG_LDLIBS := $(shell pkg-config --libs   $(PKG_NAMES))

CFLAGS     += $(PKG_CFLAGS)
LDLIBS     += $(PKG_LDLIBS)

# ----------------------------------------------------------------------------
# Top level targets
# ----------------------------------------------------------------------------

.PHONY: build install clean distclean mostlyclean

all: $(NAME)
$(NAME):$ $(OBJECTS) $(DBUS_GMAIN_DIR)/dbus-gmain.o
	 ar rcs $(NAME) $(OBJECTS) $(DBUS_GMAIN_DIR)/dbus-gmain.o


build:: $(NAME)

install:: build

clean:: mostlyclean
	$(RM) $(NAME)

distclean:: clean

mostlyclean::
	$(RM) *.o *~ *.bak

install::
	install -m755 -d $(ROOT)$(_LIBDIR)/
	install -m755 keepdisplayonplugin.a $(ROOT)$(_LIBDIR)/



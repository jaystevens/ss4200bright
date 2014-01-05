# vi:et

###############################################################################
# common variables
###############################################################################

AR    = ar
AS    = as
CC    = gcc
CPP   = $(CC) -E
CXX   = g++
LD    = ld
RM    = /bin/rm -f

ARFLAGS  = rv
ASFLAGS  =
CFLAGS   = \
           -Wall \
           -O2 \

CPPFLAGS =
CXXFLAGS = -fno-exceptions -fno-rtti
LDFLAGS  =


###############################################################################
# additional variables
###############################################################################

CP    = /bin/cp -f
ECHO  = /bin/echo
MAKE  = /usr/bin/make -r
MKDIR = /bin/mkdir -p
SED   = /bin/sed


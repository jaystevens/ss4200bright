include ${CF_PROJECT}/build/config.mk
include ${CF_PROJECT}/build/tools.mk

###############################################################################
# mandatory variables
###############################################################################

OBJECTS = testNAS.o

MODULES = nasapi/libnasapi.o

TARGET =


###############################################################################
# optional variable extensions/overrides
###############################################################################

CFLAGS += -I${CF_PROJECT}/include

include ${CF_PROJECT}/build/rules.mk

###############################################################################
# optional variables and rules
###############################################################################

GOALS =  testNas

LDFLAGS =

testNas : $(OBJECTS) $(MODULES)
#	$(MKDIR) $(dir $@)
	$(CXX) -o $@ $^ $(LDFLAGS)


###############################################################################
# optional rule extensions
###############################################################################

all :: $(GOALS)

clean ::
	$(RM) $(GOALS)

clobber ::
	$(RM) $(GOALS)
	$(RM) $(shell find . -name '*~')
	$(RM) $(shell find . -name '*.cache')


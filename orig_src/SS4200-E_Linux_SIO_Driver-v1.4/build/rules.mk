
ifneq ($(MAKELEVEL_LIMIT), $(MAKELEVEL))
.DEFAULT :
	$(MAKE) -C $(@D) $(@F)

.PHONY : all clean clobber

all ::
	$(foreach MODULE, $(MODULES), $(MAKE) -C $(dir $(MODULE));)

all :: $(TARGET)

clean ::
	-$(RM) $(OBJECTS) $(TARGET)
	-$(foreach DIR, $(MODULES), $(MAKE) -i -C $(dir $(DIR)) $@;)

clobber ::
	-$(RM) $(OBJECTS) $(TARGET)
	-$(RM) tags *.d *~
	-$(foreach DIR, $(MODULES), $(MAKE) -i -C $(dir $(DIR)) $@;)

$(TARGET) : $(OBJECTS) $(MODULES)
	$(LD) -r -o $@ $^

phony :

%.d : %.c
	$(CC) -M $(CFLAGS) $< > $@

%.d : %.cpp
	$(CXX) -M $(CFLAGS) $(CXXFLAGS) $< > $@

%.o : %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.o : %.cpp
	$(CXX) -c -o $@ $(CFLAGS) $(CXXFLAGS) $<


ifneq (clobber, $(MAKECMDGOALS))
ifneq (clean, $(MAKECMDGOALS))
-include $(filter %.d, $(OBJECTS:%.o=%.d)) phony
endif
endif
endif


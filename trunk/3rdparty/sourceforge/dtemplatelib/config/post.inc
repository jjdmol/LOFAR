# Written for GNU -*- makefile -*-.

ifdef mydirs
CPPFLAGS += $(mydirs:%=-I%)
LDFLAGS += $(mydirs:%=-L%/$(BUILD))

vpath %.so $(mydirs:%=%/$(BUILD))
vpath %.a $(mydirs:%=%/$(BUILD))

real-clean: clean
	for d in $(mydirs) ; \
	    do (cd $$d; ./build.sh -b $(BUILD) -p $(PLATFORM) clean) ; \
	done
else
real-clean: clean
endif

ifdef mylibs
# This magic turns "libFOOdir = ../foo" into a subbuild
$(mylibs): -l%:
	cd $($(@:-l%=lib%dir)); pwd; ./build.sh $(ARGS) -b $(BUILD) \
		PROJECT_CPPFLAGS='$(PROJECT_CPPFLAGS)'
endif

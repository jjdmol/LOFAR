CFLAGS+=-I/usr/include/python2.2
CPPFLAGS+=-I/usr/include/python2.3

%.cc:	%.cxx
	cp $< $@

%_wrap.cxx:	%.swg
	swig -c++ -python $<

all:
	cd selfcal; make

_array.so:	array.o array_wrap.o
	ld -shared -o $@ $^

clean:
	-@rm *.o
	-@rm _*.so
	-@rm selfcal_wrap.*
	-@rm selfcal.py
	-@rm core
	cd selfcal; make clean

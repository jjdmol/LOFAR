CFLAGS+=-I/usr/include/python2.2
CPPFLAGS+=-I/usr/include/python2.3

%.cc:	%.cxx
	cp $< $@

%_wrap.cxx:	%.swg
	swig -c++ -python $<

_selfcal.so:	selfcal.o selfcal_wrap.o
	ld -shared -o _selfcal.so $^

clean:
	-@rm *.o
	-@rm _selfcal.so
	-@rm selfcal_wrap.*
	-@rm selfcal.py
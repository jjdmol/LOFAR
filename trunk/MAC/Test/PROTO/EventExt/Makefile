AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) $< && \
	chmod a+w $@

%_wrap.cc: %.ph
	swig -c++ -python -shadow -o $@ $<

%.so: %_wrap.o # %.o
	ld -shared -o $*c.so $^

CXXFLAGS=-I/usr/include/python2.2 -g -I.

all: MY_Protocol.ph ExampleUse MY_Protocol.so 

clean:
	rm -f *.o *.so *_wrap.* ExampleUse MyProtocol.py *.pyc MY_Protocol.ph


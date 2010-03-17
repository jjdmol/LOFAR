
AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) $< && \
	chmod a+w $@

%_wrap.cc: %.ph
	swig -I.-IGCF -c++ -python -shadow -o $@ $<

_%.so: %_wrap.o
	g++ -shared -o _$*.so $^

CXXFLAGS=-I/usr/include/python2.2 -g -I. -IGCF

all: _pybind.so MY_Protocol.ph ExampleUse _MY_Protocol.so

clean:
	rm -f *.o *.so *_wrap.* *.pyc \
	ExampleUse \
	MyProtocol.py \
	MY_Protocol.ph \
	MY_Protocol.py \
	pybind.py


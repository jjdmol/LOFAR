
AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) $< && \
	chmod a+w $@

%_wrap.cc: %.ph
	swig -I.-IGCF -c++ -python -shadow -o $@ $<

_%.so: %_wrap.o
	g++ -shared -o _$*.so $^

ExampleUse: MY_Protocol.o ExampleUse.o GCF_Event.o
	g++ MY_Protocol.o ExampleUse.o GCF_Event.o -o ExampleUse

CXXFLAGS=-I/usr/include/python2.2 -g -I. -IGCF

all: MY_Protocol.ph ExampleUse _MY_Protocol.so

clean:
	rm -f *.o *.so *_wrap.* *.pyc \
	ExampleUse \
	MyProtocol.py \
	MY_Protocol.ph \
	MY_Protocol.cc \
	MY_Protocol.py \
	pybind.py


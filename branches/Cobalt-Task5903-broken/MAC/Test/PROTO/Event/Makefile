
AUTOGEN = autogen
SUFFIXES = .ph

CXXFLAGS=-I/usr/include/python2.2 -g -I. -IGCF

%.ph: %.prot protocol.tpl
	$(AUTOGEN) $< && \
	chmod a+w $@

%.cc: %.prot protocol.tpl
	$(AUTOGEN) $< && \
	chmod a+w $@

%_wrap.cc: %.ph
	swig -I.-IGCF -c++ -python -shadow -o $@ $<

_%.so: %_wrap.o GCF_Event.o %.o
	g++ -shared -o _$*.so $^

all: MY_Protocol.ph ExampleUse _MY_Protocol.so

ExampleUse: MY_Protocol.o ExampleUse.o GCF_Event.o
	g++ MY_Protocol.o ExampleUse.o GCF_Event.o -o ExampleUse

clean:
	rm -f *.o *.so *_wrap.* *.pyc \
	ExampleUse \
	MyProtocol.py \
	MY_Protocol.ph \
	MY_Protocol.cc \
	MY_Protocol.py \
	pybind.py


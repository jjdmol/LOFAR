
%_wrap.cc: %.ph
	swig -c++ -python -shadow -o $@ $<

%_wrap.cc: %.i
	swig -c++ -python -shadow -o $@ $<

%.so: %_wrap.o # %.o
	ld -shared -o $*c.so $^

CXXFLAGS=-I/usr/include/python2.2 -g

all: MyProtocol.so example.so ExampleUse

clean:
	rm -f *.o *.so *_wrap.* ExampleUse MyProtocol.py example.py *.pyc


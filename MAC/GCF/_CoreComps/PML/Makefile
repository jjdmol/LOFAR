lof_subdir = $(shell pwd | sed -e "s%[a-zA-Z0-9/.\-_]*LOFAR/%%g")
back_dir = $(shell echo $(lof_subdir) | sed -e "s%/[^/]\+%/..%g" -e "s%^[^/]\+%..%")

all:
	cd ./build/gnu_debug && make && echo done

all_opt:
	cd ./build/gnu_opt && make && echo done

all_dopt:
	cd ./build/gnu_dopt && make && echo done

check:
	cd ./build/gnu_debug && make check && echo done

aids:
	cd ./build/gnu_debug && make aids && echo done

clean:
	cd ./build/gnu_debug && make clean && echo done

distcd:
	cd ./build/gnu_debug && make distclean && echo done

.PHONY: config configure bootstrap

bootstrap:
	./cleanup && \
	test -f bootstrap || (sed '$(back_dir)/bootstrap' -e "s%autoconf%$(back_dir)/autoconf%g" > bootstrap && chmod a+x bootstrap ) && \
	test -d build || (mkdir build && mkdir build/gnu_debug) && \
	./bootstrap && cd build/gnu_debug && export CC=gcc296 CXX=g++296 && ../../lofarconf && echo done

configure:
	test -d build || (mkdir build && mkdir build/gnu_debug) && \
	cd build/gnu_debug && export CC=gcc296 CXX=g++296 && ../../lofarconf && echo done

configure_opt:
	test -f bootstrap || (sed '$(back_dir)/bootstrap' -e "s%autoconf%$(back_dir)/autoconf%g" > bootstrap && chmod a+x bootstrap ) && \
	test -d build || (mkdir build && mkdir build/gnu_opt) && \
	./bootstrap && cd build/gnu_opt && ../../lofarconf && echo done

configure_dopt:
	test -f bootstrap || (sed '$(back_dir)/bootstrap' -e "s%autoconf%$(back_dir)/autoconf%g" > bootstrap && chmod a+x bootstrap ) && \
	test -d build || (mkdir build && mkdir build/gnu_dopt) && \
	./bootstrap && cd build/gnu_dopt && ../../lofarconf && echo done

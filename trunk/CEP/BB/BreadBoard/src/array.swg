%module array
%{
#include "array.h"
%}

%include carrays.i
%array_class(float,pars);

%include array.h

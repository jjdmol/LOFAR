%module selfcal
%{
#include "selfcal.h"
#include "../SelfcalEngineStub.h"
#include <string>
%}

%include selfcal.h
%include carrays.i
%include cpointer.i
%include typemaps.i
%array_class(float,pars);
%array_class(bool,bools);

%include "../SelfcalEngineStub.h"

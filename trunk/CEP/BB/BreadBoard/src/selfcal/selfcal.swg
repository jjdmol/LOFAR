%module selfcal
%{
#include "selfcal.h"
#include "../SelfcalEngineStub.h"
#include <string>
%}

%include carrays.i
%include cpointer.i
%array_class(float,pars);
%array_class(bool,bools);
%pointer_functions(float,floatp);

%include selfcal.h
%include "../SelfcalEngineStub.h"


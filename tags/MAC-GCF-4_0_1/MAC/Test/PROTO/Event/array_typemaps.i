
%typemap(in) std::string* ($*1_ltype tempstr) {
	char * temps; int templ;
	if (PyString_AsStringAndSize($input, &temps, &templ)) return NULL;
	tempstr = $*1_ltype(temps, templ);
	$1 = &tempstr;
}
%typemap(out) std::string* {
	$result = PyString_FromStringAndSize($1->data(), $1->length());
}

#define INPUT_ARRAY_TYPEMAP(type, converter)								\
%typemap(in) type [ANY] (type temp[$1_dim0]) {						\
  int i;										\
  if (!PySequence_Check($input)) {							\
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");				\
    return NULL;									\
  }											\
  if (PySequence_Length($input) != $1_dim0) {						\
    PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected $1_dim0 elements");	\
    return NULL;									\
  }											\
  for (i = 0; i < $1_dim0; i++) {							\
    PyObject *o = PySequence_GetItem($input,i);						\
    if (PyNumber_Check(o)) {								\
      temp[i] = (type)converter(o);							\
    } else {										\
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");		\
      return NULL;									\
    }											\
  }											\
  $1 = temp;										\
}

#define OUTPUT_ARRAY_TYPEMAP(type, converter, convtype)	\
%typemap(out) type [ANY] {				\
  int i;						\
  $result = PyList_New($1_dim0);			\
  for (i = 0; i < $1_dim0; i++) {			\
    PyObject *o = converter(convtype ($1[i]));		\
    PyList_SetItem($result,i,o);			\
  }							\
}

INPUT_ARRAY_TYPEMAP(int, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(short, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(long, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(long long, PyLong_AsLongLong);
INPUT_ARRAY_TYPEMAP(unsigned int, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(unsigned short, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(unsigned long, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(unsigned long long, PyLong_AsUnsignedLongLong);
INPUT_ARRAY_TYPEMAP(unsigned char, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(signed char, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(bool, PyInt_AsLong);
INPUT_ARRAY_TYPEMAP(float, PyFloat_AsDouble);
INPUT_ARRAY_TYPEMAP(double, PyFloat_AsDouble);

OUTPUT_ARRAY_TYPEMAP(int, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(short, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(long, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(long long, PyLong_FromLongLong, (long long));
OUTPUT_ARRAY_TYPEMAP(unsigned int, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(unsigned short, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(unsigned long, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(unsigned long long, PyLong_FromUnsignedLongLong, (unsigned long long));
OUTPUT_ARRAY_TYPEMAP(unsigned char, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(signed char, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(bool, PyInt_FromLong, (long));
OUTPUT_ARRAY_TYPEMAP(float, PyFloat_FromDouble, (double));
OUTPUT_ARRAY_TYPEMAP(double, PyFloat_FromDouble, (double));


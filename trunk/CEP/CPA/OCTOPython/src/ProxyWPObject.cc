#include <OCTOPUSSY/OctoproxyWP.h>
#include <OCTOPUSSY/Octopussy.h>
#include <Python.h>
#include "OctoPython.h"
#include "AID-OctoPython.h"
//#include "structmember.h"

using namespace OctoPython;
    
typedef struct 
{
    PyObject_HEAD
    PyObject *address;
    WPRef wpref;
} PyProxyWP;

// -----------------------------------------------------------------------
// dealloc
// destructor
// -----------------------------------------------------------------------
static void
PyProxyWP_dealloc(PyProxyWP* self)
{
  Py_XDECREF(self->address);
  self->ob_type->tp_free((PyObject*)self);
}

// -----------------------------------------------------------------------
// new
// allocator
// -----------------------------------------------------------------------
static PyObject *
PyProxyWP_new (PyTypeObject *type, PyObject *, PyObject *)
{
  PyProxyWP *self;

  self = (PyProxyWP *)type->tp_alloc(type, 0);
  if( self != NULL ) 
    self->address = PyString_FromString("");

  return (PyObject *)self;
}


// -----------------------------------------------------------------------
// init
// initializer
// -----------------------------------------------------------------------
static int
PyProxyWP_init(PyProxyWP *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = {"wpid", NULL};
  char *wpid;
  if( ! PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &wpid) )
      return -1; 
  try
  {
    AtomicID wpc = wpid ? AtomicID(wpid) : AidPython;
    // Is OCTOPUSSY running? attach new proxy WP to it
    if( !Octopussy::isRunning() )
      returnError(-1,OctoPython,"OCTOPUSSY not initialized");
    self->wpref <<= new Octoproxy::ProxyWP(wpc);
    Octopussy::dispatcher().attach(self->wpref.dewr_p(),DMI::ANONWR);
    // Get address
    self->address = pyFromHIID(self->wpref->address());
    Py_INCREF(self->address);
  }
  catchStandardErrors(-1);
  return 0;
}

//static PyMemberDef PyProxyWP_members[] = 
//{
//    {"address", T_OBJECT_EX, offsetof(PyProxyWP,address), 0,
//     "address of proxy WP"},
//    {NULL}  /* Sentinel */
//};

// -----------------------------------------------------------------------
// address accessor
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_address (PyProxyWP* self)
{
  Py_INCREF(self->address);
  return self->address;
}

// helper function, converts scope string to Message::Scope constant
static int resolveScope (int &scope,const char *chscope)
{
  switch( chscope[0] )
  {
    case 'g': scope = Message::GLOBAL;  return 0;
    case 'h': scope = Message::HOST;    return 0;
    case 'l': scope = Message::LOCAL;   return 0;
  }
  returnError(-1,Value,"illegal scope argument");
}

// -----------------------------------------------------------------------
// subscribe
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_subscribe (PyProxyWP* self,PyObject *args)
{
  PyObject *mask_seq;
  char *chscope;
  if( !PyArg_ParseTuple(args,"Os",&mask_seq,&chscope) )
    return NULL;
  try
  {
    HIID id; int scope;
    // convert arguments
    if( pyToHIID(id,mask_seq) < 0 || resolveScope(scope,chscope)<0 )
      return NULL;
    // subscribe
    self->wpref().subscribe(id,scope);
  }
  catchStandardErrors(NULL);
  returnNone;
}

// -----------------------------------------------------------------------
// unsubscribe
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_unsubscribe (PyProxyWP* self,PyObject *args)
{
  PyObject *mask_seq;
  if( !PyArg_ParseTuple(args,"O",&mask_seq) )
    return NULL;
  try
  {
    HIID id;
    // convert arguments
    if( pyToHIID(id,mask_seq) < 0 )
      return NULL;
    // unsubscribe
    self->wpref().unsubscribe(id);
  }
  catchStandardErrors(NULL);
  returnNone;
}

// -----------------------------------------------------------------------
// send
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_send (PyProxyWP* self,PyObject *args)
{
  PyObject *py_msg,*py_dest;
  if( !PyArg_ParseTuple(args,"OO",&py_msg,&py_dest) )
    return NULL;
  try
  {
    HIID dest;
    Message::Ref msg;
    // convert arguments
    if( pyToHIID(dest,py_dest) < 0 || pyToMessage(msg,py_msg) < 0 )
      return NULL;
    self->wpref().send(msg,dest);
  }
  catchStandardErrors(NULL);
  returnNone;
}

// -----------------------------------------------------------------------
// publish
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_publish (PyProxyWP* self,PyObject *args)
{
  PyObject *py_msg;
  char *chscope;
  if( !PyArg_ParseTuple(args,"Os",&py_msg,&chscope) )
    return NULL;
  try
  {
    Message::Ref msg; int scope;
    // convert arguments
    if( resolveScope(scope,chscope)<0 || pyToMessage(msg,py_msg) < 0 )
      return NULL;
    self->wpref().publish(msg,0,scope);
  }
  catchStandardErrors(NULL);
  returnNone;
}

// -----------------------------------------------------------------------
// num_pending
// returns # of messages in WPs queue
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_num_pending (PyProxyWP* self,PyObject *args)
{
  if( !PyArg_ParseTuple(args,"") )
    return NULL;
  try
  {
    WPInterface &wp = self->wpref();
    if( !wp.isRunning() )
      returnError(NULL,OctoPython,"proxy wp no longer running");
    Thread::Mutex::Lock lock(wp.queueCondition());
    int res = wp.queue().size();
    return PyInt_FromLong(res);
  }
  catchStandardErrors(NULL);
}

// -----------------------------------------------------------------------
// receive
// returns first message from WP's queue
// -----------------------------------------------------------------------
static PyObject * PyProxyWP_receive (PyProxyWP* self,PyObject *args)
{
  int wait=1;
  if( !PyArg_ParseTuple(args,"|i",&wait) )
    return NULL;
  try
  {
    WPInterface &wp = self->wpref();
    Thread::Mutex::Lock lock(wp.queueCondition());
    // wait for something to arrive in queue (if asked to)
    while( wait && wp.queue().empty() )
    {
      if( !wp.isRunning() )
        returnError(NULL,OctoPython,"proxy wp no longer running");
      wp.queueCondition().wait();
    }
    // still empty? Return none (only possible when wait=0)
    if( wp.queue().empty() )
      returnNone;
    // pop first message and return it
    PyObject * py_msg = pyFromMessage(wp.queue().front().mref.deref());
    wp.queue().pop_front();
    return py_msg;
  }
  catchStandardErrors(NULL);
}


// -----------------------------------------------------------------------
// members/data structures init
// -----------------------------------------------------------------------

static PyMethodDef PyProxyWP_methods[] = {
    {"address",     (PyCFunction)PyProxyWP_address, METH_NOARGS,
                  "return the proxy wp's address" },
    {"subscribe",   (PyCFunction)PyProxyWP_subscribe, METH_VARARGS,
                  "add subscription" },
    {"unsubscribe", (PyCFunction)PyProxyWP_unsubscribe, METH_VARARGS,
                  "remove subscription" },
    {"send",        (PyCFunction)PyProxyWP_send, METH_VARARGS,
                  "send message" },
    {"publish",     (PyCFunction)PyProxyWP_publish, METH_VARARGS,
                  "publish message" },
    {"num_pending", (PyCFunction)PyProxyWP_num_pending, METH_VARARGS,
                  "number of pending messages in queue" },
    {"receive",     (PyCFunction)PyProxyWP_receive, METH_VARARGS,
                  "receives message from queue" },
    {NULL}  /* Sentinel */
};

PyTypeObject OctoPython::PyProxyWPType = {
    PyObject_HEAD_INIT(NULL)
    0,                          /*ob_size*/
    "octopython_c.proxy_wp",    /*tp_name*/
    sizeof(PyProxyWP),          /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)PyProxyWP_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PyProxyWP objects",       /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyProxyWP_methods,         /* tp_methods */
    0, /*PyProxyWP_members,*/  /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyProxyWP_init,  /* tp_init */
    0,                         /* tp_alloc */
    PyProxyWP_new,             /* tp_new */
};
                            

#ifndef DIRECTIVEDATABCA_H_HEADER_INCLUDED_C0961372
#define DIRECTIVEDATABCA_H_HEADER_INCLUDED_C0961372

#include <DTL.h>

//##ModelId=3F697FB30119
class DirectiveDataBCA
{
public:
  void operator() (dtl::BoundIOs &cols, 
		   DirectiveData  &rowbuf) {

         cols["id"] << rowbuf.id;
         cols["text"] << rowbuf.text;
      }
};



#endif /* DIRECTIVEDATABCA_H_HEADER_INCLUDED_C0961372 */

#include <string.h>
#include <iostream>
#include <GCF/GCF_Event.h>
class TransObject : public GCFTransportable
{
  public:
    TransObject(int aValue1, float aValue2, string aValue3) :
      value1(aValue1),
      value2(aValue2),
      value3(aValue3)
    {
    }
    TransObject() :
      value1(0),
      value2(0.0),
      value3("")
    {
    }
    virtual ~TransObject() {};
    unsigned int pack(char* buffer)
    {
      unsigned int offset = 0;
      memcpy(buffer + offset, &value1, sizeof(value1));
      offset += sizeof(value1);
      memcpy(buffer + offset, &value2, sizeof(value2));
      offset += sizeof(value2);
      offset += packString(buffer + offset, value3);
      return offset;
    }
    unsigned int unpack(char* buffer)
    {
      unsigned int offset = 0;
      memcpy(&value1, buffer + offset, sizeof(value1));
      offset += sizeof(value1);
      memcpy(&value2, buffer + offset, sizeof(value2));
      offset += sizeof(value2);
      offset += unpackString(value3, buffer + offset);
      return offset;
    }
    unsigned int getSize()
    {
      return sizeof(value1) + sizeof(value2) + sizeof(unsigned int) + value3.length();
    }
  
    int value1;
    float value2;
    string value3;  
}; 
#include "MY_Protocol.ph"

void recv(GCFEvent& e)
{
    ABSBeamAllocEvent* ba = (ABSBeamAllocEvent*)(&e);
    ABSBeamAllocEventExt bae(*ba, true);

    bae.base.param1;
    bae.base.param2;

    string t(bae.ext2, bae.ext2Dim);

    cout << "bae.ext2 = " << t << endl;
    cout << "bae.ext3 = " << bae.ext3 << endl;
    cout << "bae.obj1->value3 = " << ((TransObject*)bae.pObj1)->value3 << endl;
}

void test()
{
    ABSBeamAllocEvent ba;

    ba.param1 = 10;
    ba.param2 = 20;

    ABSBeamAllocEventExt bae(ba);
    
    bae.ext1Dim = 100;
    bae.ext1 = new int[100];

    for (int i = 0; i < 100; i++) bae.ext1[i] = i;

    string t("test_string_ext2");
    bae.ext2Dim = t.length();
    bae.ext2      = (char*)t.c_str();
    
    bae.ext3 = "test_string_ext3";
    TransObject transObj(10, 20.0, "test_string_obj1");
    bae.pObj1 = &transObj;
    unsigned int packsize;
    void * buf = bae.pack(packsize);

    char* newbuf = new char[ba.length];
    
    memcpy(newbuf, &ba, sizeof(ba));
    memcpy(newbuf+sizeof(ba), buf, packsize);

    recv(*((GCFEvent*)newbuf));

    delete [] newbuf;
    delete [] bae.ext1;
}

int main(int, char**)
{
    test();

    return 0;
}


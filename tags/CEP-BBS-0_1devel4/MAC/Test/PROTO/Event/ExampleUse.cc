#include <string.h>
#include <iostream>

using namespace std;

#include "MY_Protocol.ph"

using namespace MY_Protocol;

void send(GCFEvent& e);
void recv(char* buf);
void current_state(GCFEvent& e);

// USER part - start
void test()
{
    ABSBeamAllocEvent ba;

    ba.param1[0] = 10; ba.param1[1] = 20;
    ba.param2 = 20;

    ba.ext1Dim = 100;
    ba.ext1 = new int[100];

    for (int i = 0; i < 100; i++) ba.ext1[i] = i;

    string t("test_string_ext2");
    ba.ext2Dim = t.length();
    ba.ext2      = (char*)t.c_str();
    
    ba.ext3 = "test_string_ext3";
    TransObject transObj(10, 20.0, "test_string_obj1");
    ba.pObj1 = &transObj;
    send(ba);

    delete [] ba.ext1;
}

void current_state(GCFEvent& e)
{
    ABSBeamAllocEvent ba(e); // automatically unpacks the received data from 'e' into 'ba'

    ba.param1;
    ba.param2;

    string t(ba.ext2, ba.ext2Dim);

    cout << "bae.ext2 = " << t << endl;
    cout << "bae.ext3 = " << ba.ext3 << endl;
    cout << "bae.obj1->value3 = " << ((TransObject*)ba.pObj1)->value3 << endl;
}
// USER part - end

// TM part - start
void send(GCFEvent& e)
{
    unsigned int packsize;
    void* buf = e.pack(packsize);

    recv((char*)buf);
}

// part of GCFRawPort::recvEvent
void recv(char* buf)
{
    GCFEvent e;
    memcpy(&e.signal, buf, sizeof(e.signal));
    memcpy(&e.length, buf + sizeof(e.signal), sizeof(e.length));
    
    char* newbuf = new char[sizeof(e) + e.length];
    memcpy(newbuf, &e, sizeof(GCFEvent));
    memcpy(newbuf + sizeof(GCFEvent), buf + sizeof(e.length) + sizeof(e.signal), e.length);
    
    current_state(*((GCFEvent*) newbuf));
    delete [] newbuf;
}
// TM part - end

int main(int, char**)
{
    test();

    return 0;
}


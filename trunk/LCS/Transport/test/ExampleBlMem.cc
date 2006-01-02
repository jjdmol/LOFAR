//# ExampleMemBL.cc: Test program for basic Transport classes
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <DH_Example.h>
#include <DH_ExampleExtra.h>
#include <Transport/Connection.h>
#include <Transport/TH_Mem.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;

#ifdef USE_THREADS

void* startWriterThread(void* thread_arg)
{
  Connection* conn = (Connection*)thread_arg; 
  DH_Example* dh   = (DH_Example*)conn->getDataHolder();
  for (int count = 1; count <= 20; count++)
  {
    dh->getBuffer()[0] = makefcomplex(count+17,-3.5);
    dh->setCounter(count);
    conn->write();
  }
  pthread_exit(NULL);
}

void* startReaderThread(void* thread_arg)
{
  Connection* conn = (Connection*)thread_arg; 
  DH_Example* dh   = (DH_Example*)conn->getDataHolder(true);
  int* result = new int;
  *result = 1;

  for (int count = 1; count <= 20; count++)
  {
    conn->read();
    if (dh->getBuffer()[0] != makefcomplex(count+15, -4.5) ||
	dh->getCounter() != count)
    {
      *result = 0;
    }
  }
  pthread_exit(result);
}

void* startVarWriterThread(void* thread_arg)
{
  Connection* conn    = (Connection*)thread_arg; 
  DH_ExampleExtra* dh = (DH_ExampleExtra*)conn->getDataHolder();

  for (int count=1; count <= 10; count++)
  {
    // fill the DataHolders.
    {
      dh->getBuffer()[0] = makefcomplex(17,-3.5*count);
      dh->setCounter(1);
      // fill extra blob
      BlobOStream& bos = dh->fillVariableBuffer();
      bos << "a string";
      // do the data transport
      conn->write();
    }
    {
      dh->getBuffer()[0] = makefcomplex(15,-4.5*count);
      dh->setCounter(2);
      // do the data transport (without data in the extra blob)
      conn->write();
    }
    {
      dh->getBuffer()[0] = makefcomplex(151,-4.5*count);
      dh->setCounter(20);
      dh->clearVariableBuffer();
      // do the data transport (without data in the extra blob)
      conn->write();
    }
    {
      dh->getBuffer()[0] = makefcomplex(1.7,3.52);
      dh->setCounter(3);
      BlobOStream& bos = dh->fillVariableBuffer();
      bos << int(1) << float(3*count);
      bos.putStart ("p3", 3);
      bos.putEnd();
      // do the data transport
      conn->write();
    }
  }
  pthread_exit(NULL);
}

void* startVarReaderThread(void* thread_arg)
{
  Connection* conn    = (Connection*)thread_arg; 
  DH_ExampleExtra* dh = (DH_ExampleExtra*)conn->getDataHolder(true);
  int* result = new int;
  *result = 1;

  for (int count=1; count <= 10; count++)
  {
    {
      conn->read();
      cout << "read a " << count << endl;
      if (dh->getBuffer()[0] != makefcomplex(17, -3.5*count) ||
	  dh->getCounter() != 1)
      {
	*result = 0;
      }
      int version;
      bool found;
      BlobIStream& bis = dh->readVariableBuffer(found,version);
      if (!found) {
	cout << "!found 1" << endl;
	*result = 0;
      } else {
	std::string str;
	bis >> str;
	bis.getEnd();
	if (str != "a string") {
	  *result = 0;
	}
      }
    }
    {
      conn->read();
      cout << "read a " << count << endl;
      if (dh->getBuffer()[0] != makefcomplex(15, -4.5*count) ||
	  dh->getCounter() != 2)
      {
	*result = 0;
      }
      int version;
      bool found;
      BlobIStream& bis = dh->readVariableBuffer(found,version);
      if (!found) {
	cout << "!found 2" << endl;
	*result = 0;
      } else {
	std::string str;
	bis >> str;
	bis.getEnd();
	if (str != "a string") {
	  *result = 0;
	}
      }
    }
    {
      conn->read();
      cout << "read a " << count << endl;
      if (dh->getBuffer()[0] != makefcomplex(151, -4.5*count) ||
	  dh->getCounter() != 2)
      {
	*result = 0;
      }
      int version;
      bool found;
      dh->readVariableBuffer(found,version);
      if (found) {
	cout << "found 3" << endl;
	*result = 0;
      }
    } 
    {
      conn->read();
      cout << "read a " << count << endl;
      if (dh->getBuffer()[0] != makefcomplex(1.7, 3.52) ||
	  dh->getCounter() != 3)
      {
	*result = 0;
      }
      int version;
      bool found;
      BlobIStream& bis = dh->readVariableBuffer(found,version);
      if (!found) {
	cout << "!found 4" << endl;
	*result = 0;
      } else {
	int v1;
	float v2;
	bis >> v1 >> v2;
	int vers = bis.getStart ("p3");
	bis.getEnd();
	bis.getEnd();
	if (v1 != 1  ||  v2 != 3*count ||  vers != 3) {
	  *result = 0;
	}
      } 
    } 
  }

  cout << "var read done" << endl;
  pthread_exit(result);
}

#endif


bool test1()
{
#ifdef USE_THREADS
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // connect DH1 to DH2 with blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection1", &DH1, &DH2, &memTH, true);
    
  // initialize
  DH1.init();
  DH2.init();
  
  // Create a writing thread
  pthread_t writer;
  if (pthread_create(&writer, NULL, startWriterThread, &conn))
  {
    cout << "Thread creation failed" << endl;
  }

  usleep(1000000); // Wait a second

  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH2.setCounter(0);

  // Read
  bool result = true;
  for (int count = 1; count <= 20; count++)
  {
    conn.read();
    if (DH2.getBuffer()[0] != makefcomplex(count+17, -3.5) ||
	DH2.getCounter() != count)
    {
      result = false;
    }
  }

  pthread_join(writer, NULL);

  cout << "test1 result: " << result << endl;
  return result;
#else
  return true;
#endif
}

bool test2()
{
#ifdef USE_THREADS
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // connect DH1 to DH2 with blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection2", &DH1, &DH2, &memTH, true);
    
  // initialize
  DH1.init();
  DH2.init();

  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH2.setCounter(0);
  
  // Create a reading thread
  pthread_t reader;
  if (pthread_create(&reader, NULL, startReaderThread, &conn))
  {
    cout << "Thread creation failed." << endl;
  }

  usleep(1000000); // Wait a second

  // Write
  for (int count = 1; count <= 20; count++)
  {
    DH1.getBuffer()[0] = makefcomplex(count+15,-4.5);
    DH1.setCounter(count);
    conn.write();
  }

  int* result;
  pthread_join(reader, (void**)&result);
  
  cout << "Test2 result: " << *result << endl;
  return (*result);
#else
  return true;
#endif
 }

bool testVar1()
{
#ifdef USE_THREADS
  DH_ExampleExtra DH1("dh1", 1);
  DH_ExampleExtra DH2("dh2", 1);
    
  // connect DH1 to DH2 with blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection3", &DH1, &DH2, &memTH, true);
    
  // initialize
  DH1.init();
  DH2.init();
  
  // Create a writing thread
  pthread_t varWriter;
  if (pthread_create(&varWriter, NULL, startVarWriterThread, &conn))
  {
    cout << "Thread creation failed" << endl;
  }

  usleep(1000000); // Wait a second

  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH2.setCounter(0);
  // Read
  bool result = true;
  for (int count=1; count <= 10; count++)
  {
    {
      conn.read();
      if (DH2.getBuffer()[0] != makefcomplex(17, -3.5*count) ||
	  DH2.getCounter() != 1)
      {
	result = false;
      }
      int version;
      bool found;
      BlobIStream& bis = DH2.readVariableBuffer(found,version);
      if (!found) {
	result = false;
      }
      std::string str;
      bis >> str;
      bis.getEnd();
      if (str != "a string") {
	result = false;
      }
    }
    {
      conn.read();
      if (DH2.getBuffer()[0] != makefcomplex(15, -4.5*count) ||
	  DH2.getCounter() != 2)
      {
	result = false;
      }
      int version;
      bool found;
      BlobIStream& bis = DH2.readVariableBuffer(found,version);
      if (!found) {
	result = false;
      }
      std::string str;
      bis >> str;
      bis.getEnd();
      if (str != "a string") {
	result = false;
      }
    }
    {
      conn.read();
      if (DH2.getBuffer()[0] != makefcomplex(151, -4.5*count) ||
	  DH2.getCounter() != 20)
      {
	result = false;
      }
      int version;
      bool found;
      DH2.readVariableBuffer(found,version);
      if (found) {
	result = false;
      }
    } 
    {
      conn.read();
      if (DH2.getBuffer()[0] != makefcomplex(1.7, 3.52) ||
	  DH2.getCounter() != 3)
      {
	result = false;
      }
      int version;
      bool found;
      BlobIStream& bis = DH2.readVariableBuffer(found,version);
      if (!found) {
	result = false;
      }
      int v1;
      float v2;
      bis >> v1 >> v2;
      int vers = bis.getStart ("p3");
      bis.getEnd();
      bis.getEnd();
      if (v1 != 1  ||  v2 != 3*count ||  vers != 3) {
	result = false;
      } 
    } 
  }
  pthread_join(varWriter, NULL);

  cout << "testVar1 result: " << result << endl;
  return result;
#else
  return true;
#endif
}

bool testVar2()
{
#ifdef USE_THREADS
  DH_ExampleExtra DH1("dh1", 1);
  DH_ExampleExtra DH2("dh2", 1);
    
  // connect DH1 to DH2 with blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection4", &DH1, &DH2, &memTH, true);
    
  // initialize
  DH1.init();
  DH2.init();

  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH2.setCounter(0);
  
  // Create a reading thread
  pthread_t varReader;
  if (pthread_create(&varReader, NULL, startVarReaderThread, &conn))
  {
    cout << "Thread creation failed." << endl;
  }

  usleep(1000000); // Wait a second

  // Write
  for (int count=1; count <= 10; count++)
  {
    // fill the DataHolders.
    {
      DH1.getBuffer()[0] = makefcomplex(17,-3.5*count);
      DH1.setCounter(1);
      // fill extra blob
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << "a string";
      // do the data transport
      conn.write();
      cout << "wrote a " << count << endl;
    }
    {
      DH1.getBuffer()[0] = makefcomplex(15,-4.5*count);
      DH1.setCounter(2);
      // do the data transport (without data in the extra blob)
      conn.write();
      cout << "wrote b " << count << endl;
    }
    {
      DH1.getBuffer()[0] = makefcomplex(151,-4.5*count);
      DH1.setCounter(2);
      DH1.clearVariableBuffer();
      // do the data transport (without data in the extra blob)
      conn.write();
      cout << "wrote c " << count << endl;
    }
    {
      DH1.getBuffer()[0] = makefcomplex(1.7,3.52);
      DH1.setCounter(3);
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << int(1) << float(3*count);
      bos.putStart ("p3", 3);
      bos.putEnd();
      // do the data transport
      conn.write();
      cout << "wrote d " << count << endl;
    }
  }

  cout << "var write done" << endl;
  int* result;
  pthread_join(varReader, (void**)&result);
  
  cout << "TestVar2 result: " << *result << endl;
  return (*result);
#else
  return true;
#endif
}



int main()
{
  bool result = true;
  try {
    INIT_LOGGER("ExampleBlMem.log_prop");
    cout << "Transport Example test program" << endl;
    cout << "test1 ..." << endl;
    result &= test1();
    cout << "test2 ..." << endl;
    result &= test2();
    cout << "testVar1 ..." << endl;
    result &= testVar1();
    cout << "testVar2 ..." << endl;
    result &= testVar2();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    result = false;
  }
  return (result ? 0:1);
}

//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C95AADB016E.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C95AADB016E.cm

//## begin module%3C95AADB016E.cp preserve=no
//## end module%3C95AADB016E.cp

//## Module: GWClientWP%3C95AADB016E; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\GWClientWP.h

#ifndef GWClientWP_h
#define GWClientWP_h 1

//## begin module%3C95AADB016E.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C95AADB016E.additionalIncludes

//## begin module%3C95AADB016E.includes preserve=yes
//## end module%3C95AADB016E.includes

// Socket
#include "Socket.h"
// GatewayWP
#include "GatewayWP.h"
// WorkProcess
#include "WorkProcess.h"
//## begin module%3C95AADB016E.declarations preserve=no
//## end module%3C95AADB016E.declarations

//## begin module%3C95AADB016E.additionalDeclarations preserve=yes
#pragma aidgroup PSCF
#pragma aid Reconnect FailConnect Reopen
//## end module%3C95AADB016E.additionalDeclarations


//## begin GWClientWP%3C95A941002E.preface preserve=yes
//## end GWClientWP%3C95A941002E.preface

//## Class: GWClientWP%3C95A941002E
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C95AA83029E;GatewayWP { -> }

class GWClientWP : public WorkProcess  //## Inherits: <unnamed>%3C95A941009C
{
  //## begin GWClientWP%3C95A941002E.initialDeclarations preserve=yes
  //## end GWClientWP%3C95A941002E.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: GWClientWP%3C95A9410081
      GWClientWP (const string &host1, const string &port1);

    //## Destructor (generated)
      ~GWClientWP();


    //## Other Operations (specified)
      //## Operation: start%3C95A941008B
      void start ();

      //## Operation: stop%3C95A9410092
      void stop ();

      //## Operation: timeout%3C95A9410093
      int timeout (const HIID &id);

      //## Operation: receive%3C95A9410095
      int receive (MessageRef& mref);

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: host%3C95A941007E
      string getHost () const;

      //## Attribute: port%3C95A941007F
      string getPort () const;

    // Additional Public Declarations
      //## begin GWClientWP%3C95A941002E.public preserve=yes
      typedef enum { STOPPED,WAITING,CONNECTING,CONNECTED } States;
      //## end GWClientWP%3C95A941002E.public
  protected:
    // Additional Protected Declarations
      //## begin GWClientWP%3C95A941002E.protected preserve=yes
      // tries to open and connect the socket
      void tryConnect ();
  
      bool reconnect_timeout_set;
       
      //## end GWClientWP%3C95A941002E.protected
  private:
    //## Constructors (generated)
      GWClientWP();

      GWClientWP(const GWClientWP &right);

    //## Assignment Operation (generated)
      GWClientWP & operator=(const GWClientWP &right);

    // Additional Private Declarations
      //## begin GWClientWP%3C95A941002E.private preserve=yes
      //## end GWClientWP%3C95A941002E.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin GWClientWP::host%3C95A941007E.attr preserve=no  public: string {U} 
      string host;
      //## end GWClientWP::host%3C95A941007E.attr

      //## begin GWClientWP::port%3C95A941007F.attr preserve=no  public: string {U} 
      string port;
      //## end GWClientWP::port%3C95A941007F.attr

    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C95A941009D
      //## Role: GWClientWP::sock%3C95A941009E
      //## begin GWClientWP::sock%3C95A941009E.role preserve=no  private: Socket { -> 0..1RHgN}
      Socket *sock;
      //## end GWClientWP::sock%3C95A941009E.role

    // Additional Implementation Declarations
      //## begin GWClientWP%3C95A941002E.implementation preserve=yes
      // when we spawn a child gateway, we watch for its bye message
      // to know when to start connecting again. This holds the
      // id of that message.
      HIID child_bye;
      //## end GWClientWP%3C95A941002E.implementation
};

//## begin GWClientWP%3C95A941002E.postscript preserve=yes
//## end GWClientWP%3C95A941002E.postscript

// Class GWClientWP 

//## Get and Set Operations for Class Attributes (inline)

inline string GWClientWP::getHost () const
{
  //## begin GWClientWP::getHost%3C95A941007E.get preserve=no
  return host;
  //## end GWClientWP::getHost%3C95A941007E.get
}

inline string GWClientWP::getPort () const
{
  //## begin GWClientWP::getPort%3C95A941007F.get preserve=no
  return port;
  //## end GWClientWP::getPort%3C95A941007F.get
}

//## begin module%3C95AADB016E.epilog preserve=yes
//## end module%3C95AADB016E.epilog


#endif

//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CB562880395.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CB562880395.cm

//## begin module%3CB562880395.cp preserve=no
//## end module%3CB562880395.cp

//## Module: GlishClientWP%3CB562880395; Package specification
//## Subsystem: OCTOPUSSY::Glish%3CB5A6190195
//## Source file: F:\lofar8\oms\LOFAR\src-links\OCTOPUSSY\Glish\GlishClientWP.h

#ifndef GlishClientWP_h
#define GlishClientWP_h 1

//## begin module%3CB562880395.additionalIncludes preserve=no
#include "DMI/Common.h"
#include "DMI/DMI.h"
//## end module%3CB562880395.additionalIncludes

//## begin module%3CB562880395.includes preserve=yes
#include "OCTOPUSSY/Glish/AID-Glish.h"
//## end module%3CB562880395.includes

// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
//## begin module%3CB562880395.declarations preserve=no
//## end module%3CB562880395.declarations

//## begin module%3CB562880395.additionalDeclarations preserve=yes
#pragma aidgroup Glish
#pragma aid GlishClientWP

class GlishSysEventSource;
class GlishRecord;
//## end module%3CB562880395.additionalDeclarations


//## begin GlishClientWP%3CB5618B0373.preface preserve=yes
//## end GlishClientWP%3CB5618B0373.preface

//## Class: GlishClientWP%3CB5618B0373
//## Category: OCTOPUSSY%3BCEC935032A
//## Subsystem: OCTOPUSSY::Glish%3CB5A6190195
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class GlishClientWP : public WorkProcess  //## Inherits: <unnamed>%3CB5619C036E
{
  //## begin GlishClientWP%3CB5618B0373.initialDeclarations preserve=yes
  //## end GlishClientWP%3CB5618B0373.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: GlishClientWP%3CB562BB0226
      GlishClientWP (GlishSysEventSource *src, bool autostp = True, AtomicID wpc = AidGlishClientWP);

    //## Destructor (generated)
      ~GlishClientWP();


    //## Other Operations (specified)
      //## Operation: start%3CBA97E70232
      virtual bool start ();

      //## Operation: stop%3CBABEA10165
      virtual void stop ();

      //## Operation: input%3CBACB920259
      virtual int input (int , int );

      //## Operation: timeout%3CBACFC6013D
      virtual int timeout (const HIID &);

      //## Operation: receive%3CB5622B01ED
      virtual int receive (MessageRef &mref);

      //## Operation: glishRecToMessage%3CB57C8401D6
      MessageRef glishRecToMessage (const GlishRecord &glrec);

      //## Operation: messageToGlishRec%3CB57CA00280
      bool messageToGlishRec (const Message &msg, GlishRecord &glrec);

    // Additional Public Declarations
      //## begin GlishClientWP%3CB5618B0373.public preserve=yes
      // max number of glish events processed per one polling loop
      static const int MaxEventsPerPoll = 10;
      
      static void recToGlish (const DataRecord &rec, GlishRecord& glrec);
      static void objectToBlockRec (const BlockableObject &obj,GlishRecord &rec );
      
      //## end GlishClientWP%3CB5618B0373.public
  protected:
    // Additional Protected Declarations
      //## begin GlishClientWP%3CB5618B0373.protected preserve=yes
      void glishToRec (const GlishRecord &glrec, DataRecord& rec);
      BlockableObject * blockRecToObject (const GlishRecord &rec );
      //## end GlishClientWP%3CB5618B0373.protected
  private:
    //## Constructors (generated)
      GlishClientWP();

      GlishClientWP(const GlishClientWP &right);

    //## Assignment Operation (generated)
      GlishClientWP & operator=(const GlishClientWP &right);

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: autostop%3CBAE1740040
      bool autostop () const;

    // Additional Private Declarations
      //## begin GlishClientWP%3CB5618B0373.private preserve=yes
      // shuts down the link
      void shutdown ();
      //## end GlishClientWP%3CB5618B0373.private
  private: //## implementation
    // Data Members for Class Attributes

      //## Attribute: evsrc%3CB561E2013E
      //## begin GlishClientWP::evsrc%3CB561E2013E.attr preserve=no  private: GlishSysEventSource * {U} 
      GlishSysEventSource *evsrc;
      //## end GlishClientWP::evsrc%3CB561E2013E.attr

      //## begin GlishClientWP::autostop%3CBAE1740040.attr preserve=no  private: bool {U} 
      bool autostop_;
      //## end GlishClientWP::autostop%3CBAE1740040.attr

    // Additional Implementation Declarations
      //## begin GlishClientWP%3CB5618B0373.implementation preserve=yes
      // flag: have unprocessed events in the stream
      bool connected,has_events;
      // tick of oldest unprocessed event
      ulong evtick;
      //## end GlishClientWP%3CB5618B0373.implementation
};

//## begin GlishClientWP%3CB5618B0373.postscript preserve=yes
//## end GlishClientWP%3CB5618B0373.postscript

// Class GlishClientWP 

//## Get and Set Operations for Class Attributes (inline)

inline bool GlishClientWP::autostop () const
{
  //## begin GlishClientWP::autostop%3CBAE1740040.get preserve=no
  return autostop_;
  //## end GlishClientWP::autostop%3CBAE1740040.get
}

//## begin module%3CB562880395.epilog preserve=yes
GlishClientWP * makeGlishClientWP (int argv,const char *argv[] );
//## end module%3CB562880395.epilog


#endif

//#  GCF_Answer.h:  Callback wrapper class for GCFMyPropertySet, GCFMyProperty, 
//#                 GCFProperty, GCFApc
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef GCF_ANSWER_H
#define GCF_ANSWER_H

#include <GCF/GCF_TMProtocols.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_Event.h>

/**
 * This is an abstract class, which provides the possibility to handle 
 * asynchronous PML actions (like load APC or get property) and value changed 
 * indications (after subscription).
 * 
 * A specialisation of this class can be given to the following classes to 
 * handle their responses or the value changed indications in a generic way:
 * GCFMyPropertySet, GCFMyProperty, GCFProperty, GCFApc
 * 
 * All responses or indications (events) will be given in the handleAnswer 
 * method based on the GCFEvent structure. This provides the flexibility to 
 * handle events in a state machine of a task, to skip or to handle in a 
 * different way.
*/

class GCFPValue;

class GCFAnswer
{
  public:
    GCFAnswer () {;}
    virtual ~GCFAnswer () {;}
    
    /** This is the abstract callback method, which will be invoked
      * by one of the following classes (if an instance of this class is set to 
      * one of their objects):
      * GCFMyPropertySet, GCFMyProperty, GCFProperty, GCFApc
      * There are many ways to fill in this handling method, but one of the most 
      * likely ways is to forward this answer message to a task context. See the 
      * following example (see for more examples in the GCF Test suite code).
      * @code
      *   class Answer : public GCFAnswer
      *   {
      *     public:
      *       Answer(GCFTask& t) : _t(t) {;}
      *       ~Answer() {;}
      * 
      *       void handleAnswer(GCFEvent& answer);
      *       {
      *         // The port can be a dummy port @see GCFFsm
      *         // or a port of the task _t
      *         _t.dispatch(answer, aPort); 
      *       }
      *   
      *     private:    
      *       GCFTask& _t;
      *   };
      *   // If aState is the current state this method will be called indirect
      *   // by the dispatch method call.  
      *   GCFEvent::TResult Application::aState(GCFEvent& e, GCFPortInterface& p)
      *   {
      *     ...
      *     switch (e.signal)
      *     {
      *       case F_ENTRY_SIG:
      *         break;
      *   
      *       case F_MYPLOADED_SIG:
      *       {
      *         // The event always sould be casted by the static_cast operator
      *         GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      *         assert(pResponse);
      *         if ((strcmp(pResponse->pScope, propertySetB1.scope) == 0) &&
      *             (pResponse->result == GCF_NO_ERROR))
      *         {
      *           ...
      *         }
      *         break;
      *       }
      *     }
      *   }
      * @endcode
      * @param answer could be one of the special event structs, which 
      *               transports the respones/indication data. Note that almost 
      *               all parameters of these events are pointers to data, which 
      *               is created in and managed by PML
      */
    virtual void handleAnswer (GCFEvent& answer) = 0;
    
  private:
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFAnswer (const GCFAnswer&);
    GCFAnswer& operator= (const GCFAnswer&);  
    //@}
};

/// NOTE: These structs cannot be used to send messages by real port 
/// implementations like TCP. 
struct GCFPropValueEvent : public GCFEvent
{
  /// @param sig can only be F_VCHANGEMSG_SIG, F_VGETRESP_SIG
  GCFPropValueEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFPropValueEvent);
  }
  const GCFPValue* pValue; ///< Pointer to the changed value object
  const char* pPropName;   ///< Pointer to the string of the property name
  bool internal;           ///< Indicates whether the internal/owned/my 
                           ///< (GCFMyProperty) or an other (GCFProperty) 
                           ///< property has changed (not used with F_VGETRESP_SIG)
};

struct GCFPropAnswerEvent : public GCFEvent
{
  /// @param sig can only be F_SUBSCRIBED_SIG
  GCFPropAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFPropAnswerEvent);
  }
  const char* pPropName;
};

struct GCFAPCAnswerEvent : public GCFEvent
{
  /// @param sig can only be F_APCLOADED_SIG, F_APCUNLOADED_SIG, F_APCRELOADED_SIG
  GCFAPCAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFAPCAnswerEvent);
  }
  const char* pApcName; ///< Pointer to the name string of the APC (excl. path and extension)
  const char* pScope;   ///< Scope of the apc
  TGCFResult result;    ///< Result of the requested action:
                        ///<    GCF_APCLOAD_ERROR, GCF_APCUNLOAD_ERROR, GCF_APCRELOAD_ERROR
};

struct GCFMYPropAnswerEvent : public GCFEvent
{
  /// @param sig can only be F_MYPLOADED_SIG, F_MYPUNLOADED_SIG
  GCFMYPropAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFMYPropAnswerEvent);
  }
  const char* pScope;   ///< Scope of the propertyset
  TGCFResult result;    ///< Result of the requested action: 
                        ///<    GCF_MYPROPSLOAD_ERROR, GCF_MYPROPSUNLOAD_ERROR
};

enum {
  F_PML_PROTOCOL = F_GCF_PROTOCOL + 2,
};
/**
 * F_PML_PROTOCOL signals
 */
enum {
  F_SUBSCRIBED_ID = 1,                                 
  F_UNSUBSCRIBED_ID,      
  F_VCHANGEMSG_ID,               
  F_VGETRESP_ID,        
  F_APCLOADED_ID,
  F_APCUNLOADED_ID,
  F_APCRELOADED_ID,
  F_MYPLOADED_ID,
  F_MYPUNLOADED_ID,
};

#define F_SUBSCRIBED_SIG    F_SIGNAL(F_PML_PROTOCOL, F_SUBSCRIBED_ID,    F_IN)
#define F_UNSUBSCRIBED_SIG  F_SIGNAL(F_PML_PROTOCOL, F_UNSUBSCRIBED_ID,  F_IN)
#define F_VCHANGEMSG_SIG    F_SIGNAL(F_PML_PROTOCOL, F_VCHANGEMSG_ID,    F_IN)
#define F_VGETRESP_SIG      F_SIGNAL(F_PML_PROTOCOL, F_VGETRESP_ID,      F_IN)
#define F_APCLOADED_SIG     F_SIGNAL(F_PML_PROTOCOL, F_APCLOADED_ID,     F_IN)
#define F_APCUNLOADED_SIG   F_SIGNAL(F_PML_PROTOCOL, F_APCUNLOADED_ID,   F_IN)
#define F_APCRELOADED_SIG   F_SIGNAL(F_PML_PROTOCOL, F_APCRELOADED_ID,   F_IN)
#define F_MYPLOADED_SIG     F_SIGNAL(F_PML_PROTOCOL, F_MYPLOADED_ID,     F_IN)
#define F_MYPUNLOADED_SIG   F_SIGNAL(F_PML_PROTOCOL, F_MYPUNLOADED_ID,   F_IN)

extern const char* F_PML_PROTOCOL_signalnames[];

#endif

//#  GCF_RTAnswer.h:  Callback wrapper class for GCFMyPropertySet, GCFRTMyProperty, 
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

#ifndef GCF_RTANSWER_H
#define GCF_RTANSWER_H

#include <GCF/GCF_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Event.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace RTCPMLlight 
  {
/**
 * This is an abstract class, which provides the possibility to handle 
 * asynchronous PMLlite actions (like load property set) and value changed 
 * indications (after subscription).
 * 
 * A specialisation of this class can be given to the following classes to 
 * handle their responses or the value changed indications in a generic way:
 * GCFRTMyPropertySet, GCFRTMyProperty
 * 
 * All responses or indications (events) will be given in the handleAnswer 
 * method based on the TM::GCFEvent structure. This provides the flexibility to 
 * handle events in a state machine of a task, to skip or to handle in a 
 * different way.
*/

class GCFRTAnswer
{
  public:
    GCFRTAnswer () {;}
    virtual ~GCFRTAnswer () {;}
    
    /** This is the abstract callback method, which will be invoked
      * by one of the following classes (if an instance of this class is set to 
      * one of their objects):
      * GCFRTMyPropertySet, GCFRTMyProperty
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
      *       void handleAnswer(TM::GCFEvent& answer);
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
      *   TM::GCFEvent::TResult Application::aState(TM::GCFEvent& e, GCFPortInterface& p)
      *   {
      *     ...
      *     switch (e.signal)
      *     {
      *       case F_ENTRY:
      *         break;
      *   
      *       case F_MYPLOADED:
      *       {
      *         // The event always sould be casted by the static_cast operator
      *         GCFMYPropAnswerEvent* pResponse = static_cast<GCFMYPropAnswerEvent*>(&e);
      *         ASSERT(pResponse);
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
      *               is created in and managed by PMLlite
      */
    virtual void handleAnswer (TM::GCFEvent& answer) = 0;
    
  private:
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFRTAnswer (const GCFRTAnswer&);
    GCFRTAnswer& operator= (const GCFRTAnswer&);  
    //@}
};

enum 
{
  F_PML_PROTOCOL = TM::F_GCF_PROTOCOL,
};
/**
 * F_PML_PROTOCOL signals
 */
enum 
{
  F_VCHANGEMSG_ID = 1,
  F_MYPS_ENABLED_ID,
  F_MYPS_DISABLED_ID,
};

#define F_VCHANGEMSG    F_SIGNAL(F_PML_PROTOCOL, F_VCHANGEMSG_ID,    F_IN)
#define F_MYPS_ENABLED  F_SIGNAL(F_PML_PROTOCOL, F_MYPS_ENABLED_ID,  F_IN)
#define F_MYPS_DISABLED F_SIGNAL(F_PML_PROTOCOL, F_MYPS_DISABLED_ID, F_IN)

/// NOTE: These structs cannot be used to send messages by real port 
/// implementations like TCP. 
struct GCFPropValueEvent : public TM::GCFEvent
{
  /// @param sig can only be F_VCHANGEMSG
  GCFPropValueEvent(unsigned short sig) : TM::GCFEvent(sig)
  {
      length = sizeof(GCFPropValueEvent);
  }
  const Common::GCFPValue* pValue; ///< Pointer to the changed value object
  const char* pPropName;   ///< Pointer to the string of the property name
  bool internal;           ///< Indicates whether the internal/owned/my 
                           ///< (GCFRTMyProperty)
                           ///< property has changed (not used with F_VGETRESP)
};

struct GCFPropSetAnswerEvent : public TM::GCFEvent
{
  /// @param sig can only be F_MYPS_ENABLED, F_MYPS_DISABLED
  GCFPropSetAnswerEvent(unsigned short sig) : TM::GCFEvent(sig)
  {
      length = sizeof(GCFPropSetAnswerEvent);
  }
  const char* pScope;   ///< Scope of the propertyset
  Common::TGCFResult result;    ///< Result of the requested action: 
                        ///<    GCF_MYPS_ENABLE_ERROR, GCF_MYPS_DISABLE_ERROR, 
};

extern const char* F_PML_PROTOCOL_signalnames[];
  } // namespace RTCPMLlight
 } // namespace GCF
} // namespace LOFAR

#endif

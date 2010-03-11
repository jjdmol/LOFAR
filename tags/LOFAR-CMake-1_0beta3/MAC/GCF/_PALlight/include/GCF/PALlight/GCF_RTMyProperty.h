//#  GCF_RTMyProperty.h: 
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

#ifndef GCF_RTMYPROPERTY_H
#define GCF_RTMYPROPERTY_H

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_Defines.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFEvent;
  }
  namespace RTCPMLlight 
  {

class GCFRTMyPropertySet;
class GCFRTAnswer;
/**
 * 
 */

class GCFRTMyProperty
{
  public:
    inline const string& getName () const 
      { return _name;}
      
    /// @return the given property name including the scope of the related property set
    virtual const string getFullName () const;

    inline virtual void setAnswer (GCFRTAnswer* pAnswerObj) 
      {_pAnswerObj = pAnswerObj;}

    Common::GCFPValue* getValue () const;
    Common::GCFPValue* getOldValue () const;
   
    //@{
    /** They both first copies the current local (owned) value
     * to the old value followed by setting the changing the current
     * with the parameter 'value'. In case the property is linked
     * and the property is readable then the value in SCADA DB will be 
     * updated too.
     * @return can be GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
     */
    Common::TGCFResult setValue (const string& value);
    Common::TGCFResult setValue (const Common::GCFPValue& value);        
    //@}
    inline bool isLinked () const {return _isLinked && !_isBusy;}
    //@{
    /** Access mode can be: GCF_READABLE_PROP and/or GCF_WRITABLE_PROP
     * NOTE: If both modes are set and the property is linked, the setValue 
     * method call results immediate in a F_VCHANGEMSG answer event. This 
     * on its turn copies the current to the old value and the changed value to
     * current value. This means that the current value from before the setValue 
     * will not be available after the answer event is received (old value is 
     * then the same as the new value). On the other hand this construction can 
     * been seen as a asynchronous 'set' action.
     */ 
    void setAccessMode (Common::TAccessMode mode, bool on);
    bool testAccessMode (Common::TAccessMode mode) const;   
    //@}
    
  private:
    friend class GCFRTMyPropertySet;
    
    GCFRTMyProperty (const Common::TPropertyInfo& propertyFields, 
                     GCFRTMyPropertySet& propertySet);
    GCFRTMyProperty(GCFRTMyPropertySet& propertySet);
    virtual ~GCFRTMyProperty ();
    
    void link ();
    void unlink ();    
    
    void valueChanged (const Common::GCFPValue& value);           

  private: // helper methods
    virtual void dispatchAnswer(TM::GCFEvent& answer);  
    
  private: 
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFRTMyProperty (const GCFRTMyProperty&);
    GCFRTMyProperty& operator= (const GCFRTMyProperty&);  
    //@}

  private:
    string            _name;
    GCFRTMyPropertySet& _propertySet;
    Common::TAccessMode       _accessMode;
    Common::GCFPValue*        _pCurValue;
    Common::GCFPValue*        _pOldValue;
    bool              _isLinked;
    bool              _isBusy;
    GCFRTAnswer*      _pAnswerObj;
};
  } // namespace RTCPMLlight
 } // namespace GCF
} // namespace LOFAR
#endif

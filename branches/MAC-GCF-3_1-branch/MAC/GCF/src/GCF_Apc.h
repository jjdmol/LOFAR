//#  GCF_Apc.h: manages the lifecycle of an APC for an Application 
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

#ifndef GCF_APC_H
#define GCF_APC_H

#include <GCF/GCF_Defines.h>

/**
 * This class represents an APC. It gives the Application the possibility to 
 * control a certain APC, which then can be (un/re)loaded. All responses of the 
 * possible actions can be handled by using a specialisation of the GCFAnswer 
 * class.
*/

class GCFAnswer;
class GPMController;

class GCFApc
{
  public:
    /** Constructor
     * @param name name of an APC (excl. path and extension); location of the 
     *             APC's is dependend on the setting of the Property Agent 
     *             (default is $MAC_CONFIG)
     * @param scope scope for this APC
     * @param pAnswerObj object on which the answer of the (re/un)load request
     *                   should be send (if 0 no answer will be send to the 
     *                   Application)
     */
    
    GCFApc (string name, 
            string scope, 
            GCFAnswer* pAnswerObj = 0) :
            _isLoaded(false),
            _isBusy(false),
            _pAnswerObj(pAnswerObj),
            _name(name),
            _scope(scope) 
            {;}

    /** Constructor
     * This contructor sets the name and scope default to an empty string
     * @param pAnswerObj object on which the answer of the (re/un)load request
     *                   should be send (if 0 no answer will be send to the 
     *                   Application)
     */
    GCFApc (GCFAnswer* pAnswerObj = 0) :
            _isLoaded(false),
            _isBusy(false),
            _pAnswerObj(pAnswerObj),
            _name(""),
            _scope("") 
            {;}
    virtual ~GCFApc ();

    inline void setAnswer (GCFAnswer* pAnswerObj) {_pAnswerObj = pAnswerObj;}
    inline const string& getName () const {return _name;}
    inline const string& getScope () const {return _scope;}
    TGCFResult setName (const string name);
    TGCFResult setScope (const string scope);
    TGCFResult setApcData (const string name, 
                           const string scope);
    /**
     * Loads the apc
     * @param loadDefaults specifies whether the defaults in a APC file should 
     *                     be loaded and set in de SCADA DB or not
     *                     NOTE: A default in the APC overwrites the default of 
     *                     an owned property set on a link request
     * @return GCF_BUSY, GCF_ALREADY_LOADED or GCF_NO_PROPER_DATA
     */
    TGCFResult load (bool loadDefaults = true);
    /**
     * Reloads the apc
     * @return GCF_BUSY, GCF_NOT_LOADED or GCF_NO_PROPER_DATA
     */
    TGCFResult reload ();
    /**
     * Unloads the apc
     * @return GCF_BUSY, GCF_NOT_LOADED or GCF_NO_PROPER_DATA
     */
    TGCFResult unload ();
    
    inline bool isLoaded () const 
      {return _isLoaded;}
    inline bool defaultsLoaded () const 
      {return (_isLoaded && _loadDefaults);}
                  
  private:
    friend class GPMController;
    void loaded (TGCFResult result);
    void reloaded (TGCFResult result);
    void unloaded (TGCFResult result);
    void dispatchAnswer (unsigned short sig, TGCFResult result);
    inline bool mustLoadDefaults () const 
      { return _loadDefaults; }    
    
  private:
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFApc (const GCFApc&);
    GCFApc& operator= (const GCFApc&);  
    //@}

  private:
    bool        _isLoaded;
    bool        _isBusy;
    bool        _loadDefaults;
    GCFAnswer*  _pAnswerObj;
    string      _name;
    string      _scope;
};
#endif

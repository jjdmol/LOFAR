/*
 * ConfigPanelHelper.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otb.util;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;

/**
 * This (singleton) class maintains a list of available panels that can be shown on a given name in the TemplateMaintenanceBrowser.
 * In a later stage this should be obtained from the database to make it more solid and dynamic.
 * The panels here are mainly for adding configuration information
 *
 * @created 11-07-2006
 *
 * @author coolen
 *
 * @version $Id$
 *
 */
public class ConfigPanelHelper {
   
    private HashMap<String,Vector<String> > itsPanelMap;
    private Vector<String> itsVector;
    private static ConfigPanelHelper ref;
    
    /** Creates a new instance of ConfigPanelHelper */
    private ConfigPanelHelper() {
        initMap();
    }
    
    public static synchronized ConfigPanelHelper getConfigPanelHelper() {
        if (ref== null) {
            ref = new ConfigPanelHelper();
        }
        return ref;
    }
    
    @Override
    public Object clone() throws CloneNotSupportedException {
        throw new CloneNotSupportedException();
    }
    
    private void initMap() {
        itsPanelMap = new HashMap<String, Vector<String> >();
        
        //generic panels
        itsVector = new Vector<String>();
        addBasePanels();
        itsPanelMap.put("*",itsVector);
        
        //BBS
//        itsVector = new Vector<String>();
//        itsVector.add("nl.astron.lofar.sas.otbcomponents.bbs.BBSPanel");
//        itsPanelMap.put("BBSControl",itsVector);
        
        //BBS Strategy
//        itsVector = new Vector<String>();
//        itsVector.add("nl.astron.lofar.sas.otbcomponents.bbs.BBSStrategyPanel");
//        itsPanelMap.put("Strategy",itsVector);
  

        //OLAP
        itsVector = new Vector<String>();
        itsVector.add("nl.astron.lofar.sas.otbcomponents.userpanels.OlapPanel");
        itsPanelMap.put("OLAP",itsVector);
        
        // Observation
        itsVector = new Vector<String>();
        itsVector.add("nl.astron.lofar.sas.otbcomponents.userpanels.ObservationPanel");
        itsPanelMap.put("Observation",itsVector);
        
        //TBB
        itsVector = new Vector<String>();
        itsVector.add("nl.astron.lofar.sas.otbcomponents.userpanels.TBBConfigPanel");
        itsPanelMap.put("TBB",itsVector);
        
        //Imager
//        itsVector = new Vector<String>();
//        itsVector.add("nl.astron.lofar.sas.otbcomponents.userpanels.ImagerPanel");
//        itsPanelMap.put("Imager",itsVector);
    }
    
    
    private void addBasePanels() {
        //generic node panel
        itsVector.add("nl.astron.lofar.sas.otbcomponents.NodeViewPanel");
        //generic parameter panel
        itsVector.add("nl.astron.lofar.sas.otbcomponents.ParameterViewPanel");
    }
    
    /**
     * Returns the possible panels for this Key
     *
     * @param aKey  the key that you want the panellist for
     *              if aKey = "*" it returns all default panels
     *              if aKey = ""  all NON default panels are returned
     * @return the Vector that contains all panels for this key
     */
    public Vector getPanels(String aKey) {
        Vector returnVector = null;
        Iterator i = itsPanelMap.keySet().iterator();
        while(i.hasNext()){
            String key = (String)i.next();
            if(aKey.equals(key) || (aKey.equals("") && !key.equals("*"))){
                returnVector = itsPanelMap.get(key);
            }
        }
        return returnVector;
    }
    
    /**
     * Returns all keys kept in this class
     *
     * @returns all keys
     */
    public Set<String> getKeys() {

        return itsPanelMap.keySet();

    }

    /**
     * Look if a key available at all.
     *
     *@param aKey The key that you want to test
     *@return true if key was available, false if not
     */
    public boolean isKey(String aKey) {
        boolean returnBool = false;
        Iterator i = itsPanelMap.keySet().iterator();
        while(i.hasNext()){
            String key = (String)i.next();
            if(aKey.equals(key)){
                returnBool = true;
            }
        }
        return returnBool;
    }
    
    
}

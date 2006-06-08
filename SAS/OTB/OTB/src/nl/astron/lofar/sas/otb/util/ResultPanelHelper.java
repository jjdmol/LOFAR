/*
 * ResultPanelHelper.java
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
import java.util.Vector;

/**
 * This (singleton) class maintains a list of available panels that can be shown on a given (VIC)name in the ResultBrowser.
 * In a later stage this should be obtained from the database to make it more solid and dynamic.
 *
 * @created 16-05-2006
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class ResultPanelHelper {
   
    private HashMap<String,Vector<String> > itsPanelMap;
    private Vector<String> itsVector;
    private static ResultPanelHelper ref;
    
    /** Creates a new instance of ResultPanelHelper */
    private ResultPanelHelper() {
        initMap();
    }
    
    public static synchronized ResultPanelHelper getResultPanelHelper() {
        if (ref== null) {
            ref = new ResultPanelHelper();
        }
        return ref;
    }
    
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
        itsVector = new Vector<String>();
        //addBasePanels();
        itsVector.add("nl.astron.lofar.sas.otbcomponents.ParmDBPlotPanel");
        itsPanelMap.put("ParmDB",itsVector);
        
        
    }
    
    
    private void addBasePanels() {
        //generic logging panel
        itsVector.add("nl.astron.lofar.sas.otbcomponents.LogParamPanel");
        //generic node panel
        itsVector.add("nl.astron.lofar.sas.otbcomponents.NodeViewPanel");
        //generic parameter panel
        itsVector.add("nl.astron.lofar.sas.otbcomponents.ParameterViewPanel");
    }
    
    /**
     * Returns the possible panels for this Key
     *
     * @param aKey  the key that you want the panellist for
     * @return the Vector that contains all panels for this key
     */
    public Vector getPanels(String aKey) {
        Vector returnVector = null;
        Iterator i = itsPanelMap.keySet().iterator();
        while(i.hasNext()){
            String key = (String)i.next();
            if(aKey.indexOf(key) == 0){
                returnVector = itsPanelMap.get(key);
            }
        }
        return returnVector;
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
            if(aKey.indexOf(key) == 0){
                returnBool = true;
            }
        }
        return returnBool;
    }
    
    
}

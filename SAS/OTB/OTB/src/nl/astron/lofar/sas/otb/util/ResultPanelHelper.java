/*
 * ResultPanelHelper.java
 *
 * Created on 16 mei 2006, 11:09
 *
 * This class maintains a list of available panels that can be shown on a given (VIC)name in the ResultBrowser.
 * In a later stage this should be obtained from the database to make it more solid and dynamic.
 * 
 * 
 */

package nl.astron.lofar.sas.otb.util;

import java.util.HashMap;
import java.util.Vector;

/**
 *
 * @author coolen
 */
public class ResultPanelHelper {
   
    private HashMap<String,Vector<String> > itsPanelMap;
    private Vector<String> itsVector;
    
    /** Creates a new instance of ResultPanelHelper */
    public ResultPanelHelper() {
        initMap();
    }
    
    private void initMap() {
        itsPanelMap = new HashMap<String, Vector<String> >();
        
        //generic panels
        itsVector = new Vector<String>();
        addBasePanels();
        itsPanelMap.put("*",itsVector);

        
        //BBS
        itsVector = new Vector<String>();
        addBasePanels();
        itsVector.add("nl.astron.lofar.sas.otbcomponents.parmDBPlotPanel");
        itsPanelMap.put("BBS",itsVector);
        
        
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
        return itsPanelMap.get(aKey);
    }
    
    /**
     * Look if a key available at all.
     *
     *@param aKey The key that you want to test
     *@return true if key was available, false if not
     */
    public boolean isKey(String aKey) {
        return itsPanelMap.containsKey(aKey);
    }
    
    
}

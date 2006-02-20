/*
 * jConverter.java
 *
 * Created on February 17, 2006, 2:13 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.jotdb2;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

/**
 *
 * @author coolen
 */
public class jConverter {
   private jClassifConv    itsCC;
   private jParamTypeConv  itsPTC;
   private jTreeStateConv  itsTSC;
   private jTreeTypeConv   itsTTC;
   private jUnitConv       itsUC;
        
    /** Creates a new instance of jConverter */
    public jConverter() {
        initConverter() ;
    }
     
    private void initConverter() {
        // instantiate all possible converters
        itsCC  = new jClassifConv();
        itsPTC = new jParamTypeConv();
        itsTSC = new jTreeStateConv();
        itsTTC = new jTreeTypeConv();
        itsUC  = new jUnitConv();
    }
    
    public long getClassif(String aConv) { 
        return itsCC.get(aConv);
    }
    
    public String getClassif(long aConv) {
        return itsCC.get(aConv);
    }
    
    public HashMap getClassif() {
        return itsCC.getTypes();
    }

    public long getParamType(String aConv) { 
        return itsPTC.get(aConv);
    }
    
    public String getParamType(long aConv) {
        return itsPTC.get(aConv);
    }
    
    public HashMap getParamType() {
        return itsPTC.getTypes();
    }
    public long getTreeState(String aConv) { 
        return itsTSC.get(aConv);
    }
    
    public String getTreeState(long aConv) {
        return itsTSC.get(aConv);
    }
    
    public HashMap getTreeState() {
        return itsTSC.getTypes();
    }
    public long getTreeType(String aConv) { 
        return itsTTC.get(aConv);
    }
    
    public String getTreeType(long aConv) {
        return itsTTC.get(aConv);
    }
    
    public HashMap getTreeType() {
        return itsTTC.getTypes();
    }
    public long getUnit(String aConv) { 
        return itsUC.get(aConv);
    }
    
    public String getUnit(long aConv) {
        return itsUC.get(aConv);
    }
    
    public HashMap getUnit() {
        return itsUC.getTypes();
    }

}

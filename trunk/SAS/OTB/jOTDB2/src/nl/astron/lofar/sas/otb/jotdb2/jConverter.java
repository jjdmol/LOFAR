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
    
    public short getClassif(String aConv) { 
        return itsCC.get(aConv);
    }
    
    public String getClassif(short aConv) {
        return itsCC.get(aConv);
    }
    
    public HashMap<Short,String> getClassif() {
        return itsCC.getTypes();
    }

    public short getParamType(String aConv) { 
        return itsPTC.get(aConv);
    }
    
    public String getParamType(short aConv) {
        return itsPTC.get(aConv);
    }
    
    public HashMap<Short,String> getParamType() {
        return itsPTC.getTypes();
    }
    public short getTreeState(String aConv) { 
        return itsTSC.get(aConv);
    }
    
    public String getTreeState(short aConv) {
        return itsTSC.get(aConv);
    }
    
    public HashMap<Short,String> getTreeState() {
        return itsTSC.getTypes();
    }
    public short getTreeType(String aConv) { 
        return itsTTC.get(aConv);
    }
    
    public String getTreeType(short aConv) {
        return itsTTC.get(aConv);
    }
    
    public HashMap<Short,String> getTreeType() {
        return itsTTC.getTypes();
    }
    public short getUnit(String aConv) { 
        return itsUC.get(aConv);
    }
    
    public String getUnit(short aConv) {
        return itsUC.get(aConv);
    }
    
    public HashMap<Short,String> getUnit() {
        return itsUC.getTypes();
    }

}

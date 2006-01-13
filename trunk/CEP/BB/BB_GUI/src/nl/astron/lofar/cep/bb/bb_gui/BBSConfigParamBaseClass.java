/*
 * BBSConfigParamBaseClass.java
 *
 * Created on 20 december 2005, 15:07
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.cep.bb.bb_gui;

/**
 *
 * @author coolen
 */
public class BBSConfigParamBaseClass {
    private String itsPrefix;
    private String itsBBSName;
    private Object itsValue;
    private Object itsOldValue;
    private Object itsDefaultValue;
    
    /** Creates a new instance of BBSConfigParamBaseClass */
    public BBSConfigParamBaseClass(String aP,String aN, Object aDV) {
        itsPrefix=aP;
        itsBBSName=aN;
        itsValue=aDV;
        itsOldValue=aDV;
        itsDefaultValue=aDV;
    }
    
    public  String getPrefix() {
        return itsPrefix;
    }
    
    public String getBBSName(){
        return itsBBSName;
    }
    
    public Object getValue(){
        return itsValue;
    }
    
    public Object getOldValue() {
        return itsOldValue;
    }
    
    public Object getDefaultValue() {
        return itsDefaultValue;
    }
    
    public void setValue(Object aV) {
        setOldValue(itsValue);
        itsValue=aV;
    }
    
    private void setOldValue(Object anOV) {
        itsOldValue=anOV;
    }
    
}

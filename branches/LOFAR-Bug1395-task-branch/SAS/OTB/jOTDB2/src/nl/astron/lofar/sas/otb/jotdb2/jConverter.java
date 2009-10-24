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

import java.rmi.RemoteException;
import java.util.HashMap;

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
    
    public short getClassif(String aConv) throws RemoteException{
        short aS;
        try {
            aS=itsCC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getClassif(String) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;        
    }
    
    public String getClassif(short aConv) throws RemoteException {
        String aS=null;
        try {
            aS=itsCC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getClassif(Short) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public HashMap<Short,String> getClassif() throws RemoteException {
        HashMap<Short,String> aM=null;
        try {
            aM = itsCC.getTypes();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getClassif() error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aM;     
    }

    public short getParamType(String aConv) throws RemoteException { 
        short aS;
        try {
            aS = itsPTC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getParamType(String) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public String getParamType(short aConv) throws RemoteException {
        String aS=null;
        try {
            aS = itsPTC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getParamType(Short) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;
    }
    
    public HashMap<Short,String> getParamType() throws RemoteException {
        HashMap<Short,String> aM=null;
        try {
            aM = itsPTC.getTypes();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getClassif() error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aM;            
    }
    public short getTreeState(String aConv) throws RemoteException { 
        short aS;
        try {
            aS = itsTSC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeState(String) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public String getTreeState(short aConv) throws RemoteException {
        String aS=null;
        try {
            aS = itsTSC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeState(short) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public HashMap<Short,String> getTreeState() throws RemoteException {
        HashMap<Short,String> aM=null;
        try {
            aM = itsTSC.getTypes();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeState() error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aM;            
    }
    public short getTreeType(String aConv) throws RemoteException { 
        short aS;
        try {
            aS = itsTTC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeType(String) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public String getTreeType(short aConv) throws RemoteException {
        String aS=null;
        try {
            aS = itsTTC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeType(short) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public HashMap<Short,String> getTreeType() throws RemoteException {
        HashMap<Short,String> aM=null;
        try {
            aM =itsTTC.getTypes();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getTreeType() error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aM;            
    }
    public short getUnit(String aConv) throws RemoteException { 
        short aS;
        try {
            aS = itsUC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getUnit(String) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public String getUnit(short aConv) throws RemoteException {
        String aS=null;
        try {
            aS =itsUC.get(aConv);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getUnit(short) error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aS;            
    }
    
    public HashMap<Short,String> getUnit() throws RemoteException {
        HashMap<Short,String> aM=null;
        try {
            aM =itsUC.getTypes();
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getUnit() error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aM;            
    }

}

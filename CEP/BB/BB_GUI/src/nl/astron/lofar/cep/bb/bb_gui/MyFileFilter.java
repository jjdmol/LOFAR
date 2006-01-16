/*
 * MyFileFilter.java
 *
 * Created on 4 januari 2006, 13:44
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.cep.bb.bb_gui;

import java.io.File;
import javax.swing.filechooser.*;

/**
 *
 * @author Coolen
 */
public class MyFileFilter extends FileFilter {
    
    private String itsFilterChoice;
    private String itsFilterDescription;
    
    /** Creates a new instance of MyFileChooser */
    public MyFileFilter(String aFilterChoice) {
        itsFilterChoice=aFilterChoice;
        if (aFilterChoice.equals("LoadConfig") ||
                aFilterChoice.equals("NewConfig")  ||
                aFilterChoice.equals("ConfigSaveAs")) {
            itsFilterDescription="BB Configuration File (*.cfg)";
        } else if (aFilterChoice.equals("GetFlowEntry")) { 
            itsFilterDescription="BB Flow Configuration File (*.flow_cfg)";            
        } else if (aFilterChoice.equals("MSName")) {
            itsFilterDescription="Measurement Sets (*.MS)";
        } else  if (aFilterChoice.equals("SkyTableName")) {
            itsFilterDescription="LSM Table (*_gsm.MEP)";
        } else if (aFilterChoice.equals("MeqTableName")) {
            itsFilterDescription="Common Parameter Table (*.MEP)";
        } else {
            itsFilterDescription=aFilterChoice;
        }
    }
    
    public boolean accept(File aF) {
        
       if (aF == null) {
           return false;
       }
       
       if (aF.isDirectory()) {
           return true;
       } 

       String extension = getExtension(aF);
       if (itsFilterChoice.equals("MSName")) {
           if (extension != null && extension.equals("ms")) {
               return true;
           } else {
               return false;
           }
       } else if (itsFilterChoice.equals("MeqTableName")) {
           if (extension != null && extension.equals("mep")) {
               return true;
           } else {
               return false;
           }
       } else if (extension != null && itsFilterChoice.equals("SkyTableName")) {
           if (extension.equals("mep")) {
               return true;
           } else {
               return false;
           }
           
       } else if (extension != null && itsFilterChoice.equals("GetFlowEntry")) {
           if (extension.equals("cfg")) {
               return true;
           } else {
               return false;
           }
       } else if (extension != null && (itsFilterChoice.equals("NewConfig")    ||
               itsFilterChoice.equals("LoadConfig")   || 
               itsFilterChoice.equals("ConfigSaveAs"))) {
           if (extension.equals("cfg")) {
               return true;
           } else {
               return false;
           }
       } else {
           return true;
       }
    }
    
    public String getDescription() {
        return itsFilterDescription;
    }
    
    private String getExtension(File aF) {
        String ext=null;
        String s=aF.getName();
        int i= s.lastIndexOf('.');
        if (i >= 0 && i < s.length() -1) {
            ext = s.substring(i+1).toLowerCase();
        }
        return ext;
    }
    
    
}

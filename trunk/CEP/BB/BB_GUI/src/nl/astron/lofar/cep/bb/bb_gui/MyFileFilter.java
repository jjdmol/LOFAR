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
    
    /** Creates a new instance of MyFileChooser */
    public MyFileFilter(String aFilterChoice) {
        itsFilterChoice=aFilterChoice;
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
           if (extension.equals("ms")) {
               return true;
           } else {
               return false;
           }
       } else if (itsFilterChoice.equals("meqTableName")) {
           if (extension.equals("mep")) {
               return true;
           } else {
               return false;
           }
       } else if (itsFilterChoice.equals("skyTableName")) {
           if (extension.equals("mep")) {
               return true;
           } else {
               return false;
           }
           
       } else {
           return true;
       }
    }
    
    public String getDescription() {
        return itsFilterChoice;
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

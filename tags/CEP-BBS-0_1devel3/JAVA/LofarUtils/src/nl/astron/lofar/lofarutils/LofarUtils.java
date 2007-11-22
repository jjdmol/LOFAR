/*
 * LofarUtils.java
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
 *
 */ 

package nl.astron.lofar.lofarutils;

import javax.swing.DefaultListModel;
import javax.swing.JComboBox;
import javax.swing.JList;

/**
 * This panel contains a TreePanel and some textfields that display information
 * about the selected treenode. Also a log screen to be able to view logging on the
 * selected jobs.
 *
 * An event listener was added to catch TreeSelection events from the TreePanel
 *
 * @created 27-06-2006, 15:05
 * @author Coolen
 * @version $Id$
 * @updated
 */

public abstract class LofarUtils {
    
    
    /** Returns the c part of a string a.b.c*/
    static public String keyName(String aName) {
        String aS="";
        if (aName == null || aName.length() < 1) {
            return aS;
        }

        String s[] = aName.split("\\.");
        
        if (s.length>0) {
            return s[s.length-1];
        } else {
            return aName;
        }
    }
    
    /** Returns the a.b part of a string a.b.c */
    static public String moduleName(String aName) {
        String aS="";
        if (aName == null || aName.length() < 1) {
            return aS;
        }
        String s[] = aName.split("\\.");
        
        if (s.length>0) {
            aS=s[0];
            for (int i=1;i< s.length-1;i++) {
                aS+="."+s[i];
            }
        }
        return aS;        
    }
    
    /** Returns if the given string is a reference or not 
     * References are strings that start with >> for now.
     */
    static public boolean isReference(String aString) {
        // since an empty string will return true on the startswith method (see equals(onject) )
        // we need to check for it first.
        if (aString == null || aString.length() < 1) {
            return false;
        }
        if (aString.startsWith(">>")) {
            return true;
        } else {
            return false;
        }
    }
    
    /** converts string to boolean 
     * t, true returns true
     * f, false returns false
     */
    static public boolean StringToBoolean(String aS) {
        boolean aB = false;
        if (aS.equalsIgnoreCase("t") || aS.equalsIgnoreCase("true")) {
            aB=true;
        } else if (aS.equalsIgnoreCase("f") || aS.equalsIgnoreCase("false")) {
            aB=false;
        } else {
            System.out.println("getBoolean ERROR: no boolean: " + aS);
        }
        return aB;
    }
    
    /** converts boolean to string
     * false - f
     * true  - t
     */
    static public String BooleanToString(boolean aB) {
       if (aB) {
           return "t";
       } else {
           return "f";
       } 
    }

    /**
     * Creates a String representation of the contents of a JList component<br><br>
     * Example of output with createQuotes : ["123","456","789"]<br>
     * Example of output with !createQuotes: [123,456,789]<br>
     * Returns '[]' when no items were detected in the JList.
     *
     * @param aListComponent the JList component of which the data has to be converted
     * @param createQuotes add/do not add quotes to each individual value in the string output
     * @return String representation of aListComponent
     */
    static public String createList(JList aListComponent,boolean createQuotes) {
        String aList="[";
        if (aListComponent.getModel().getSize() > 0) {
            if(createQuotes){
                aList += "\"";
            }
            aList += (String)aListComponent.getModel().getElementAt(0);
            if(createQuotes){
                aList += "\"";
            }
            for (int i=1; i < aListComponent.getModel().getSize();i++) {
                aList+= ",";
                if(createQuotes){
                    aList += "\"";
                }
                aList += aListComponent.getModel().getElementAt(i);
                if(createQuotes){
                    aList += "\"";
                }
            }
            
        }
        aList+="]";
        return aList;
    }
        
    /**
     * Fills a JList component with a String representation of the to-be contents<br><br>
     * Example of input argument theList with removeQuotes : ["123","456","789"]<br>
     * Example of input argument theList with !removeQuotes: [123,456,789]
     *
     * @param aListComponent the JList component where the data has to be inserted in.
     * @param theList the String representation of a JList component
     * @param removeQuotes removes/does not remove quotes from each individual value in the theList input
     */
    static public void fillList(JList aListComponent,String theList,boolean removeQuotes) {
        DefaultListModel itsModel = new DefaultListModel();
        aListComponent.setModel(itsModel);
        String aList = theList;
        if (aList.startsWith("[")) {
            aList = aList.substring(1,aList.length());
        }
        if (aList.endsWith("]")) {
            aList = aList.substring(0,aList.length()-1);
        }
        if(!aList.equals("")){
            String[] aS=aList.split("\\,");
            for (int i=0; i< aS.length;i++) {
                if(removeQuotes){
                    itsModel.addElement(aS[i].substring(1,aS[i].length()-1));
                }else{
                    itsModel.addElement(aS[i]);
                }
            }
            aListComponent.setModel(itsModel);
        }
    }
    /**
     * Creates a String representation of the <b><i>selected</i></b> items in a JList component<br><br>
     * Example of output with createQuotes : ["123","456","789"]<br>
     * Example of output with !createQuotes: [123,456,789]<br>
     * Returns '[]' when no selected items were detected in the JList.
     * 
     * @param aListComponent the JList component of which the selected items have to be in a String
     * @param createQuotes add/do not add quotes to each individual value in the string output
     * @return String representation of the selected items in a aListComponent
     */
    static public String createStringFromSelectionList(JList aListComponent,boolean createQuotes) {
        String aList="[";
        if (aListComponent.getModel().getSize() > 0) {
            int[] selectedIndices = aListComponent.getSelectedIndices();
            for (int i=0; i < selectedIndices.length;i++) {
                if(i>0) aList+= ",";
                if(createQuotes){
                    aList += "\"";
                }
                aList += aListComponent.getModel().getElementAt(selectedIndices[i]);
                if(createQuotes){
                    aList += "\"";
                }
            }
        }
        aList+="]";
        return aList;
    }
    /**
     * Selects items in a JList component using a String representation of the to-be selected contents<br><br>
     * Example of input argument theList with removeQuotes : ["123","456","789"]<br>
     * Example of input argument theList with !removeQuotes: [123,456,789]
     *
     * @param aListComponent the JList component where the data has to be inserted in.
     * @param theList the String representation of the selected items in a JList component
     * @param removeQuotes removes/does not remove quotes from each individual value in the theList input
     */
    static public void fillSelectionListFromString(JList aListComponent,String theList,boolean removeQuotes) {
        String aList = theList;
        if (aList.startsWith("[")) {
            aList = aList.substring(1,aList.length());
        }
        if (aList.endsWith("]")) {
            aList = aList.substring(0,aList.length()-1);
        }
        if(!aList.equals("")){
            String[] aS=aList.split("\\,");
            String[] toBeSelectedValues = new String[aS.length];
            for (int i=0; i< aS.length;i++) {
                if(removeQuotes){
                    toBeSelectedValues[i] = aS[i].substring(1,aS[i].length()-1);
                }else{
                    toBeSelectedValues[i] = aS[i];
                }
            }
            int[] toBeSelectedIndices = new int[toBeSelectedValues.length];
            int aValueIndex = 0;
            if(toBeSelectedValues.length>0){
                for(String aValue : toBeSelectedValues){
                    
                    for(int in = 0; in < aListComponent.getModel().getSize();in++){
                        String aCorrType = (String)aListComponent.getModel().getElementAt(in);
                        if(aValue.equals(aCorrType)){
                            toBeSelectedIndices[aValueIndex] = in;
                        }
                    }
                    aValueIndex++;
                }
                aListComponent.setSelectedIndices(toBeSelectedIndices);
                
            }else{
                aListComponent.clearSelection();
            }
        }
    }
    
    /* Routine to strip all possible choices from a "popup" string <br><br>
     * Example of a popup string: "file|socket|ethernet;socket"<br> 
     * The | splitted values are the possible combochoices<br>
     * The ; splitted value is the default choice.
     * 
     * @param aComboBox the JComboBox that need to be set
     * @param aS the string that contains the possible choices
     */ 
    static public void setPopupComboChoices(JComboBox aComboBox,String aS) {
        aComboBox.removeAllItems();
        if (aS.length() < 1) {
            return;
        }
        // first strip input on ; because after the ; a default choice has been given.
        String[] stripped = aS.split("\\;");
        String defaultChoice = "";
        if (stripped.length > 1) {
            defaultChoice = stripped[1];
        }
        
        // now determine all possible choices and fill the combobox
        String[] choices = stripped[0].split("\\|");
        for (int i=0; i< choices.length;i++) {
            if (!choices[i].equals("")) {
                aComboBox.addItem(choices[i]);
            }
        }
        
        // set the default choice (if any)
        if (!defaultChoice.equals("")) {
            aComboBox.setSelectedItem(defaultChoice);
        } else {
            aComboBox.setSelectedIndex(0);
        }
        aComboBox.validate();
    }    
}

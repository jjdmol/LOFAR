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


import java.awt.Component;
import java.awt.KeyboardFocusManager;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.Collator;
import java.util.BitSet;
import java.util.regex.Pattern;
import javax.swing.DefaultListModel;
import javax.swing.Icon;
import javax.swing.JComboBox;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.text.JTextComponent;

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
        String aS = "";
        if (aName == null || aName.length() < 1) {
            return aS;
        }

        String s[] = aName.split("\\.");

        if (s.length > 0) {
            return s[s.length - 1];
        } else {
            return aName;
        }
    }

    /** Returns the a.b part of a string a.b.c */
    static public String moduleName(String aName) {
        String aS = "";
        if (aName == null || aName.length() < 1) {
            return aS;
        }
        String s[] = aName.split("\\.");

        if (s.length > 0) {
            aS = s[0];
            for (int i = 1; i < s.length - 1; i++) {
                aS += "." + s[i];
            }
        }
        return aS;
    }

    /** Returns if the given string is a reference or not 
     * References are strings that start with >> for now.
     */
    static public boolean isReference(String aString) {
        // since an empty string will return true on the startswith method (see equals(object) )
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

    /** converts boolean to string
     * false - f
     * true  - t
     */
    static public String BooleanToString(boolean aB) {
        if (aB) {
            return "true";
        } else {
            return "false";
        }
    }

    /** converts string to boolean
     * false - f,F,0,false,FALSE,False
     * true  - t,T,1,true,TRUE,True
     */
    static public boolean StringToBoolean(String aS) {
        if (aS.toLowerCase().startsWith("t") ||aS.toLowerCase().startsWith("1") ) {
            return true;
        } else {
            return false;
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
    static public String createList(JList aListComponent, boolean createQuotes) {
        String aList = "[";
        if (aListComponent.getModel().getSize() > 0) {
            if (createQuotes) {
                aList += "\"";
            }
            aList += (String) aListComponent.getModel().getElementAt(0);
            if (createQuotes) {
                aList += "\"";
            }
            for (int i = 1; i < aListComponent.getModel().getSize(); i++) {
                aList += ",";
                if (createQuotes) {
                    aList += "\"";
                }
                aList += aListComponent.getModel().getElementAt(i);
                if (createQuotes) {
                    aList += "\"";
                }
            }

        }
        aList += "]";
        return aList;
    }

    /**
     * Fills a JList component with a String representation of the to-be contents<br><br>
     * Example of input argument theList with !removeQuotes : ["123","456","789"]<br>
     * Example of input argument theList with removeQuotes: [123,456,789]
     *
     * @param aListComponent the JList component where the data has to be inserted in.
     * @param theList the String representation of a JList component
     * @param removeQuotes removes/does not remove quotes from each individual value in the theList input
     */
    static public void fillList(JList aListComponent, String theList, boolean removeQuotes) {
        DefaultListModel itsModel = new DefaultListModel();
        aListComponent.setModel(itsModel);
        String aList = theList;
        if (aList.startsWith("[")) {
            aList = aList.substring(1, aList.length());
        }
        if (aList.endsWith("]")) {
            aList = aList.substring(0, aList.length() - 1);
        }
        if (!aList.equals("")) {
            String[] aS = aList.split("\\,");
            for (int i = 0; i < aS.length; i++) {
                if (removeQuotes) {
                    itsModel.addElement(aS[i].substring(1, aS[i].length() - 1));
                } else {
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
    static public String createStringFromSelectionList(JList aListComponent, boolean createQuotes) {
        String aList = "[";
        if (aListComponent.getModel().getSize() > 0) {
            int[] selectedIndices = aListComponent.getSelectedIndices();
            for (int i = 0; i < selectedIndices.length; i++) {
                if (i > 0) {
                    aList += ",";
                }
                if (createQuotes) {
                    aList += "\"";
                }
                aList += aListComponent.getModel().getElementAt(selectedIndices[i]);
                if (createQuotes) {
                    aList += "\"";
                }
            }
        }
        aList += "]";
        return aList;
    }

    /**
     * Selects items in a JList component using a String representation of the to-be selected contents<br><br>
     * Example of input argument theList with !removeQuotes : ["123","456","789"]<br>
     * Example of input argument theList with removeQuotes: [123,456,789]
     *
     * @param aListComponent the JList component where the data has to be inserted in.
     * @param theList the String representation of the selected items in a JList component
     * @param removeQuotes removes/does not remove quotes from each individual value in the theList input
     */
    static public void fillSelectionListFromString(JList aListComponent, String theList, boolean removeQuotes) {
        String aList = theList;
        if (aList.startsWith("[")) {
            aList = aList.substring(1, aList.length());
        }
        if (aList.endsWith("]")) {
            aList = aList.substring(0, aList.length() - 1);
        }
        if (!aList.equals("")) {
            String[] aS = aList.split("\\,");
            String[] toBeSelectedValues = new String[aS.length];
            for (int i = 0; i < aS.length; i++) {
                if (removeQuotes) {
                    toBeSelectedValues[i] = aS[i].substring(1, aS[i].length() - 1);
                } else {
                    toBeSelectedValues[i] = aS[i];
                }
            }
            int[] toBeSelectedIndices = new int[toBeSelectedValues.length];
            int aValueIndex = 0;
            if (toBeSelectedValues.length > 0) {
                for (String aValue : toBeSelectedValues) {

                    for (int in = 0; in < aListComponent.getModel().getSize(); in++) {
                        String aCorrType = (String) aListComponent.getModel().getElementAt(in);
                        if (aValue.equals(aCorrType)) {
                            toBeSelectedIndices[aValueIndex] = in;
                        }
                    }
                    aValueIndex++;
                }
                aListComponent.setSelectedIndices(toBeSelectedIndices);

            } else {
                aListComponent.clearSelection();
            }
        }
    }

    /** Routine to strip all possible choices from a "popup" string <br><br>
     * Example of a popup string: "file|socket|ethernet;socket"<br> 
     * The | splitted values are the possible combochoices<br>
     * The ; splitted value is the default choice.
     * 
     * @param aComboBox the JComboBox that need to be set
     * @param aS the string that contains the possible choices
     */
    static public void setPopupComboChoices(JComboBox aComboBox, String aS) {
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
        for (int i = 0; i < choices.length; i++) {
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

    static public BitSet beamletToBitSet(String aS) {

        BitSet aBitSet = new BitSet(216);

        if (aS == null || aS.length() <= 2) {
            return aBitSet;
        }
        //remove [] from string
        String beamlets = aS.substring(aS.indexOf("[") + 1, aS.lastIndexOf("]"));

        // split into seperate beamlet nr's
        String[] beamletList = beamlets.split("[,]");

        //fill bitset

        for (int j = 0; j < beamletList.length; j++) {
            int val;
            try {
                val = Integer.parseInt(beamletList[j]);
                aBitSet.set(val);
            } catch (NumberFormatException ex) {
                System.out.println("Error converting beamlet numbers");

            }
        }
        return aBitSet;
    }

    static public String ltrim(String aS, String matchSet) {
        String reg = "^[" + matchSet + "]*";
        return Pattern.compile(reg).matcher(aS).replaceAll("");
    }

    static public String rtrim(String aS, String matchSet) {
        String reg = "[" + matchSet + "]*$";
        return Pattern.compile(reg).matcher(aS).replaceAll("");
    }
    
    static public int findFirstOf(String aS,String matchSet) {
        int first=-1;
        for (int i=0; i< matchSet.length();i++) {
            if (first != 0) {
                int found=aS.indexOf(matchSet.charAt(i));
                if (found < first) {
                    first=found;
                }
            } else {
                break;
            }
        }
        return first;
    }

    /** Function arrayFromList
     *
     * Given a comma seperated list, possibly within brackets this function will return all the comma seperated items as an array from strings
     *
     * @param  aList    the comma seperated List
     * @return the list entries as String []
     */

    static public String[] arrayFromList(String aList) {

        // split list and strip tab,space,",',[ and ]
        String aS1=aList.replaceAll("[\\s\"\'\\]\\[]+", "");
        String[] aV = aS1.split(",");

        return aV;
    }

    /** Function to return nr of elements in an stringArray
     *
     */
    static public int nrArrayStringElements(String orgStr) {
        int nr = -1;
  	// destroyable copy, containing expanded String to avoid allready available x..y constructions
  	String baseStr=LofarUtils.expandedArrayString(orgStr);

  	// strip the brackets, space and tab
  	baseStr =LofarUtils.ltrim(baseStr," 	\\[");
  	baseStr =LofarUtils.rtrim(baseStr," 	\\]");


        //split the baseString into individual numbers
        String[] splitted = baseStr.split("[,]");

        return splitted.length;
    }

    /**  Function compactedArrayString(string)
     * 
     *
     *
     * Given een array string ( '[ xx, xx, xx ]' ) this utility compacts the string
     * by replacing series with range.
     * Eg. [ lii001, lii002, lii003, lii005 ] --> [ lii001..lii003, lii005 ]
     *
     * ATTENTION !!!!!   this code is a duplicate from the code used in: LOFAR/LCS/Common/src/StringUtils.cc
     * and Navigator/libs/gcf-util.ctl for PVSS
     * If a structural bug is discovered. or the code has been modified then you also should have a look there !!!
     * 
     *
     * @param  orgStr    The string that needs to be compacted
     * @return the compacted string or the orgStr after failure
     * 
     * ATTENTION, because we only need it for number only strings here, and JAVA has no printf,scanf compatibility easy
     * we only will do it for numbers now, this might need to be changed later 
     */

    static public String compactedArrayString(String orgStr) {
        
  	// destroyable copy, containing expanded String to avoid allready available x..y constructions
  	String baseStr=LofarUtils.expandedArrayString(orgStr);

  	// strip the brackets, space and tab
  	baseStr =LofarUtils.ltrim(baseStr," 	\\[");
  	baseStr =LofarUtils.rtrim(baseStr," 	\\]");

        
        //split the baseString into individual numbers
        String[] splitted = baseStr.split("[,]");
        
        String result="[";
        int sum=0;
        int oldnum=-1;
        int anI=-1;
        int len = splitted.length;

        //if len <= 2 return, since only 2 nrs max are available
        if (len <= 2 ) return orgStr;

        try {
            for (String aS:splitted) {
                // return if non digits are found
                if (!aS.matches("-?\\d+" )) return orgStr;

                anI = Integer.valueOf(aS).intValue();
                if (oldnum == -1) {
                    oldnum=anI;
                    result+= anI;
                    sum++;
                } else {
                    // check if sequence
                    if (anI != oldnum+1) {
                        // not in sequence anymore, close last sequence
                        // and reset counters
                        if (sum == 2) {
                            // don't compact 13,14 into 13..14
                            result += ","+oldnum;
                        } else if (sum >2) {
                            result += ".."+oldnum;
                        }
                        // write new startSequence
                        result += ","+anI;
                        //reset sequence counter
                        sum=1;
                    } else {
                        sum++;
                    }
                    oldnum=anI;
                }
            }
            // add last found number and close
            // and reset counters
            if (sum > 2) {
                result += ".."+oldnum;
            } else if ( sum == 2) {
                result += ","+oldnum;
            } else if (oldnum != anI) {
                result += ","+anI;
            }

        } catch (Exception ex) {
            System.out.println("Error in CompactedArrayString: " + ex.getMessage());
            return orgStr;
        }

        result += "]";
        return result;
    }

    static public String expandedArrayString(String orgStr) {
        
  	// destroyable copy
  	String baseStr=orgStr;

        //no use expand strings thyat don't have .. sequence'
        if (!orgStr.contains("..")) return orgStr;

  	// strip the brackets, space and tab
  	baseStr =LofarUtils.ltrim(baseStr," 	\\[");
  	baseStr =LofarUtils.rtrim(baseStr," 	\\]");
        
        //split the baseString into individual numbers
        String[] splitted = baseStr.split("[,]");
        
        String result="[";
        boolean first=true;
         
        try {
            for (String aS:splitted) {

                // check if a .. is found in the string
                if (aS.contains("..")) {
                    String [] split = aS.split("[.]+");
                    int lowI  = Integer.valueOf(split[0]).intValue();
                    int highI = Integer.valueOf(split[1]).intValue();
                    for(int i=lowI; i<= highI;i++) {
                        if (first){
                            result+=i;
                            first=false;
                        } else { 
                            result+=","+i;
                        }
                    }
                } else if (!aS.isEmpty()){
                  int anI = Integer.valueOf(aS).intValue();
                  if (first) {
                      result+= anI;
                      first=false;
                  } else {
                      result+=","+anI;
                  }  
                }
            }
        } catch (Exception ex) {
            System.out.println("Error in CompactedArrayString: " + ex.getMessage());
            return orgStr;
        }

        result += "]";
        return result;
    }

    static public void sortModel(DefaultListModel model)
    {
        int numItems = model.getSize();
        String[] a = new String[numItems];
        model.copyInto(a);
        LofarUtils.sortArray(a);
        model.clear();
        for (int i=0;i<a.length;i++)
        {
            model.addElement(a[i]);
        }
    }

    static public void sortArray(String[] strArray)
    {
        if (strArray.length == 1)    // no need to sort one item
            return;
        Collator collator = Collator.getInstance();
        String strTemp;
        for (int i=0;i<strArray.length;i++)
        {
            for (int j=i+1;j<strArray.length;j++)
            {
                if (collator.compare(strArray[i], strArray[j]) > 0)
                {
                    strTemp = strArray[i];
                    strArray[i] = strArray[j];
                    strArray[j] = strTemp;
                }
            }
        }
    }

    /** Shows a generic error panel
     *
     * @param parent parent component to which the panel will belong
     * @param aS the string that contains the error message
     */
    static public void showErrorPanel(Component parent,String aS,Icon icon) {
        if (aS.isEmpty()) {
            aS="There has been an error, check logging for details";
        }

        JOptionPane.showMessageDialog(parent,aS,"LOFAR Error",JOptionPane.WARNING_MESSAGE,icon);

    }

    /** Function rad2deg
     *  calculates notation in degrees from rad
     *
     * @param  rad   input in rad
     * @returns degrees
    */
    static public double rad2deg(double rad) {
        double deg=0;

        deg=rad*(360/(2*Math.PI));
        return deg;
    }

    /** Function deg2rad
     *  calculates notation in rad from degrees
     *
     * @param  rad   input in rad
     * @returns degrees
    */
    static public double deg2rad(double deg) {
        double rad=0;

        rad=deg/360*2*Math.PI;
        return rad;
    }

    /** Function rad2hms
     *  calculates notation in hms from rad
     *
     * @param  rad   input in rad
     * @returns string with h:m:s
     */
    static public String rad2hms(double rad) {

        if (rad < 0) rad += 2*Math.PI;
        return(LofarUtils.deg2hms(LofarUtils.rad2deg(rad)));
    }

    /** Function hms2rad
     *  calculates notation in rad from hms
     *
     * @param  hms   input in string with h:m:s
     * @returns rad
     */
    static public double hms2rad(String hms) {

        return(LofarUtils.deg2rad(LofarUtils.hms2deg(hms)));
    }


     /** Function rad2dms
     *  calculates notation in hms from rad
     *
     * @param  rad   input in rad
     * @returns string with d:m:s
     */
    static public String rad2dms(double rad) {

        return(LofarUtils.deg2dms(LofarUtils.rad2deg(rad)));
    }


   /** Function dms2rad
     *  calculates notation in rad from dms
     *
     * @param  dms   input in string with d:m:s
     * @returns rad
     */
    static public double dms2rad(String dms) {

        return(LofarUtils.deg2rad(LofarUtils.dms2deg(dms)));
    }

    /** Function deg2hms
     *  calculates notation in hms from degrees
     *
     * @param  degrees   input in degrees
     * @returns string with h:m:s
     */
    static public String deg2hms(double deg) {
        String hms="";

        if (deg < 0) deg += 360;

        int    h = (int) deg/15;
        int    m = (int) ((deg-h*15)/15*60);
        double s =  (deg-(h*15.)-(m*15./60))/15.*3600;

        hms=Integer.toString(h)+":"+Integer.toString(m)+":"+Double.toString(s);
        return hms;
    }

    /** Function hms2deg
     *  calculates notation in deg from hms
     *
     * @param  hms   string with h:m:s
     * @returns degrees
     */
    static public double hms2deg(String hms) throws NumberFormatException {
        double deg = 0;

        if (hms.isEmpty()) {
            return deg;
        }

        String[] values = hms.split(":");
        int h = Integer.valueOf(values[0]);
        int m = Integer.valueOf(values[1]);
        double s = Double.valueOf(values[2]);

        deg = (h * 15.) + (m * 15. / 60) + (s * 15. / 3600);

        return deg;
    }

    /** Function deg2dms
     *  calculates notation in hms from degrees
     *
     * @param  degrees   input in degrees
     * @returns string with d:m:s
     */
    static public String deg2dms(double deg) {
        String hms="";

        int    d = (int) deg;
        int    m = (int) ((deg-d)*60);
        double s =  (deg-d-(m/60.))*3600.;

        if (m<0) m*=-1;
        if (s<0) s*=-1;

        hms=Integer.toString(d)+":"+Integer.toString(m)+":"+Double.toString(s);
        return hms;
    }

    /** Function dms2deg
     *  calculates notation in deg from dms
     *
     * @param  hms   string with h:m:s
     * @returns degrees
     */
    static public double dms2deg(String dms) throws NumberFormatException {
        double deg=0;

        if (dms.isEmpty()) return deg;

        String [] v1 = dms.split(":");
        int d = Integer.valueOf(v1[0]);
        int m = Integer.valueOf(v1[1]);
        double s = Double.valueOf(v1[2]);

        if (d < 0) {
            if (m>0) m*=-1;
            if (s>0) s*=-1;
        }
        deg= d+(m/60.)+(s/3600.);

        return deg;
    }

    /** Function dms2hms
     *  calculates notation in hms from dms
     *
     * @param  dms   string with dms
     * @returns string with h:m:s
     */
    static public String dms2hms(String dms) {

        if (dms.isEmpty()) return dms;

        return LofarUtils.rad2hms(LofarUtils.dms2rad(dms));
    }

    /** Function hms2dms
     *  calculates notation in dms from hms
     *
     * @param  hms   string with h:m:s
     * @returns string with dms
     */
    static public String hms2dms(String hms) {

        if (hms.isEmpty()) return hms;

        return LofarUtils.rad2dms(LofarUtils.hms2rad(hms));
    }

    /** Function changeCoordinate(String from, String to, String coordinate)
     *  returns value in another coordinate type
     *
     * @param  from        Coordinate type old value is in (rad,deg or hms)
     * @param  to          Coordinate type new value will be in (rad,deg or hms)
     * @param  coordinate  The value that needs to be recalculated
     * @returns string     with new coord value
     */
    static public String changeCoordinate(String from, String to, String coordinate) throws NumberFormatException {

        if (from.equals("rad")) {
            if (to.equals("deg")) {
                return Double.valueOf(LofarUtils.rad2deg(Double.valueOf(coordinate))).toString();
            } else if (to.equals("hms")) {
                return LofarUtils.rad2hms(Double.valueOf(coordinate));
            } else if (to.equals("dms")) {
                return LofarUtils.rad2dms(Double.valueOf(coordinate));
            }
        } else if (from.equals("deg")) {
            if (to.equals("rad")) {
                return Double.valueOf(LofarUtils.deg2rad(Double.valueOf(coordinate))).toString();
            } else if (to.equals("hms")) {
                return LofarUtils.deg2hms(Double.valueOf(coordinate));
            } else if (to.equals("dms")) {
                return LofarUtils.deg2dms(Double.valueOf(coordinate));
            }
        } else if (from.equals("hms")) {
            if (to.equals("rad")) {
                return Double.valueOf(LofarUtils.hms2rad(coordinate)).toString();
            } else if (to.equals("deg")) {
                return Double.valueOf(LofarUtils.hms2deg(coordinate)).toString();
            } else if (to.equals("dms")) {
                return LofarUtils.hms2dms(coordinate);
            }
        } else if (from.equals("dms")) {
            if (to.equals("rad")) {
                return Double.valueOf(LofarUtils.dms2rad(coordinate)).toString();
            } else if (to.equals("deg")) {
                return Double.valueOf(LofarUtils.dms2deg(coordinate)).toString();
            } else if (to.equals("hms")) {
                return LofarUtils.dms2hms(coordinate);
            }
        }

        // else return unchanged initial value
        return coordinate;
    }

    /** Function directionTypeToAngle1(String coordType,int angle)
     *  returns angle name for a give coordType
     *
     * @param  directionType   Name of the direction Type
     * @param  angle       Angle1 or Angle2
     * @return name to use for angle
     */
    static public String directionTypeToAngle(String directionType,int angle){
        String Angle1="Angle1";
        String Angle2="Angle2";

        if (directionType.equalsIgnoreCase("J2000")) {
            Angle1="Right Ascension";
            Angle2="Declination";
        } else if (directionType.equalsIgnoreCase("ITRF")) {
        } else if (directionType.equalsIgnoreCase("B1950")) {
            Angle1="Right Ascension";
            Angle2="Declination";
        } else if (directionType.equalsIgnoreCase("HADEC")) {
            Angle1="Hour Angle";
            Angle2="Declination";
        } else if (directionType.equalsIgnoreCase("AZELGEO")) {
            Angle1="Azimuth";
            Angle2="Elevation";
        } else if (directionType.equalsIgnoreCase("TOPO")) {
        } else if (directionType.equalsIgnoreCase("ICRS")) {
        } else if (directionType.equalsIgnoreCase("APP")) {
            Angle1="Right Ascension";
            Angle2="Declination";
        } else if (directionType.equalsIgnoreCase("GALACTIC")) {
            Angle1="Longitude";
            Angle2="Latitude";
        } else if (directionType.equalsIgnoreCase("COMET")) {
            Angle1="Longitude";
            Angle2="Latitude";
        } else if (directionType.equalsIgnoreCase("MERCURY")) {
        } else if (directionType.equalsIgnoreCase("VENUS")) {
        } else if (directionType.equalsIgnoreCase("MARS")) {
        } else if (directionType.equalsIgnoreCase("JUPITER")) {
        } else if (directionType.equalsIgnoreCase("SATURN")) {
        } else if (directionType.equalsIgnoreCase("URANUS")) {
        } else if (directionType.equalsIgnoreCase("NEPTUNE")) {
        } else if (directionType.equalsIgnoreCase("PLUTO")) {
        } else if (directionType.equalsIgnoreCase("SUN")) {
        } else if (directionType.equalsIgnoreCase("MOON")) {
        }

        if (angle==1) {
            return Angle1;
        } else if (angle==2) {
            return Angle2;
        } else {
            return "";
        }
    }
    
        static public class TextSelector {

            private static FocusHandler installedInstance;

            /**
             * Install an PropertyChangeList listener to the default focus manager
             * and selects text when a text component is focused.       
             */
            public static void install() {
                //already installed
                if (installedInstance != null) {
                    return;
                }

                installedInstance = new FocusHandler();

                KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();



                kfm.addPropertyChangeListener("focusOwner", installedInstance);
            }

            public static void uninstall() {
                if (installedInstance != null) {
                    KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();
                    kfm.removePropertyChangeListener("focusOwner", installedInstance);
                }
            }

            private static class FocusHandler implements PropertyChangeListener {

            @Override
                public void propertyChange(PropertyChangeEvent evt) {
                    if (evt.getNewValue() instanceof JTextComponent) {
                        JTextComponent text = (JTextComponent) evt.getNewValue();
                       //select text if the component is editable
                        //and the caret is at the end of the text
                        if (text.isEditable()) {
//                                && text.getDocument().getLength() == text.getCaretPosition()) {

                            text.select(0,text.getDocument().getLength());
                        }
                    }
                }
            }
        }


}   
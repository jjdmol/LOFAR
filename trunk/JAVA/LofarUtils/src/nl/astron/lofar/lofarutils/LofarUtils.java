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

        String s[] = aName.split("[.]");
        
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
        String s[] = aName.split("[.]");
        
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
}

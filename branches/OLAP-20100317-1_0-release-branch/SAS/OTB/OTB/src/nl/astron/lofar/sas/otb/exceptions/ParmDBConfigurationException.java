/*
 * ParmDBConfigurationException.java
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

package nl.astron.lofar.sas.otb.exceptions;

/**
 * This exception can be thrown if a the XML file containing the PARMDB tables and such encounters an exception that will have to
 * be logged and/or communicated with the user.
 * 
 * @author pompert
 * @version ParmDBConfigurationExceptionaAccessException.java,v 1.5 2006/05/05 13:35:26 pompert Exp $
 * @see ParmDBConfigurationHelper
 * @created June 08, 2006, 13:22
 */
public class ParmDBConfigurationException extends Exception{
    
    private String message;
    
    /**
     * 
     * Creates a new instance of ParmDBConfigurationException, using the supplied message
     * 
     * @param message A custom string to be added to the exception message
     */
    public ParmDBConfigurationException(String message) {
        super();
        this.message = message;
    }
    /** 
     * Returns the exception message that was set in the constructor
     * @return the Exception message
     */
    public String getMessage(){
        return message;
    }
    
    
}

/*
 * NoAccessException.java
 *
 * Created on January 17, 2006, 5:28 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.exceptions;

/**
 *
 * @author blaakmeer
 */
public class NoAccessException extends Exception {
    
    /** Creates a new instance of NoAccessException */
    public NoAccessException() { }
    public NoAccessException(String msg) { super(msg); }
    
}

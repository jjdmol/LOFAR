/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.astron.lofar.sas.otb.util;

/**
 *
 * @author coolen
 */

public interface IBeam extends Cloneable {
    
    public IBeam clone();
    
    // give type of beam
    public String getType();   
    
}

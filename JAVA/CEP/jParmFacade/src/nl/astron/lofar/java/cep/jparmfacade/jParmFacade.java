/*
 * jParmFacade.java  JNI Interface base class for the CEP/BB/ParmFacade class
 *
 * Created on April 18, 2006, 9:54 AM
 *
 */

package nl.astron.lofar.java.cep.jparmfacade;

import java.util.HashMap;
import java.util.Vector;

/**
 *
 * @author coolen
 */
public class jParmFacade {
    
    /** Creates a new instance of jParmFacade */
    public jParmFacade(String tableName) {
        try {
            initParmFacade(tableName);
        } catch (Exception ex) {
            System.out.println("Error during init: ");
            ex.printStackTrace();
        }
    }
    
    // Make a connection to the given ParmTable.
    private native void initParmFacade (String tableName) throws Exception;
    
    // Get the domain range (as startx,endx,starty,endy) of the given
    // parameters in the table.
    // This is the minimum start value and maximum end value for all parameters.
    // An empty name pattern is the same as * (all parm names).
    public native Vector<Double> getRange(String parmNamePattern) throws Exception;
    
    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    public native Vector<String> getNames(String parmNamePattern) throws Exception;
    

    // Get the parameter values for the given parameters and domain.
    // The domain is given by the start and end values, while the grid is
    // given by nx and ny.
    public native HashMap<String,Vector<Double>> getValues(String parmNamePattern,
            double startx, double endx, int nx,
            double starty, double endy, int ny) throws Exception;
    
}

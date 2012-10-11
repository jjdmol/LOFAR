/* 
 * tParmFacade.java: test program for class jParmFacade
 *
 * Created on April 19, 2006, 11:05 AM
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacade;

/**
 *
 * @author coolen
 */
public class tParmFacade {

    static {
        System.loadLibrary("jparmfacade");
    }
 

    public static void main(String[] args) {
        tParmFacade tPF = new tParmFacade();
        String parmTable="tParmFacade.in_mep1";
        
        if (args.length <  1) {
            System.out.println("Run as: tParmFacade parmtable");
            tPF.test(parmTable);
        } else {
            parmTable=args[0];
            System.out.println("Running with parmTable: "+parmTable);
        
            tPF.test(parmTable);
        }
     }
    
    /** Creates a new instance of tParmFacade */
    private void test(String parmTable) {
        System.out.println("Testing with table: "+parmTable);
        try {
            // create a jParmFacade
            jParmFacade aPF = new jParmFacade();
            aPF.setParmFacadeDB(parmTable);
        
            if (aPF ==  null) {
                System.out.println("Error starting ParmFacade");
                return;
            }
        
            Vector<String> names = aPF.getNames("");
            System.out.println("Names: " + names);
            Vector<Double> range = aPF.getRange("");
            System.out.println("Range: " + range);
            HashMap<String,Vector<Double> > values = aPF.getValues ("*",
							range.elementAt(0), range.elementAt(1), 4,
							range.elementAt(2), range.elementAt(3), 2);

         
            Iterator it = values.keySet().iterator();
            while (it.hasNext()) {  
                String key = (String)it.next();
                System.out.println(key + "  <->  " + values.get(key));
            }
        } catch (Exception ex) {
            System.out.println("Unexpected exception: " + ex);
            return;
        }
    }
}

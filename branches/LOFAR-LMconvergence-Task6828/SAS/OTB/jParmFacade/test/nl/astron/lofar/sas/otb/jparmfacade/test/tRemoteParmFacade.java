package nl.astron.lofar.sas.otb.jparmfacade.test;
/* 
 * tRemoteParmFacade.java: test program for class jParmFacade via RMI
 *
 * Created on June 1, 2006, 11:05 AM
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

import java.rmi.Naming;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import nl.astron.lofar.sas.otb.jparmfacade.jParmFacadeInterface;

/**
 *
 * @author coolen
 */
public class tRemoteParmFacade {
    
    private static jParmFacadeInterface aPF;
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "10777";
    public static String RMIValName         = jParmFacadeInterface.SERVICENAME;

    public static void main(String[] args) {
        tRemoteParmFacade tPF = new tRemoteParmFacade();
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
            aPF = (jParmFacadeInterface) Naming.lookup ("rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIValName);
        
            if (aPF ==  null) {
                System.out.println("Error starting ParmFacade");
                return;
            }
            System.out.println("Working with DB: ");
            aPF.setParmFacadeDB(parmTable);
            System.out.println(parmTable);
            
            System.out.println("Getting names: ");
            ArrayList<String> names = aPF.getNames("");
            System.out.println("Found " + names);
            
            
            System.out.println("Getting Range: ");
            ArrayList<Double> range = aPF.getRange("");
            System.out.println("Found " + range);
            
            
            System.out.println("Getting Values: ");
            HashMap<String,ArrayList<Double> > values = aPF.getValues ("*",
							range.get(0), range.get(1), 4,
							range.get(2), range.get(3), 2);

            Iterator it = values.keySet().iterator();
            System.out.println("Found: ");
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
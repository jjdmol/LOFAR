/*
 * CmdExec.java
 *
 * Created on January 16, 2006, 11:16 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.otb.util;

import java.io.*;

/**
 *
 * @author blaakmeer
 */
public class CmdExec {
    
    /** Creates a new instance of CmdExec */
    public CmdExec(String cmdline) {
        try {
            String line;
            Process p = Runtime.getRuntime().exec(cmdline);
            BufferedReader input =
            new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = input.readLine()) != null) {
                System.out.println(line);
            }
            input.close();
        }
        catch (Exception err) {
            err.printStackTrace();
        }
    }
}

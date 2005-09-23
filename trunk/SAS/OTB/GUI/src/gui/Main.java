/*
 * Main.java
 *
 * Created on September 9, 2005, 2:21 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package gui;



/**
 *
 * @author coolen
 */
public class Main {
 
 
    /** Creates a new instance of Main */
    public Main() {
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
         // TODO code application logic here
        otbgui aGui = new otbgui();
        aGui.setSize(900, 500);
        aGui.setVisible(true);
    }    
}

/*
 * Main.java
 *
 * Created on October 14, 2005, 10:52 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package tfc_gui;

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
       tfc_gui aGui = new tfc_gui();
       aGui.setSize(950,600);
       aGui.setVisible(true);
    }
    
}

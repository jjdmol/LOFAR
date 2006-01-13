/*
 * Main.java
 *
 * Created on October 18, 2005, 2:35 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.cep.bb.bb_gui;

/**
 *
 * @author coolen
 */
public class Main {
    
    public static BB_Gui itsGui;
    
    /** Creates a new instance of Main */
    public Main() {
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        initGui();
    }
    
    private static void initGui() {
        itsGui = new BB_Gui();
        itsGui.setSize(910,825);
        itsGui.setResizable(false);
        itsGui.setVisible(true);
    }
    
}

/* ListDialog.java
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
 */

package nl.astron.lofar.sas.otbcomponents;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;


/**
 *
 * @created 14-07-2006, 9:31
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class ListDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(ListDialog.class);
    static String name = "ListDialog";
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs
     * @param   modal       Should the Dialog be modal or not
     * @param   aList       the List to work with
     * @param   aTitle      the title for the dialog
     */
    public ListDialog(java.awt.Frame parent, boolean modal, String aList, String aTitle) {
        super(parent, modal);
        initComponents();
        itsList = aList;
        itsTitle = aTitle;
        listPanel.setTitle(aTitle);
        listPanel.setList(aList);
        listPanel.validate();
        listPanel.setVisible(true);
    }
    
    public String getList() {        
        return itsList;
    }
    
    public boolean hasChanged() {
        return isChanged;
    }
    
    /** if this var is a reference, we need to make that obvious
     */
    public void setWarning(String aS) {
        listPanel.setWarning(aS);
    }
    
    public void removeWarning() {
        listPanel.removeWarning();
    }

    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        listPanel = new nl.astron.lofar.sas.otbcomponents.ListPanel();

        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog");
        setResizable(false);
        getContentPane().add(listPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, -1, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents


    private MainFrame itsMainFrame = null;
    private String itsList = "";
    private String itsTitle = "";
    private boolean isChanged=false;
            
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ListPanel listPanel;
    // End of variables declaration//GEN-END:variables

}

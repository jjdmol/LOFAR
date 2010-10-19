/*
 * FlowListRenderer.java
 *
 * Created on January 16, 2006, 3:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.cep.bb.bb_gui;

import java.awt.Component;
import java.io.File;
import javax.swing.DefaultListCellRenderer;
import javax.swing.JList;

/**
 *
 * @author coolen
 */
public class FlowListRenderer extends DefaultListCellRenderer {
    
    /** Creates a new instance of FlowListRenderer */
    public FlowListRenderer() {
    }

    public Component getListCellRendererComponent(JList list, 
                                              Object value,
                                              int index, boolean isSelected,
                                              boolean cellHasFocus) {

    super.getListCellRendererComponent(list, 
                                       value, 
                                       index, 
                                       isSelected, 
                                       cellHasFocus);
    setText(((File)value).getName());
    return this;
  }
}

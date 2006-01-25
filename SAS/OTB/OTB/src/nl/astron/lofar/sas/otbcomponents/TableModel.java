/*
 * TableModel.java
 *
 * Created on January 13, 2006, 4:48 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otbcomponents;

/**
 *
 * @author blaakmeer
 */
public class TableModel extends javax.swing.table.AbstractTableModel {
    
    /** Creates a new instance of TableModel */
    public TableModel() {
    }

    public int getRowCount() {
        return data.length;
    }

    public String getColumnName(int c) {
        return headers[c];
    }

    public int getColumnCount() {
        return headers.length;
    }

    public Object getValueAt(int r, int c) {
        return data[r][c];
    }
    
   
    private String headers[] = {"header1","header2"};
    private Object data[][] = {{null,null},{null,null}};
}

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
    
    private String headers[];
    private Object data[][];

    /** Creates a new instance of TableModel */
    public TableModel(String headers[],Object data[][]) {
        this.headers = headers;
        this.data = data;
    }

    public int getRowCount() {
        return data.length;
    }

    public String getColumnName(int c) {
        try {
            return headers[c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }

    public int getColumnCount() {
        return headers.length;
    }

    public Object getValueAt(int r, int c) {
        try {
            return data[r][c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }
}

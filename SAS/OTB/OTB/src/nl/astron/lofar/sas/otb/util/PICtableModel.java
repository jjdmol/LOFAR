/*
 * PICtableModel.java
 *
 * Created on January 31, 2006, 11:11 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

/**
 * Implements the data model behind the PIC table 
 *
 * @author blaakmeer
 */
public class PICtableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"dit","dat","wat","nogiets","laatste"};
    private OtdbRmi otdbRmi;

    /** Creates a new instance of PICtableModel */
    public PICtableModel(OtdbRmi otdbRmi) {
        this.otdbRmi = otdbRmi;
    }

    /** Returns the number of rows */
    public int getRowCount() {
        int rowCount;
        
        //TODO: get rowcount from OTDB
        rowCount = 10;
        return rowCount;
    }

    /** Returns the column names */
    public String getColumnName(int c) {
        try {
            return headers[c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }

    /** Returns the number of columns */
    public int getColumnCount() {
        return headers.length;
    }

    /** Returns the value at row r and column c */
    public Object getValueAt(int r, int c) {
        try {
            //TODO: get data from OTDB. now returning test data
            return new Integer(r*c);
        }
        catch(ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }
}

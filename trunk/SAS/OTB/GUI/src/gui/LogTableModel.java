/*
 * LogTableModel.java
 *
 * Created on September 23, 2005, 11:55 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package gui;

import jOTDB.jOTDBinterface;
import jOTDB.jOTDBtree;
import java.util.Vector;
import javax.swing.table.AbstractTableModel;

/**
 *
 * @author coolen
 */
public class LogTableModel extends AbstractTableModel {
    
       
       String headers[] = {"TimeStamp", "Value"};
       Object data[][];
       
             
       public LogTableModel(){
          setLogList();
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
            
       public void setLogList() {
       }
}

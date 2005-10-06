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
import jOTDB.jOTDBvalue;
import jOTDB.jTreeValueInterface;
import java.util.Vector;
import javax.swing.table.AbstractTableModel;

/**
 *
 * @author coolen
 */
public class LogTableModel extends AbstractTableModel {
    
       
       String headers[] = {"TimeStamp", "Value"};
       Object data[][];
       boolean itsDebugFlag=false;
       
       public void setDebugFlag(boolean aFlag) {
           itsDebugFlag=aFlag;
       }
             
       public LogTableModel(jTreeValueInterface anInterface, Vector aLogList){
          setLogList(anInterface, aLogList);
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
            
      public void setLogList(jTreeValueInterface aRemoteValue,Vector aLogList) {
           data = new Object[aLogList.size()][headers.length];
                try {
                    for (int k=0; k< aLogList.size();k++) {
                        data[k][0]=((jOTDBvalue)aLogList.elementAt(k)).time;
                        data[k][1]=((jOTDBvalue)aLogList.elementAt(k)).value;
                    }
                    fireTableDataChanged();
                  } 
                  catch (Exception e)
                  {
	            System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	          }  
            }
}

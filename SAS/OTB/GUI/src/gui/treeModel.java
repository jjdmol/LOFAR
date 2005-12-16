/*
 * treeModel.java
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
import jOTDB.jConverterInterface;
import java.util.Vector;
import javax.swing.table.AbstractTableModel;
import java.rmi.Naming;


/**
 *
 * @author coolen
 */
public class treeModel extends AbstractTableModel {
    
       boolean itsDebugFlag=false;
       String headers[] = {"TreeID", "Creator", "CreationDate", "Classification", 
            "Type", "State", "Campaign", "Start Time", "Stop Time"};
       Object data[][];
       
       public void setDebugFlag(boolean aFlag) {
           itsDebugFlag=aFlag;
       }
             
       public treeModel(jOTDBinterface aRemoteOTDB, Vector aTreeList){
          setTreeList(aRemoteOTDB,aTreeList);
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
            
       public void setTreeList(jOTDBinterface aRemoteOTDB,Vector aTreeList) {
           data = new Object[aTreeList.size()][headers.length];
           
                try {
                    
                    String aRMC="rmi://"+otbgui.RMIServerName+":"+otbgui.RMIServerPort+"/"+otbgui.RMIConverterName;

                    jConverterInterface types = (jConverterInterface) Naming.lookup (aRMC); 
                    
                    if (aTreeList.size() == 0) {
                        if (itsDebugFlag) System.out.println("Error:" + aRemoteOTDB.errorMsg());
                    } else {
                        if (itsDebugFlag) System.out.println("Collected tree list");
                    }
                    for (int k=0; k< aTreeList.size();k++) {
                        if (itsDebugFlag) System.out.println("getTreeInfo(aTreeList.elementAt("+k+"),false)");
	                Integer i = new Integer((Integer)aTreeList.elementAt(k));
	                jOTDBtree tInfo = aRemoteOTDB.getTreeInfo(i.intValue(), false);
	                if (tInfo.treeID()==0) {
                            if (itsDebugFlag) System.out.println("No such tree found!");
                        } else {
                            if (itsDebugFlag) System.out.println("Gathered info for ID: "+tInfo.treeID());
                        
                            data[k][0]=new Integer(tInfo.treeID());	   
	                    data[k][1]=new String(tInfo.creator);
	                    data[k][2]=new String(tInfo.creationDate);	
	                    data[k][3]=new String(types.getClassif(tInfo.classification));
	                    data[k][4]=new String(types.getTreeType(tInfo.type));
	                    data[k][5]=new String(types.getTreeState(tInfo.state));
	                    data[k][6]=new String(tInfo.campaign);	
	                    data[k][7]=new String(tInfo.starttime);
	                    data[k][8]=new String(tInfo.stoptime);
	                }
                    }
                    fireTableDataChanged();
                  } 
                  catch (Exception e)
                  {
	            System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	          }  
            }
}

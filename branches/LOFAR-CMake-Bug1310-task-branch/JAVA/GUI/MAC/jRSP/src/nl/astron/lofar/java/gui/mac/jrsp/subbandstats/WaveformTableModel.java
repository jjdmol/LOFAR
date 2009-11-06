/*
 * WaveformTableModel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.gui.mac.jrsp.subbandstats;

import java.util.ArrayList;
import javax.swing.event.TableModelEvent;
import javax.swing.table.AbstractTableModel;
import nl.astron.lofar.java.mac.jrsp.WGRegisterType;

/**
 *
 * @author balken
 */
public class WaveformTableModel extends AbstractTableModel 
{
    /** Headers */
    private String[] headers = {"Board", "Antenna", "Frequency", "Phase", "Amplitude"};
    
    /** Data */
    private ArrayList<WGRegisterType> data;
    
    /**
     * Constructor
     */
    public WaveformTableModel()
    {
        data = new ArrayList<WGRegisterType>();
    }
    
    public int getRowCount()
    {
        return data.size();
    }
    
    public int getColumnCount()
    {
        return headers.length;
    }
    
    public String getColumnName(int column)
    {
        return headers[column];
    }
    
    public Object getValueAt(int row, int column)
    {
        switch(column)
        {
            case 0:
                return data.get(row).board;
            case 1:
                return data.get(row).antenna;
            case 2:
                return data.get(row).getFrequency();
            case 3:
                return data.get(row).getPhase();
            case 4:
                return data.get(row).amplitude;
            default:
                return null;
        }
    }
    
    public WGRegisterType getRow(int row)
    {
        return data.get(row);
    }
    
    public WGRegisterType[] getRows(int[] rows)
    {
        WGRegisterType[] ret = new WGRegisterType[rows.length];
        
        for (int i = 0; i < rows.length; i++) {
            ret[i] = data.get(rows[i]);
        }
        
        return ret;
    }
    
    
    /**
     * Updates the list with the data.
     */
    public void updateList(ArrayList<WGRegisterType> newData)
    { 
        data.clear();
        data = new ArrayList<WGRegisterType>(newData);        
        fireTableChanged(new TableModelEvent(this));
    }    
}

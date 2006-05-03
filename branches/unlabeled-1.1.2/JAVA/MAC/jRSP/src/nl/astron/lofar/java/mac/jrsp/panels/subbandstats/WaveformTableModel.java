package nl.astron.lofar.mac.apl.gui.jrsp.panels.subbandstats;

import java.util.ArrayList;
import javax.swing.event.TableModelEvent;
import javax.swing.table.AbstractTableModel;
import nl.astron.lofar.mac.apl.gui.jrsp.WGRegisterType;

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

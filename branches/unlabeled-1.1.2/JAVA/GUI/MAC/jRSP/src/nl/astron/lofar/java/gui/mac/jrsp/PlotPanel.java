// tijdelijke globale plotpanel ding

package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
import gov.noaa.pmel.util.Range2D;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JPanel;

/**
 *
 * @author balken
 */
public class PlotPanel extends JPanel
{   
    /** SGT zut. */
    private JPlotLayout layout;
         
    private SGTMetaData xMetaData;
    private SGTMetaData yMetaData;
    
    private ArrayList<String> ids;
    
    /**
     * 
     * Creates a new instance of PlotPanel.
     */
    public PlotPanel() 
    {
        String title = "Subband Statistics";
                
        layout = new JPlotLayout(false, false, false, title, null, true);
        layout.setBatch(false);
        layout.setEditClasses(false);
        layout.setTitles(title, "", "");
        // the intervals on the y-axis are halved
        layout.setAutoIntervals(10, layout.getYAutoIntervals() / 2);
        
        xMetaData = new SGTMetaData("Frequency", "MHz");
        yMetaData = new SGTMetaData("y", "y-iets");
        
        ids = new ArrayList<String>();
        
        add(layout);
    }
    
    public void init(String title)
    {
        layout.setTitles(title, "", "");
    }
       
    /**
     * Adds a line, with the points stored in /data/, and /id/ as identifier.
     */
    public void addLine(double[] data, String id)
    {           
        double[] xValues = new double[data.length];
        
        // process subbandstats data
        for (int i = 0; i < data.length; i ++)
        {
            xValues[i] = i * ( 80.0 / 512);
        }
                                
        //SimpleLine line = new SimpleLine(xValues, yValues, id);
        SimpleLine line = new SimpleLine(xValues, data, id);
        line.setId(id);
        line.setXMetaData(xMetaData);
        line.setYMetaData(yMetaData);
        
        Range2D tempYRange = layout.getRange().getYRange();
        
        layout.addData(line, id);
    }
    
    /**
     * Removes the line associated with the given id.
     */
    public void removeLine(String id)
    {
        layout.clear(id);
    }
    
    public void removeAllLines()
    {
        layout.clear();
    }
}

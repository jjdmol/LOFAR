package nl.astron.lofar.mac.apl.gui.jrsp.panels.waveformsettings;

import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
import gov.noaa.pmel.util.Range2D;
import javax.swing.JPanel;

/**
 *
 * @author balken
 */
public class WaveformSettingsPlotPanel extends JPanel
{    	
    /** SGT zut. */
    private JPlotLayout layout;
       
    /** 
     * Creates a new instance of WaveformSettingsPlotPanel.
     */
    public WaveformSettingsPlotPanel() 
    {
        layout = new JPlotLayout(false, false, false, "Subband Stats", null, true);
        layout.setBatch(false);
        layout.setEditClasses(false);
        layout.setTitles("Subband Stats", "", "");
        // the intervals on the y-axis are halved
        layout.setAutoIntervals(10, layout.getYAutoIntervals() / 2);
        
        add(layout);
    }
    
    /**
     * Initializes this panel.
     */
    public void init(double[] data)
    {
        addLine(data, null);
    }
    
    /**
     * Adds a line, with the points stored in /data/, and /id/ as identifier.
     */
    public void addLine(double[] data, String id)
    {
        double[] xValues = new double[data.length];
        double[] yValues = new double[data.length];
        
        // process subbandstats data
        for (int i = 0; i < data.length; i ++)
        {
            xValues[i] = i * ( 80.0 / 512);
            yValues[i] = data[i];
        }
                                
        SimpleLine line = new SimpleLine(xValues, yValues, id);
        line.setId(id);
        line.setXMetaData(new SGTMetaData("Frequency", "MHz"));
        line.setYMetaData(new SGTMetaData("y", "y-iets"));
        
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
    
}

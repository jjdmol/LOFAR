package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
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
        layout = new JPlotLayout(false, false, false, "Een titel", null, true);
        layout.setBatch(false);
        layout.setEditClasses(false);
        layout.setTitles("Hoofd titel", "2de titel", "");        
        
        add(layout);
    }
    
    public void init(double[] data)
    {
        double[] xValues = new double[data.length / 2];
        double[] yValues = new double[data.length / 2];
        
        // process subbandstats data
        for (int i = 0; i < (data.length / 2); i ++)
        {
            xValues[i] = data[i * 2];
            yValues[i] = data[(i * 2) + 1];
        }
                                
        SimpleLine line = new SimpleLine(xValues, yValues, "Lijn");
        line.setXMetaData(new SGTMetaData("x", "x-iets", false, false));
        line.setYMetaData(new SGTMetaData("y", "y-iets", false, false));
                       
        layout.addData(line, new String("Lijn"));
    }    
}

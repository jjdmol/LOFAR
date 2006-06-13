/*
 * PlotSlotViewFrame.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package nl.astron.lofar.sas.otb.util.plotter;

import java.awt.BorderLayout;
import javax.swing.JDialog;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @version $Id$
 * @created June 1, 2006, 2:56 PM
 * @author pompert
 */
public class PlotSlotViewFrame extends JDialog{
    
    protected int plotIndex;
    
    /** Creates a new instance of PlotSlotViewFrame */
    public PlotSlotViewFrame(PlotSlotManager parent, int index, String title, boolean showLegendOnly) {
        super();
        plotIndex = index;
        this.getContentPane().setLayout(new BorderLayout());
        setTitle(title);
        PlotSlot viewSlot = parent.getSlot(index);
        if(!showLegendOnly){
            setTitle(title);
            this.getContentPane().add(viewSlot,BorderLayout.CENTER);
            pack();
        }else{
            setTitle(title);
            try {
                this.getContentPane().add(viewSlot.getPlot().getLegendForPlot(),BorderLayout.CENTER);
                pack();
            } catch (PlotterException ex) {
                ex.printStackTrace();
            }
        }
        
        this.setDefaultCloseOperation(this.DISPOSE_ON_CLOSE);
        
    }
}

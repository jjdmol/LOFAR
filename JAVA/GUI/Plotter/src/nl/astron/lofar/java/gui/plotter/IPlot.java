/*
 * IPlot.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import javax.swing.JComponent;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:45
 */
public interface IPlot{

	public static final int BAR = 1;
	public static final int XYLINE = 2;
	public static final int SCATTER = 3;
	public static final int GRID = 4;
	
	/**
	 * @param type
	 * @param name
	 * @param data
	 * 
	 */
	public JComponent createPlot(int type, String name, HashMap data);
        
        public HashMap getData();
        
        public void setData(HashMap newData);
}


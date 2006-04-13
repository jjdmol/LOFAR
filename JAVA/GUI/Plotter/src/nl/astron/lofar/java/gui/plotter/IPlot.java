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
 * @version $Id$
 * @created 11-04-2006, 15:00
 * @author pompert
 */
public interface IPlot {

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

}


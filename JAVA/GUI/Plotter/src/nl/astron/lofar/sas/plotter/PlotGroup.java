/*
 * PlotGroup.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashSet;

/**
 * @created 12-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 12-apr-2006 15:18:39
 */
public class PlotGroup{

	public HashSet<IPlot> m_IPlot;

	public PlotGroup(){

	}

	public void finalize() throws Throwable {

	}

}
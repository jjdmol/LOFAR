/*
 * ITabPanel.java
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

package nl.astron.lofar.java.gui.mac.jrsp;

/**
 * This interface declares some methods that are needed for interaction with the
 * main panel.
 *
 * @author balken
 */
public interface ITabPanel 
{
    /** Update is required because the board or data has changed. */
    public final static int REQUIRED_UPDATE = 1;
    
    /** Update is suggested by the refresh. */
    public final static int REFRESH_UPDATE = 2;
    
    /**
     * Used to initialize the ITabPanel and give it a refrence to the main panel. 
     * @param   mainPanel   The MainPanel.
     */
    public void init(MainPanel mainPanel);
    
    /**
     * Method that can be called by the main panel to update this panel.
     * @param   updateType  The type of update.
     */
    public void update(int updateType);
    
    /**
     * Method that can be called to disable or enable the board.
     * @param   b       Boolean value used to determine to enable (true) or
     *                  disable (false).
     */
    public void enablePanel(boolean b);
}

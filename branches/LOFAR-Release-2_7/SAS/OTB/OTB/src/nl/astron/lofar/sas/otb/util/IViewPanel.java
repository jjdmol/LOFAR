/*
 * IViewPanel.java
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
 */

package nl.astron.lofar.sas.otb.util;

import java.awt.Component;
import javax.swing.JPanel;
import nl.astron.lofar.sas.otb.MainFrame;

/**
 * This class provides an interface to all viewpanels
 *
 * @created 16-05-2006
 * @author coolen
 * @version $Id$
 * @updated
 */
public interface IViewPanel {
    public JPanel getInstance();
    public void setMainFrame(MainFrame aMainFrame);
    public void setContent(Object anObject);
    public void setAllEnabled(boolean enabled);
    public void enableButtons(boolean visible);
    public void setButtonsVisible(boolean visible);
    public String getShortName();
    public boolean hasPopupMenu();
    public boolean isSingleton();
    public void createPopupMenu(Component aComponent, int x, int y);
    public void popupMenuHandler(java.awt.event.ActionEvent evt);
    public void addActionListener(java.awt.event.ActionListener listener);
    public void removeActionListener(java.awt.event.ActionListener listener);
}

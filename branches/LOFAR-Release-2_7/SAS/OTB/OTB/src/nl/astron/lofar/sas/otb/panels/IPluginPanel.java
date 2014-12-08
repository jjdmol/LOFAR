/*
 * IPluginPanel.java
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

package nl.astron.lofar.sas.otb.panels;

/**
 * This class provides an interface to all Panels that should be capable of being a
 * plugin panel for the OTB framework.
 *
 * @created 16-01-2006, 16:31
 * @author blaakmeer/coolen
 * @version $Id$
 * @updated
 */
public interface IPluginPanel {
    public boolean initializePlugin(nl.astron.lofar.sas.otb.MainFrame mainframe);
    public String getFriendlyName();
    public boolean hasChanged();
    public void setChanged(boolean changed);
    public void checkChanged();
}

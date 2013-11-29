//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

package org.astron.basesim;

/**
 * Title:        Visual LofarSim
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.awt.*;
import kiwi.util.Config;

//! DataHolders are connected to WorkHolders. Should WorkHolder maintain a
//! list of DataHolder(+channel numbers)?

public class GraphWorkHolder extends Graph {

  private String className = "WorkHolder";

  public GraphWorkHolder() {
    super (null,0,0,60,20,"GraphWorkHolder"+followUp++);
    defaultSettings();
  }

  public GraphWorkHolder(GraphManager man) {
    super (man,0,0,60,20,"GraphWorkHolder"+followUp++);
    defaultSettings();
  }

  /** Since the WorkHolder is part of a composite, you cannot specify its
   *  coordinates (they'll be calculated by the composite) */
  public GraphWorkHolder(GraphManager man, String name) {
    super (man,0,0,60,20,name);
    defaultSettings();
  }

  private void defaultSettings ()
  {
    setLineColor(Color.black);
    setBackground(Color.gray);
    setBaseRoundSize(50);
  }

  public boolean canBeDrawn() {
    if (getZoomLevel() < 0.5) return false; else return true;
  }

  public Config getProperties() {
    Config p = super.getProperties();
    p.putString("Class",getClassName());
    return p;
  }

  public void setClassName(String className) { this.className = className; }
  public String getClassName() { return className; }
  public GraphDataHolder buildDataHolder(boolean input) {
    return new GraphDataHolder();
  }
}
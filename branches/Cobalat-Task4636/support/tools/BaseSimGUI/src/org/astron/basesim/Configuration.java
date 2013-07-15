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

import kiwi.io.ConfigFile;
import java.awt.*;

/**
 * Title: Class Config
 * Description: Main configuration class
 * Copyright:    Copyright (c) 2001
 * Company: ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.*;
import java.io.*;

public class Configuration extends ConfigFile {

  private Font propFont, titleFont;

  public Configuration(File configFile) {
    super(configFile);
    titleFont = new Font("times",Font.BOLD,(int)(11));
    propFont = new Font("arial",Font.PLAIN,(int)(10));
  }

  public Font getGraphTitleFont() {
    return titleFont;
  }

  public Font getGraphPropFont() {
    return propFont;
  }
}
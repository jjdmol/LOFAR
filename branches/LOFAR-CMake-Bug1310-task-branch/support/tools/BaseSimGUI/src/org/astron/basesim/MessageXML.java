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
 * Title: Class MessageEvent
 * Description: Message class for SAX (XML) related messages.
 * Copyright:    Copyright (c) 2001
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import javax.swing.*;
import javax.swing.event.*;
import java.util.*;
import java.awt.*;
import org.astron.util.*;
import org.astron.util.gui.*;

public class MessageXML extends MessageEvent {

  /** XML message icon */
  final static ImageIcon xmlIcon = new ImageIcon("images/closeAllWindows16.gif");

  private int lineNr;
  private int columnNr;

  public MessageXML(Object source, String text) {
    super(source,text);
    lineNr = -1;
    columnNr = -1;
  }
  public MessageXML(Object source, String text, int type) {
    super(source,text,type);
    lineNr = -1;
    columnNr = -1;
  }
  public MessageXML(Object source, String text, int type,
    int lineNr, int columnNr) {
    super(source,text,type);
    this.lineNr = lineNr;
    this.columnNr = columnNr;
  }
  public ImageIcon getIcon() { return xmlIcon; }

  /** Jumps to line number where error occured. */
  public boolean onClick(JNotifier notifier) {

    if (lineNr == -1) return false;

    // get the Main window object; focus on XML editor component
    // REVISIT: make this a little more readable
    Main main = (Main)notifier.getParent()
                .getParent().getParent().getParent().getParent();
    main.setFocus(Main.COMP_TEXTEDITOR);
    // linenr - 1 because parser is 0 based while jedit is 1 based (for lines).
    int offset = main.getEditor().getLineStartOffset(lineNr-1);
    main.getEditor().setCaretPosition(offset);
    return true;
  }
}
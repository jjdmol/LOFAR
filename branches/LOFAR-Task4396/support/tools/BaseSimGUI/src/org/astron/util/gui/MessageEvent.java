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

package org.astron.util.gui;

/**
 * Title: Class MessageEvent
 * Description: Event object for notify Messages
 * Copyright:    Copyright (c) 2001
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import javax.swing.*;
import java.util.*;

public class MessageEvent extends EventObject {

  public static final int UNRECOV_ERROR = 0;
  public static final int ERROR = 1;
  public static final int WARNING = 2;
  public static final int NOTIFY = 3;
  public static final int HINT = 4;

  /** The message text */
  private String _message;
  /** The level of importance of this message. Must be one of the predefined
   *  variables: UNRECOV_ERROR, ERROR, WARNING, NOTIFY (Default) or HINT. */
  private int _type;

  /** Construct a MessageEvent, specify constructing object and message text */
  public MessageEvent(Object source, String text) {
    super (source);
    _message = text;
    _type = HINT;
  }

  /** Construct a MessageEvent, specify source object, message text and type */
  public MessageEvent(Object source, String text, int type) {
    super(source);
    _message = text;
    _type = type;
  }

  /** The type of message. Should be UNRECOV_ERROR, ERROR, WARNING,
   *  NOTIFY (Default) or HINT. */
  public void setType (int type) { _type = type; }

  /** Get the type of message: UNRECOV_ERROR, ERROR, WARNING, NOTIFY or HINT */
  public int getType () { return _type; }

  /** Retreive message text */
  public String getMessage() { return _message; }

  /** (Re)set the message text */
  public void setMessage(String text) { _message = text; }

  /** Get the icon that should be displayed with this message.
   *  Returns null when not specified. */
  public ImageIcon getIcon() { return null; }

  /** The action to be taken when a user clicks on the message. Does nothing,
   *  (alwayes returns true) override this function with your own handler.
   *
   *  @param notifier The JNotifier control the users clicked in
   */
  public boolean onClick(JNotifier notifier) { return true; }

  /** String representation of this object. */
  public String toString () { return "MessageEvent: "+_message; }

  /** Returns true is this message is still valid. Invalid messaged are cleared
   *  by the viewer. */
  public boolean valid () { return true; }
}
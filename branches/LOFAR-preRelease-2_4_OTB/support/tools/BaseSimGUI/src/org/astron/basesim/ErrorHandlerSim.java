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
 * Title: Class ErrorHandlerSim<p>
 * Description: <p>
 * Copyright:    Copyright (c) <p>
 * Company: Astron<p>
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.*;
import java.io.*;
import org.xml.sax.*;
import org.astron.util.*;
import org.astron.util.gui.*;

public class ErrorHandlerSim implements ErrorHandler {

  /** List of MessageListeners */
  private ArrayList _mlisteners = new ArrayList();

  /** Document name (XML file name) */
  private String _filename = new String("Noname.xml");

  public ErrorHandlerSim() {}

  public void warning(SAXParseException exception) throws SAXException {
    String message;
    if (_filename != null) message = new String (_filename + ": '");
    else message = new String ("'");
    message = message+exception.getLocalizedMessage()+"' at line "+
      exception.getLineNumber()+", char "+exception.getColumnNumber()+".";
    fireMessage(new MessageXML(this,message,MessageEvent.NOTIFY,
      exception.getLineNumber(),exception.getColumnNumber()));
  }

  public void error(SAXParseException exception) throws SAXException {
    String message;
    if (_filename != null) message = new String (_filename + ": '");
    else message = new String ("'");
    message = message+exception.getLocalizedMessage()+"' at line "+
      exception.getLineNumber()+", char "+exception.getColumnNumber()+".";
    fireMessage(new MessageXML(this,message,MessageEvent.ERROR,
      exception.getLineNumber(),exception.getColumnNumber()));
  }

  public void fatalError(SAXParseException exception) throws SAXException {
    String message;
    if (_filename != null) message = new String (_filename + ": '");
    else message = new String ("'");
    message = message+" (fatal) "+exception.getLocalizedMessage()+"' at line "+
      exception.getLineNumber()+", char "+exception.getColumnNumber()+".";
    fireMessage(new MessageXML(this,message,MessageEvent.ERROR,
      exception.getLineNumber(),exception.getColumnNumber()));
  }

  /** (Re)sets the document name. This name will be displayed in every
   *  message the ErrorHandler sends to its MessageListeners. */
  public void changeDocumentName (String name) {
    _filename = name;
  }

  /** Register a MessageListener. All XML parser messages will be send to the
   *  listener. The messages contain extra information (e.g. line number) */
  public void addMessageListener (MessageListener messageListener) {
    _mlisteners.add(messageListener);
  }
  /** Unregisters the specified MessageListener */
  public void removeMessageListener (MessageListener messageListener) {
    _mlisteners.remove(messageListener);
  }
  /** Dispatch the specified MessageEvent to all the registered listeners. */
  protected void fireMessage(MessageEvent event) {
    if (_mlisteners.size() == 0) {
      System.out.println(event);
    } else {
      for (int i=0; i<_mlisteners.size(); i++) {
        ((MessageListener)_mlisteners.get(i)).note(event);
      }
    }
  }
}
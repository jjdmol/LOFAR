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
 * Title:        <p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */

public class GraphException extends Exception {

  /**
   * Create a new GraphException.
   *
   * @param message The error or warning message.
   * @see org.xml.sax.Parser#setLocale
   */
  public GraphException (String message) {
    super(message);
    this.exception = null;
  }

  /**
   * Create a new GraphException wrapping an existing exception.
   * GraphException
   * <p>The existing exception will be embedded in the new
   * one, and its message will become the default message for
   * the GraphException.</p>
   *
   * @param e The exception to be wrapped in a GraphException.
   */
  public GraphException (Exception e)
  {
    super();
    this.exception = e;
  }

  /**
   * Create a new GraphException from an existing exception.
   *
   * <p>The existing exception will be embedded in the new
   * one, but the new exception will have its own message.</p>
   *
   * @param message The detail message.
   * @param e The exception to be wrapped in a GraphException.
   */
  public GraphException (String message, Exception e)
  {
    super(message);
    this.exception = e;
  }

  /**
   * Return a detail message for this exception.
   *
   * <p>If there is an embedded exception, and if the GraphException
   * has no detail message of its own, this method will return
   * the detail message from the embedded exception.</p>
   *
   * @return The error or warning message.
   */
  public String getMessage ()
  {
    String message = super.getMessage();

    if (message == null && exception != null) {
      return exception.getMessage();
    } else {
      return message;
    }
  }

  /**
   * Return the embedded exception, if any.
   *
   * @return The embedded exception, or null if there is none.
   */
  public Exception getException ()
  {
    return exception;
  }

  /**
   * Override toString to pick up any embedded exception.
   *
   * @return A string representation of this exception.
   */
  public String toString ()
  {
    if (exception != null) {
      return exception.toString();
    } else {
      return super.toString();
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Internal state.
  //////////////////////////////////////////////////////////////////////

  /**
   * @serial The embedded exception if tunnelling, or null.
   */
  private Exception exception;
}
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
 * Title: LayoutController
 * Description: Basic interface for layout managers
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

/**
 * The LayoutController interface must be used to build a class that
 * can layout components in a GraphComposite. Typically a layout controller
 * will calculate the layout of a given GraphComposite at its zoom level of
 * 100%. It will use the getBase...() and setBase...() methods for this purpose.
 */
public interface LayoutController {
  /**
   * Set the GraphComposite on which the manager will operate
   */
  public void setGraphComposite (GraphComposite container);
  /**
   * This method indicates wether the preconditions of the layout algoritm
   * can be met. Simple layout (like centering, flowing) should just return
   * true
   */
  public boolean canBeDone();
  /**
   * Preprocess layout specific data. This should be all calculations that can
   * be done independent of the dimensions of the GraphComposite's children.
   * The preprocess method should not be used to alter dimensions or coordinates.
   * This method was added so other layout managers can do a layout based on
   * results of another layout algoritm.
   */
  public void preprocess ();
  /**
   * Layout the Graph in this composite using a composition
   */
  public void doLayout ();
}
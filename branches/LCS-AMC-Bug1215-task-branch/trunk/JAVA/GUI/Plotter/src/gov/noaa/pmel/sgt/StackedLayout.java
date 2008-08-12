/*
 * $Id$
 *
 * This software is provided by NOAA for full, free and open release.  It is
 * understood by the recipient/user that NOAA assumes no liability for any
 * errors contained in the code.  Although this software is released without
 * conditions or restrictions in its use, it is expected that appropriate
 * credit be given to its author and to the National Oceanic and Atmospheric
 * Administration should the software be included by the recipient as an
 * element in other product development.
 */

package  gov.noaa.pmel.sgt;

import gov.noaa.pmel.sgt.beans.Panel;
import java.awt.LayoutManager;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Component;
import java.awt.Insets;

/**
 * <code>StackedLayout</code> works with <code>Pane</code> to
 * position multiple <code>Layer</code>s directly over each other.
 *
 * @author Donald Denbo
 * @version $Revision$, $Date$
 * @since 1.0
 * @see Pane#setLayout
 * @see Layer
 */
public class StackedLayout implements LayoutManager {
  public Dimension preferredLayoutSize(Container parent) {
    synchronized (parent.getTreeLock()) {
      return parent.getSize();
    }
  }
  public Dimension minimumLayoutSize(Container parent) {
    synchronized (parent.getTreeLock()) {
      return parent.getSize();
    }
  }
  public void layoutContainer(Container parent) {
    synchronized (parent.getTreeLock()) {
      JPane pane = null;
      boolean batch = false;
      if(parent instanceof JPane) {
        pane = (JPane)parent;
        batch = pane.isBatch();
        pane.setBatch(true, "StackedLayout");
      } else if(parent instanceof Panel) {
        pane = ((Panel)parent).getPane();
        batch = pane.isBatch();
        pane.setBatch(true, "StackedLayout");
      }
      Insets insets = parent.getInsets();
      Rectangle rect = parent.getBounds();
      int ncomponents = parent.getComponentCount();
      int x, y, w, h;
      x = rect.x + insets.left;
      y = rect.y + insets.top;
      w = rect.width - (insets.left + insets.right);
      h = rect.height - (insets.top + insets.bottom);
      for(int i=0; i < ncomponents; i++) {
        parent.getComponent(i).setBounds(x, y, w, h);
      }
      if(!batch) pane.setBatch(false, "StackedLayout");
    }
  }
  public void removeLayoutComponent(Component comp) {
  }
  public void addLayoutComponent(String name, Component comp) {
  }
}

package org.astron.basesim.def;

import org.astron.basesim.GraphDataHolder;
import java.awt.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class DHEmpty extends GraphDataHolder {

  public DHEmpty() {
    super();
    setBackground(Color.white);
  }

  /** Paint this Empty DataHolder. Overloaded from Graph. */
  protected void paintGraph(Graphics2D g, Point abs) {
    g.drawLine(abs.x,abs.y,abs.x+width,abs.y+height);
    g.drawLine(abs.x,abs.y+height,abs.x+width,abs.y);
  }

  public String getClassName() { return "DH_Empty"; }
}
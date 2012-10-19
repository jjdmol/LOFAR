package org.astron.basesim;

import java.awt.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class DimCtrlStepCenter extends DimensionController {

  public static int FIXED_WIDTH = 215;
  public static int FIXED_HEIGHT = 136;

  public DimCtrlStepCenter() {}

  /** Calculates the dimensions of the GraphComposite. Informs the GraphManager
   *  on the size change. */
  public void setDimensions() {
    gc.setBaseSize(new Dimension(FIXED_WIDTH,FIXED_HEIGHT));
    gc.getGraphManager().setDimensionsChanged(); // inform manager about size
                                                 // change
  }
}
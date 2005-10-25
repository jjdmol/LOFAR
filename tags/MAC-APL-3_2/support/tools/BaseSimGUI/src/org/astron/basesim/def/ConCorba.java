package org.astron.basesim.def;

import org.astron.basesim.*;
import java.awt.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

public class ConCorba extends Connection {
  public ConCorba() {
    super();
    init();
  }
  public ConCorba(Graph graphOutput, Graph graphInput) {
    super(graphOutput,graphInput);
    init();
  }
  private void init() {
    setColor(Color.blue.darker());
    setStroke(SIMPLE_DOTTED_STROKE);
    setStrokeSelected(THICK_DOTTED_STROKE);
  }
}
package org.astron.basesim.def;

import org.astron.basesim.*;
import java.awt.*;

/** The equivalent for the TH_Mem class in BaseSim. */
public class ConMem extends Connection {

  public ConMem() {
    super();
    init();
  }
  public ConMem(Graph graphOutput, Graph graphInput) {
    super(graphOutput,graphInput);
    init();
  }

  private void init() {
    setStroke(SIMPLE_DASH_STROKE);
    setStrokeSelected(THICK_DASH_STROKE);
    setColor(Color.gray);
  }
}
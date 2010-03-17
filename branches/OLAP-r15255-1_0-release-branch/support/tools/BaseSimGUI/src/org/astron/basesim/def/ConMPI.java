package org.astron.basesim.def;

import org.astron.basesim.*;
import java.awt.*;

/** The equivalent for the TH_MPI class in BaseSim. */
public class ConMPI extends Connection {
  public ConMPI() {
    super();
    init();
  }
  public ConMPI(Graph graphOutput, Graph graphInput) {
    super(graphOutput,graphInput);
    init();
  }
  private void init() {
    setColor(Color.blue);
    setStroke(SIMPLE_DASH_STROKE);
    setStrokeSelected(THICK_DASH_STROKE);
  }
}
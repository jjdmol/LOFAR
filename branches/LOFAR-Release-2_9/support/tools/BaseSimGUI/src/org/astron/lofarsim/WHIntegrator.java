package org.astron.lofarsim;

import org.astron.basesim.GraphWorkHolderImage;
import org.astron.basesim.*;
import org.astron.basesim.def.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

public class WHIntegrator extends GraphWorkHolderImage {

  public WHIntegrator() {
    super();
  }

  public String getClassName() { return "Integrator"; }
  public GraphDataHolder buildDataHolder(boolean input) {
      return new DHCorr();
  }

}
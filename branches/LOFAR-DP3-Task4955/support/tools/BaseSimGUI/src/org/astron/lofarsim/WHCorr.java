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

public class WHCorr extends GraphWorkHolderImage {

  public WHCorr() {
    super();
  }

  public String getClassName() { return "WHCorr"; }
  public GraphDataHolder buildDataHolder(boolean input) {
    if (input) {
      return new DHBeamBand();
    } else {
      return new DHCorr();
    }
  }
}
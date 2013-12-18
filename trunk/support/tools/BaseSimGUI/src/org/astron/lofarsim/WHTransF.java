package org.astron.lofarsim;

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

public class WHTransF extends GraphWorkHolderImage {

  public WHTransF() {
    super();
  }

  public String getClassName() { return "TransF"; }
  public GraphDataHolder buildDataHolder(boolean input) {
    if (input) {
      return new DHFreq();
    } else {
      return new DHFreqT();
    }
  }
}
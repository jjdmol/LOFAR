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

public class WHDataProc extends GraphWorkHolderImage {

  public WHDataProc() {
    super();
  }

  public String getClassName() { return "Data processor"; }
  public GraphDataHolder buildDataHolder(boolean input) {
    if (input) {
      return new DHBeam();
    } else {
      return new DHCorr();
    }
  }

}
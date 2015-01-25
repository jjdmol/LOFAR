package org.astron.lofarsim;

import java.awt.*;

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

public class WHBeam extends GraphWorkHolderImage {

  public WHBeam() {
    super();
    setBaseSize(new Dimension(60,5));
  }
  public String getClassName() { return "Beam"; }
  public GraphDataHolder buildDataHolder(boolean input) {
    if (input) {
      return new DHFreqT();
    } else {
      return new DHBeamT();
    }
  }
}
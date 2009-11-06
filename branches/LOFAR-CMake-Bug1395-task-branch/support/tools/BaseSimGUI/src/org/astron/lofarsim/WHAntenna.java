package org.astron.lofarsim;

import org.astron.basesim.*;
import java.awt.Toolkit;
import java.awt.*;
import org.astron.basesim.def.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company: ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

public class WHAntenna extends GraphWorkHolderImage {

  final static Image iWHAntenna =
    Toolkit.getDefaultToolkit().getImage("images/Antenna01.gif");

  public WHAntenna() {
    super();
    setBaseRoundSize(2);
    setImage(iWHAntenna);
  }

  public String getClassName() { return "Antenna"; }
  public GraphDataHolder buildDataHolder(boolean input) { return new DHAntenna(); }
}
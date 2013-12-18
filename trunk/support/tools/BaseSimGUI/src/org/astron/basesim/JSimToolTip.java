package org.astron.basesim;

import javax.swing.*;
import org.astron.util.gui.*;
import java.util.*;
import kiwi.util.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

public class JSimToolTip extends JMultiLineToolTip {

  Graph graph;

  public JSimToolTip(Graph graph) {
    super();
    this.graph = graph;
  }
  public String getTipText() {
    if (graph == null) return "";
    String text = "";
    Config p = graph.getProperties();
    Enumeration enum = p.list();
    for (int i=0; i<p.size(); i++) {
        String key = (String)enum.nextElement();
        String value = p.getProperty(key);
        text += key + "=" + value;
        if (i != p.size() - 1) text += '\n';
    }
    return text;
  }
}
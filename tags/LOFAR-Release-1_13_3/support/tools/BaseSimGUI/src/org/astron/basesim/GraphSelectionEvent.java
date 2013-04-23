package org.astron.basesim;

import java.util.EventObject;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class GraphSelectionEvent extends EventObject {

  boolean added;
  Graph graph;

  public GraphSelectionEvent(Object source, Graph graph, boolean added) {
    super(source);
    this.added = added;
    this.graph = graph;
  }

  public boolean isSelected() { return added; }
  public Graph getGraph() { return graph; }
}
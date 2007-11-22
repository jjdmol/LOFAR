package org.astron.basesim;

/**
 * Title: DimensionController
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company: ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

import java.awt.*;

/** The DimensionController class controls the dimensions of a registered
 *  GraphComposite. The default implementation inflates the GraphComposite so
 *  that its children just fit. */
public class DimensionController {

  int vertSpacing;
  int horzSpacing;
  float sFactorH = (float)0.25;
  float sFactorV = (float)0.25;
  GraphComposite gc;

  public DimensionController() {}

  /** Copy constructor */
  public DimensionController(DimensionController d) {
    vertSpacing = d.getVerticalSpacing();
    horzSpacing = d.getHorizontalSpacing();
    sFactorH = d.getSpacingFactor(true);
    sFactorV = d.getSpacingFactor(false);
    gc = d.getGraphComposite();
  }

  public void setGraphComposite(GraphComposite gc) {
    this.gc = gc;
  }

  /** Set the spacing between components in pixels at 100% zoom level. */
  public void setSpacing (int horizontal, int vertical) {
    horzSpacing = horizontal;
    vertSpacing = vertical;
  }

  public void setHorizontalSpacing(int spacing) { horzSpacing = spacing; }
  public void setVerticalSpacing(int spacing) { vertSpacing = spacing; }

  /** Get the base vertical spacing. This is the vertical spacing when the
   *  zoom level equals 1. This method is called by auto-layout managers. */
  public int getVerticalSpacing() { return vertSpacing; }

  /** Get the base horizontal spacing. This is the horizontal spacing when the
   *  zoom level equals 1. This method is called by auto-layout managers. */
  public int getHorizontalSpacing() { return horzSpacing; }

  public void setSpacingFactor(float factor, boolean horizontal) {
    if (horizontal) {
      sFactorH = factor;
    } else {
      sFactorV = factor;
    }
  }
  public float getSpacingFactor(boolean horizontal) {
    if (horizontal) {
      return sFactorH;
    } else {
      return sFactorV;
    }
  }

  public GraphComposite getGraphComposite() { return gc; }

  /** Calculates the dimensions of the GraphComposite. Informs the GraphManager
   *  on the size change. */
  public void setDimensions() {
    int maxWidth=0,maxHeight=0,temp;
    for (int i=0; i<gc.numberOfGraphs(); i++) {
      Graph graph = gc.getGraph(i);
      if (graph.isVisible()) {
        if ((temp=graph.getBaseX()+graph.getBaseWidth()) > maxWidth) {
          maxWidth=temp;
        }
        if ((temp=graph.getBaseY()+graph.getBaseHeight()) > maxHeight) {
          maxHeight=temp;
        }
      }
    }
    maxWidth += (int)(getHorizontalSpacing() * getSpacingFactor(true));
    maxHeight += (int)(getVerticalSpacing() * getSpacingFactor(false));
    gc.setBaseSize(new Dimension(maxWidth,maxHeight));
    gc.getGraphManager().setDimensionsChanged(); // inform manager about size
                                                 // change
  }
}
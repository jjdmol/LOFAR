package org.astron.basesim;

import java.awt.*;
import java.awt.image.*;
import javax.swing.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company: ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

/** The GraphWorkHolderImage is the base class for all WorkHolder that
 *  are contained within Simuls. These are only visible when the Simul
 *  is not expanded (it does not draw its children). These objects are not
 *  part of the composite tree, but their drawing code will be called by
 *  their related Simuls. */
public class GraphWorkHolderImage extends GraphWorkHolder {

  final static int INDENT = 5;

  private static int followUp = 100;
  private static MediaTracker mt = null;
  protected Image image = null;

  public GraphWorkHolderImage() {
    super ();
    defaultSettings();
  }

  public GraphWorkHolderImage(GraphManager man) {
    super (man);
    defaultSettings();
  }

  public GraphWorkHolderImage(GraphManager man, String name) {
    super (man,name);
    defaultSettings();
  }

  private void defaultSettings() {
    setBaseSize(new Dimension(DimCtrlStepCenter.FIXED_WIDTH,
                              DimCtrlStepCenter.FIXED_HEIGHT));
  }

  /** Paint this WorkHolder. A Simul workholder displays an icon, a title,
   *  and some other interesting properties depending on the type (class) of
   *  WorkHolder */
  public void paintGraph(Graphics2D g, Point abs) {
    if (image == null) {
      String text = getClassName();
      if (text == null) text = getName();
      Font font = new Font("times",Font.BOLD,(int)(11));
      FontMetrics fontMetr = g.getFontMetrics(font);
      g.setFont(font);
      int textWidth = fontMetr.stringWidth(text);
      if (textWidth >= width) return;
      int xpos = (width/2)-(textWidth/2)+abs.x;
      int ypos = (height/2) + abs.y + (int)(0.5*fontMetr.getMaxAscent());
      g.drawString(text,xpos,ypos);
      g.draw(new Rectangle(xpos-5,ypos-14,textWidth+10,20));

      // REVISIT: fix this method
      //paintProperties(g,getClassName(),new Rectangle(abs.x,abs.y,width,height));
    } else {
      g.drawImage(image,abs.x+INDENT,abs.y+INDENT,
                  width-INDENT*2,height-INDENT*2,
                  (Color)getBackground(),null);
      g.drawRect(abs.x+INDENT,abs.y+INDENT,width-INDENT*2,height-INDENT*2);
    }
  }

  /** Determines the space this Graph object needs to visualize itself
   *  properly*/
  public int getPreferredWidth() {
    return -1;
  }

  /** Determines the space this Graph object needs to visualize itself
   *  properly*/
  public int getPreferredHeight() {
    return -1;
  }

  public void setImage(Image image) {
    this.image = image;
    if (mt != null) {
      mt.addImage(image,followUp++);
    }
  }

  /** Set a MediaTracker. */
  static public void setMediaTracker(MediaTracker mediaTracker) {
    mt = mediaTracker;
  }

  static public MediaTracker getMediaTracker() { return mt; }
}
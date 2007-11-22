package org.astron.util.gui;

import javax.swing.JToggleButton;
import java.awt.event.*;
import javax.swing.border.*;
import java.awt.*;
import javax.swing.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

public class JToggleButtonEx extends JToggleButton
                              implements MouseListener {
  private Border emptyBorder;
  private Border etchedBorder;
  private Border selBorder;

  public JToggleButtonEx() {
    super();
  }
  public JToggleButtonEx(Action a) {
    super(a);
    init();
  }
  public JToggleButtonEx(Icon icon) {
    super(icon);
    init();
  }
  public JToggleButtonEx(Icon icon, boolean selected) {
    super(icon,selected);
    init();
  }
  public JToggleButtonEx(String text) {
    super(text);
    init();
  }
  public JToggleButtonEx(String text, boolean selected) {
    super(text,selected);
    init();
  }
  public JToggleButtonEx(String text, Icon icon) {
    super(text,icon);
    init();
  }
  public JToggleButtonEx(String text, Icon icon, boolean selected) {
    super(text,icon,selected);
    init();
  }
  private void init() {
    emptyBorder = BorderFactory.createEmptyBorder();
    etchedBorder = BorderFactory.createEtchedBorder();
    selBorder = BorderFactory.createBevelBorder(1);
    setBorder(emptyBorder);
    addMouseListener(this);
    Dimension d = new Dimension(34,30);
    setPreferredSize(d);
    setMaximumSize(d);
  }
  public void mouseClicked(MouseEvent e) {}
  public void mousePressed(MouseEvent e) {}
  public void mouseReleased(MouseEvent e) {}
  public void mouseEntered(MouseEvent e) {
    if (!isEnabled()) {
      setBorder(emptyBorder);
      return;
    }
    setBorder(etchedBorder);
  }
  public void mouseExited(MouseEvent e) {
    setBorder(emptyBorder);
  }
}

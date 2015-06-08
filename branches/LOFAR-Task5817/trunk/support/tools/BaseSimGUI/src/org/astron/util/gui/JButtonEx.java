package org.astron.util.gui;

import javax.swing.JButton;
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

public class JButtonEx extends JButton
                       implements MouseListener {
  private Border emptyBorder;
  private Border etchedBorder;

  public JButtonEx() {
    super();
  }

  public JButtonEx(Action a) {
    super(a);
    init();
  }
  public JButtonEx(Icon icon) {
    super(icon);
    init();
  }
  public JButtonEx(String text) {
    super(text);
    init();
  }
  public JButtonEx(String text, Icon icon) {
    super(text,icon);
    init();
  }
  private void init() {
    emptyBorder = BorderFactory.createEmptyBorder();
    etchedBorder = BorderFactory.createEtchedBorder();
    setBorder(emptyBorder);
    addMouseListener(this);
    Dimension d = new Dimension(34,26);
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
package org.astron.util.gui;

import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import javax.swing.plaf.basic.*;
import java.util.*;

/**
 * Title:        LOFARSim
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

/** A JTree that changes the default white background of a JTree to an
 *  bitmapped background. */
public class JTreeEx extends JTree {

  public JTreeEx() {
    super();
  }
  public JTreeEx(Hashtable value) {
    super(value);
  }
  public JTreeEx(Object[] value) {
    super(value);
  }
  public JTreeEx(TreeModel newModel) {
    super(newModel);
  }
  public JTreeEx(TreeNode root) {
    super(root);
  }
  public JTreeEx(TreeNode root, boolean asksAllowsChildren) {
    super(root,asksAllowsChildren);
  }
  public JTreeEx(Vector value) {
    super(value);
  }

  public void setBackground(Image i) {
    setUI(new MyTreeUI(i));
  }

  private class MyTreeUI extends javax.swing.plaf.basic.BasicTreeUI {
    private Image bkgImg = null;

    public MyTreeUI(Image i) {
      bkgImg = i;
    }

    public void paint(Graphics g, JComponent c) {
      if (bkgImg != null) {
        g.drawImage(bkgImg, 0, 0, null);
      }
      //let the superclass handle the rest
      super.paint(g, c);
    }
  }
}

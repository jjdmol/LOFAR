package org.astron.util.gui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.JFrame;

/**
 * Title:        LOFARSim
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

/** Test for JTreeEx. Needs an image: image/tb1.jpg */
public class TestJTreeEx extends JFrame {
  public TestJTreeEx() {
    getContentPane().setLayout(new BorderLayout());
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent we) {
        System.exit(0);
      }
    });
    //get the image
    Image i = Toolkit.getDefaultToolkit().createImage("images/tb1.jpg");
    //make the tree
    JTreeEx bkgTree = new JTreeEx();
    bkgTree.setBackground(i);
    getContentPane().add(bkgTree, BorderLayout.CENTER);
    setSize(300,300);
    setVisible(true);
  }

  public static void main(String[] args) {
    new TestJTreeEx();
  }
}

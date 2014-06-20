package org.astron.basesim;

import org.astron.util.gui.*;
import javax.swing.*;
import javax.swing.event.*;
import java.util.*;
import java.awt.*;

/** Displays exception messages in a message box. */
public class MessageApp extends MessageEvent {

  final static boolean to_stdout = Main.SETTINGS.getBoolean("enableStdout");

  /** XML message icon */
  final static ImageIcon appIcon = new ImageIcon("images/4.gif");

  public MessageApp(Exception e) {
    super((Object)e,e.toString());
    if (to_stdout) e.printStackTrace();
  }
  public MessageApp(Exception e, String text) {
    super((Object)e,text);
    if (to_stdout) e.printStackTrace();
  }

  public ImageIcon getIcon() { return appIcon; }

  /** Displays exception message in message box. */
  public boolean onClick(JNotifier notifier) {
    Exception e = (Exception)getSource();
    // REVISIT: make this a little more readable
    Main main = (Main)notifier.getParent()
                .getParent().getParent().getParent().getParent();
    JOptionPane.showMessageDialog(main,e.toString(),"Program exception",
                                    JOptionPane.ERROR_MESSAGE);
    //REVISIT: add 'show stack trace' button
    return true;
  }
}
/*
 * Viewer.java
 *
 * Created on November 8, 2005, 2:39 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package bb_gui;

/**
 *
 * @author coolen
 */
/* 
 * Viewer.java
 */

import java.awt.*;
import java.awt.event.*;
import javax.swing.JFrame;


public class Viewer extends JFrame {
        private Image image;
        private static boolean mainRun=false;

        public Viewer(String fileName) {
                Toolkit toolkit = Toolkit.getDefaultToolkit();
                image = toolkit.getImage(fileName);
                MediaTracker mediaTracker = new MediaTracker(this);
                mediaTracker.addImage(image, 0);
                try
                {
                        mediaTracker.waitForID(0);
                }
                catch (InterruptedException ie)
                {
                        System.err.println(ie);
                        System.exit(1);
                }
                addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    if (mainRun) {
                        System.exit(0);
                    }  else {
                        setVisible(false);
                        dispose();
                    }
                }
                });

                setSize(image.getWidth(null), image.getHeight(null));
                setTitle(fileName);
                show();
        }

        public void paint(Graphics graphics) {
                graphics.drawImage(image, 0, 0, null);
        }

        public static void main(String[] args) {
                mainRun=true;
                new Viewer(args[0]);
        }
}
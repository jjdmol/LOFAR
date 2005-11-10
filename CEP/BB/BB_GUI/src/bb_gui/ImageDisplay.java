/*
 * ImageDisplay.java
 *
 * Created on October 14, 2005, 3:34 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package bb_gui;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import javax.swing.JComponent;

/**
 *
 * @author coolen
 */


class ImageDisplay extends JComponent {
    BufferedImage image;
    float ratio = 0;
    int myWidth = 0, myHeight = 0;
    public ImageDisplay(BufferedImage bi) {
        myWidth = bi.getWidth();
        myHeight = bi.getHeight();
        image = bi;

        this.setPreferredSize(new Dimension(myWidth, myHeight));
        this.setMaximumSize(new Dimension(myWidth, myHeight));
        this.setMinimumSize(new Dimension(myWidth, myHeight));
        this.setDoubleBuffered(true); 
        revalidate();
    }
    protected void paintComponent(Graphics g) {
        Graphics2D g2d = (Graphics2D)g.create();
        g2d.drawImage(image, 0, 0, this);
        g2d.dispose(); //clean up
    }
}
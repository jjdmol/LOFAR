/**
 * SwingEx Project
 *
 * This class is part of the Swing Extension Project. The goal of this project
 * is to provide Swing components with extra functionality that isn't available
 * in the standard Swing components from Sun.
 *
 * @author Jeroen Zwartepoorte (Jeroen@xs4all.nl)
 */
package org.astron.util.gui;

import java.awt.*;
import javax.swing.*;

/**
 * This component contains extended functionality that JScrollPane doesn't have.
 * It's written so that you only have to change your JScrollPane declaration from
 * JList to JScrollPaneEx. The rest of the changes are transparent. These are the
 * extra functionalities: <BR>
 * <UL>
 * <LI><B>Background image:</B> You can change the background of the component
 * from a dull white color to a nice image. The image doesn't scroll when you
 * scroll the component(s) inside the JScrollPaneEx. That functionality lies
 * with the component inside the scrollpane.</LI></UL>
 *
 * <B>Notes :</B> Any suggestions on extra functionality are welcome!
 *
 * @author Jeroen Zwartepoorte (<A HREF="mailto:Jeroen@xs4all.nl">Jeroen@xs4all.nl</A>)
 * @author used source by Zafir Anjum (<A HREF="http://www.codeguru.com/java/articles/181.shtml">www.codeguru.com/java/articles/181.shtml</A>)
 * @version 1.0 (13-02-1999)
 */
public class JScrollPaneEx extends JScrollPane
{
	// Private variables.
    private ImageIcon ImBackground = null;

    /**
     * @see javax.swing.JScrollPane#JScrollPane
     */
	public JScrollPaneEx()
	{
    	super();
	}

    /**
     * @see javax.swing.JScrollPane#JScrollPane(java.awt.Component)
     */
    public JScrollPaneEx(Component view)
    {
    	super(view);
    }

    /**
     * @see javax.swing.JScrollPane#JScrollPane(java.awt.Component,int,int)
     */
    public JScrollPaneEx(Component view, int vsbPolicy, int hsbPolicy)
    {
    	super(view, vsbPolicy, hsbPolicy);
    }

    /**
     * @see javax.swing.JScrollPane#JScrollPane(int, int)
     */
    public JScrollPaneEx(int vsbPolicy, int hsbPolicy)
    {
    	super(vsbPolicy, hsbPolicy);
    }

    /**
     * This sets the background image. We need to repaint the component after
     * this property is set.
     */
    public void setBackgroundImage(ImageIcon ImBackground)
    {
    	this.ImBackground = ImBackground;
        repaint();
    }

    /**
     * This returns the background image currently used for painting the
     * background.
     */
    public ImageIcon getBackgroundImage()
    {
    	return ImBackground;
    }

    /**
     * This overrides JScrollPane.paint(Graphics g). We first need to draw the
     * tiled background image. After that, we can call super.paint(g) to paint
     * the JScrollPane.
     */
    public void paintComponent(Graphics g)
    {
    	if (ImBackground != null)
        {
           	// Make sure image is loaded.
           	if ((ImBackground.getIconWidth() == -1) ||
                (ImBackground.getIconHeight() == -1))
            {
            	super.paint(g);
                return;
            }

            // Tile image.
            Rectangle rect = getViewport().getViewRect();
            for (int x = 0; x < rect.width; x += ImBackground.getIconWidth())
            	for (int y = 0; y < rect.height; y += ImBackground.getIconHeight())
                	g.drawImage(ImBackground.getImage(), x, y, null, null);

            // Do not use cached image for scrolling.
            getViewport().setScrollMode(JViewport.SIMPLE_SCROLL_MODE);

            Rectangle r = g.getClipBounds();
          g.setColor(Color.red);
          g.fillRect(r.x, r.y, r.width, r.height);
          System.out.println("painted!");
        }
        //super.paint(g);
    }
}
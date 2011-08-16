/*
 * RoundButton.java
 *
 * Created on 12 maart 2009, 12:54
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Shape;
import java.awt.geom.Ellipse2D;
import javax.swing.JButton;

/**
 *
 * @author  Coolen
 */
/**
 * RoundButton - a class that produces a lightweight button.
 *
 * Lightweight components can have "transparent" areas, meaning that
 * you can see the backgtest of the container behind these areas.
 *
 */
public class RoundButton extends JButton {

  Shape shape;

  /**
   * Constructs a RoundButton with no label.
   */
  public RoundButton() {
      super("");
      init();
  }

  /**
   * Constructs a RoundButton with the specified label.
   * @param label the label of the button
   */
  public RoundButton(String label) {
      super(label);
      init();
  }


  void init() {
    // These statements enlarge the button so that it
    // becomes a circle rather than an oval.
    Dimension size = getPreferredSize();
    size.width = size.height = Math.max(size.width,size.height);
    setPreferredSize(size);

    // This call causes the JButton not to paint he background.
    // This allows us to paint a round background.
    setContentAreaFilled(false);
  }

  /**
   * Paint the round background and label.
   */
  @Override
  protected void paintComponent(Graphics g) {
  	if (getModel().isArmed()) {
        // You might want to make the highlight color
    	// a property of the RoundButton class.
    	g.setColor(Color.lightGray);
    } else {
        g.setColor(getBackground());
    }
    int s = Math.min(getSize().width - 1, getSize().height - 1);
    g.fillArc(0, 0, s, s, 0, 360);

    // This call will paint the label and the focus rectangle.
    super.paintComponent(g);
  }

  /**
   *  Paint the border of the button using a simple stroke.
   */
  @Override
  protected void paintBorder(Graphics g) {
    int s = Math.min(getSize().width - 1, getSize().height - 1);
    g.setColor(getForeground());
    g.drawArc(0, 0, s, s, 0, 360 );
  }


  /**
   *  Hit detection.
   */
  @Override
  public boolean contains(int x, int y) {
	// If the button has changed size,
   	// make a new shape object.
    if (shape == null || !shape.getBounds().equals(getBounds())) {
      shape = new Ellipse2D.Float(0, 0, getWidth(), getHeight());
    }
    return shape.contains(x, y);
  }
}




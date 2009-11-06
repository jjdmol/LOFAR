//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
// zxzss       Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

package org.astron.basesim;

/**
 * Title:        <p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      ASTRON<p>
 * @author Robbert Dam
 * @version 1.0
 */

import org.xml.sax.*;
import java.util.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.io.*;
import org.astron.util.gui.*;

/**
 * Java swing component that can display a single Graph or a complex
 * hierarchy of GraphComposites.
 */
public class JBaseSim extends JComponent implements ComponentListener,
                                                    MouseInputListener,
                                                    KeyListener,
                                                    Scrollable {

  //-- constants
  final static int MODUS_ZOOM = 0;
  final static int MODUS_PAN = 1;
  final static int MODUS_SELECT = 2;

  //-- Mouse cursor shapes
  final static Cursor CURSOR_SELECT = new Cursor(Cursor.DEFAULT_CURSOR);
  Cursor CURSOR_PAN = new Cursor(Cursor.HAND_CURSOR);
  Cursor CURSOR_ZOOM = new Cursor(Cursor.DEFAULT_CURSOR);

  //-- misc
  private GraphComposite diagram;
  private int _mode;
  private double _zoomStep;
  private double _maxZoomLevel;
  private Stack zoomStack;
  private Graph focusGraph;
  private MouseEvent lastMove;
  private JToolTip currentTip = null;

  //-- popup menu actions
  AbstractAction policyCenterSteps = new ActionCenterSteps();
  AbstractAction showChildren = new ActionShowChildren();

  public JBaseSim () {
    diagram = null;
    defaultSettings();
  }

  public JBaseSim(GraphComposite diagram) {
    setGraph(diagram);
    defaultSettings();
  }

  private void defaultSettings() {
    addMouseListener (this);
    addMouseMotionListener (this);
    addKeyListener(this);
    // set behavior of tooltips
    setToolTipText("Hi!");
    ToolTipManager.sharedInstance()
      .setDismissDelay(Main.SETTINGS.getInt("toolTipDismissDelay"));
    ToolTipManager.sharedInstance()
      .setReshowDelay(Main.SETTINGS.getInt("toolTipReshowDelay"));

    loadCursors();
  }

  private void loadCursors() {
    Toolkit defToolkit = Toolkit.getDefaultToolkit();
    Image panImage = defToolkit.getImage("images/mpan.gif");
    Image zoomImage = defToolkit.getImage("images/mzoom.gif");
    CURSOR_PAN = defToolkit.createCustomCursor(panImage,
                                               new Point(0,0), // hotspot
                                               "panCursor");
    CURSOR_ZOOM = defToolkit.createCustomCursor(zoomImage,
                                                new Point(0,0), // hotspot
                                                "zoomCursor");
  }

  private void initialize() {
    setBorder(BorderFactory.createLoweredBevelBorder());
    _mode = MODUS_SELECT;
    _zoomStep = 3;
    _maxZoomLevel = 10;
  }

  public void clearDiagram() {
    diagram = null;
    repaint();
  }

  public void setMode(int mode) {
    _mode = mode;
    switch (_mode) {
      case MODUS_PAN:
        setAutoscrolls(true);
        setCursor(CURSOR_PAN);
        break;
      case MODUS_SELECT:
        setCursor(CURSOR_SELECT);
        break;
      case MODUS_ZOOM:
        setCursor(CURSOR_ZOOM);
        break;
    }
  }
  public int getMode() { return _mode; }
  public void setZoomStepsize(double stepsize) { _zoomStep = stepsize; }
  public double getZoomStepSize() { return _zoomStep; }
  public void setMaxZoomLevel(double level) { _maxZoomLevel = level; }
  public double getMaxZoomLevel() { return _maxZoomLevel; }

  /** Resizes to diagram size and returns new component size. If the diagram
   *  size is smaller then the parent size in which this component is
   *  contained, then this method will center the diagram in the parent
   *  component.*/
  private Dimension recalcDimension() {
    Dimension r = new Dimension(getParent().getSize());
    if (diagram != null) {
      if (diagram.getWidth() > r.getWidth()) {
        r.width = diagram.getWidth();
      }
      if (diagram.getHeight() > r.getHeight()) {
        r.height = diagram.getHeight();
      }
    }
    setPreferredSize(r);
    revalidate();
    return r;
  }

  /** Lets the displayed diagram just fit the viewport. This method changed the
   *  zoom level of the diagram. */
  public void fitDiagram() {

    JViewport viewport;
    if (getParent() instanceof JViewport) {
      viewport = (JViewport)getParent();
    } else {
      System.out.println("A JBaseSim is expected to be within a JScrollPane!");
      return;
    }
    Dimension d = viewport.getSize();
    diagram.setZoomLevel(diagram.getZoomLevel()); // make sure dim. are current
    float newZoomLevel;
    if (diagram.getBaseWidth() > diagram.getBaseHeight()) {
      // adjust to width
      newZoomLevel = (float)(d.width-50) / diagram.getBaseWidth();
    } else {
      // adjust to height
      newZoomLevel = (float)(d.height-50) / diagram.getBaseHeight();
    }
    diagram.setZoomLevel(newZoomLevel);
    recalcDimension();
  }

  private JPopupMenu createPopupMenu(Graph graph) {

    if (graph instanceof GraphDiagram) return null;

    JPopupMenu popup = new JPopupMenu();
    JMenuItem labelItem = new JMenuItem();
    labelItem.setText(graph.getName());
    labelItem.setEnabled(false);
    labelItem.setBackground(Color.blue);
    popup.add(labelItem);

    if (graph instanceof GraphSimul) {
      JMenuItem showChildrenItem = new JMenuItem(showChildren);
      if (((GraphSimul)graph).numberOfSteps() == 0) {
        showChildrenItem.setEnabled(false);
      }
      if (((GraphSimul)graph).getState() == GraphSimul.STATE_INDEPENDENT) {
        showChildren.putValue(AbstractAction.NAME,"Expand");
      } else {
        showChildren.putValue(AbstractAction.NAME,"Collapse");
      }
      popup.addSeparator();
      popup.add(showChildrenItem);
    }
    focusGraph = graph;
    return popup;
  }

  public boolean parseSimul (InputStream input, MessageListener listener) {
    return parseSimul(input,listener,null);
  }

  /** Parse the specified XML file containing the simulation. */
  public boolean parseSimul (InputStream input,
                             MessageListener listener,
                             String docName) {
    Date date1 = new Date();
    ContHandlerSim contentHandler = new ContHandlerSim();
    ErrorHandlerSim errorHandler = new ErrorHandlerSim();
    errorHandler.changeDocumentName(docName);
    errorHandler.addMessageListener(listener);
    contentHandler.addMessageListener(listener);
    try {
      XMLReader parser = (XMLReader)Class.forName(Main.SETTINGS
                                                  .getString("xmlParser"))
                                                  .newInstance();
      parser.setContentHandler(contentHandler);
      parser.setErrorHandler(errorHandler);
      parser.setFeature( "http://xml.org/sax/features/validation",true);
      parser.setFeature( "http://xml.org/sax/features/namespaces",true);
      parser.setFeature( "http://apache.org/xml/features/validation/schema",true);
      parser.parse(new InputSource(input));
    } catch (Exception e) {
      Graph graph = contentHandler.getRoot();
      if (graph != null) {
        MessageApp ma = new MessageApp(e);
        graph.getGraphManager().fireMessage(ma);
      }
      return false;
    }
    contentHandler.getRoot().setTreeLevel(0);
    contentHandler.getRoot().getGraphManager().setContainer(this);
    setGraph(contentHandler.getRoot());

    Date date2 = new Date();
    long run = date2.getTime() - date1.getTime();
    listener.note(new MessageGraph(diagram,"Parsing completed ("+run+" ms)"));
    System.gc();                                    // garbage collection
    return true;
  }

  public void setGraph (GraphComposite diagram) {
    if (diagram != null) diagram.setGraphicalRoot(false);
    this.diagram = diagram;
    diagram.setBaseX(0);
    diagram.setBaseY(0);
    diagram.setGraphicalRoot(true);
    diagram.treeStructureChanged();
    diagram.setZoomLevel(0.4);

    initialize();
    fitDiagram();
    repaint();
    zoomStack = new Stack();
  }

  public Graph getGraph() { return diagram; }

  /** Overrides JComponent addNotify. Registers a ComponentListener with
   *  the new parent.*/
  public void addNotify() {
    super.addNotify();
    getParent().addComponentListener(this);
  }

  /** Adjust the JViewPort to let the specified Graph be visible as close
   *  as possible to the center of the screen.
   *  @param graph          The Graph object to be centered
   *  @param adjZoomLevel   Adjust zoom level? */
  public void centerGraph (Graph graph, boolean adjZoomLevel) {
    recalcDimension();
    Rectangle r = new Rectangle();
    Rectangle visR = getVisibleRect();
    Point p = graph.getAbsoluteLocation();
    r.x = p.x - visR.width/2 + graph.getWidth()/2;
    r.y = p.y - visR.height/2 + graph.getHeight()/2;
    r.width = visR.width;
    r.height = visR.height;
    if (r.x < 0) r.x = 0;
    if (r.y < 0) r.y = 0;
    scrollRectToVisible(r);
  }

  protected void paintComponent(Graphics g) {
    if (diagram == null) return;
    diagram.setX((int)(this.getWidth()/2-diagram.getWidth()/2));
    diagram.setY((int)(this.getHeight()/2-diagram.getHeight()/2));
    diagram.getGraphManager().setGraphics((Graphics2D)g);
    diagram.paint();
    diagram.paintConnections();
  }

  public void addGraphSelectionListener(GraphSelectionListener l) {
    diagram.getGraphManager().getSelectionManager().addGraphSelectionListener(l);
  }
  public void removeGraphSelectionListener(GraphSelectionListener l) {
    diagram.getGraphManager().getSelectionManager().removeGraphSelectionListener(l);
  }

  private void zoomIn(float newLevel, Point direction, boolean rect) {

    Graph graph = diagram.containerOf(direction);
    diagram.setZoomLevel(newLevel);

    // transform click coordinates to new component size
    Rectangle r = new Rectangle(getVisibleRect());
    Dimension oldCompDim = getSize();
    Dimension compDim = recalcDimension();
    int trX = (int)(direction.x * (compDim.width / oldCompDim.getWidth()));
    int trY = (int)(direction.y * (compDim.height / oldCompDim.getHeight()));

    graph= diagram.containerOf(new Point(trX,trY));

    // scroll JViewPort
    if (rect) {
      r.x = trX;
      r.y = trY;
    } else {
      r.x = trX-(direction.x - r.x);
      r.y = trY-(direction.y - r.y);
    }
    scrollRectToVisible(r);
    repaint();
  }

  private void zoomOut(float newLevel, Point direction) {
    diagram.setZoomLevel(newLevel);

    // transform click coordinates to new component size
    Rectangle r = new Rectangle(getVisibleRect());
    Dimension oldCompDim = getSize();
    Dimension compDim = recalcDimension();
    Point trP =
      new Point((int)(direction.x * (compDim.width / oldCompDim.getWidth())),
                (int)(direction.y * (compDim.height / oldCompDim.getHeight())));

    // scroll JViewPort
    trP.x -= direction.x - r.x;
    trP.y -= direction.y - r.y;

    ((JViewport)getParent()).setViewPosition(trP);
    repaint();
  }

  public JToolTip createToolTip() {
    Graph graph = null;
    if ( (lastMove != null) && (diagram != null) ) {
      graph = diagram.containerOf(lastMove.getPoint());
      currentTip = new JSimToolTip(graph);
      currentTip.setComponent(this);
      return currentTip;
    }
    return new JSimToolTip(null);
  }

  // ----- ComponentListener interface implementation

  public void componentHidden(ComponentEvent e) { }
  public void componentMoved(ComponentEvent e) { }
  public void componentResized(ComponentEvent e) {
    recalcDimension();
  }
  public void componentShown(ComponentEvent e) { }

  // ----- MouseInputListener interface implementation

  private boolean mouseClicked;
  private Rectangle dragRect = new Rectangle();
  private Point press = new Point();

  public void mouseClicked(MouseEvent e) {
    if (diagram == null) return;               // no diagram loaded
    switch (_mode) {
      case MODUS_ZOOM:
        float newZoomLevel = (float)diagram.getZoomLevel();

        if ((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0) {

          // push current zoom level on zoom stack
          zoomStack.push(new ZoomRecord(newZoomLevel));
          newZoomLevel += _zoomStep*(newZoomLevel/_maxZoomLevel);
          if (newZoomLevel > _maxZoomLevel) {
            zoomStack.pop();
            return;       // max. level reached
          }

          zoomIn(newZoomLevel,e.getPoint(),false);
          return;
        } else if ((e.getModifiers() & MouseEvent.BUTTON3_MASK) != 0) {
          // zoom out
          if (zoomStack.empty()) {
            newZoomLevel -= _zoomStep*(newZoomLevel/_maxZoomLevel);
          } else {
            ZoomRecord zR = (ZoomRecord)zoomStack.pop();
            newZoomLevel = zR.zoomLevel;
          }
          zoomOut(newZoomLevel,e.getPoint());
          return;
        } else return;
      case MODUS_SELECT:

        // select/deselect a Graph
        if ((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0) {
          Graph graph = diagram.containerOf(e.getPoint());
          SelectionManager s = diagram.getGraphManager().getSelectionManager();
          if (graph != null) {
            if (e.isControlDown()) {
              s.addInverseSelection(graph);
            } else {
              s.setInverseSelection(graph);
            }
          } else s.unselectAll();
        } else if ((e.getModifiers() & MouseEvent.BUTTON3_MASK) != 0) {
          Graph graph = diagram.containerOf(e.getPoint());
          if (graph != null) {
            JPopupMenu popup = createPopupMenu(graph);
            if (popup != null) popup.show(this,e.getX(),e.getY());
          }
        }
        repaint();
        return;

      case MODUS_PAN:
        return;
      default:
        System.err.println("Unreconized mode set. Use one of predefined modus");
        return;
    }
  }
  public void mouseEntered(MouseEvent e) {
  }
  public void mouseExited(MouseEvent e) {
  }
  public void mousePressed(MouseEvent e) {
    mouseClicked = true;
    if (_mode == MODUS_PAN) {
      press.setLocation(getLocationOnScreen());
      press.x += e.getPoint().x;
      press.y += e.getPoint().y;
    } else {
      press.setLocation(e.getX(),e.getY());
      dragRect.setBounds(e.getX(),e.getY(),0,0);
    }
  }
  public void mouseReleased(MouseEvent e) {
    if (diagram == null) return;
    if (mouseClicked) return; // user wasn't draggin
    // check if rectangle was larger then 3 pixels
    if (dragRect.width <= 3 || dragRect.height <= 3) {
      Graphics2D g = (Graphics2D)getGraphics();
      g.setXORMode(Color.white);
      g.draw(dragRect);
      return;
    }
    if (_mode == MODUS_SELECT) {
      Graphics2D g = (Graphics2D)getGraphics();
      g.setXORMode(Color.white);
      g.draw(dragRect);
      Vector bounded = new Vector(30);
      diagram.enumBoundedGraphs(dragRect,bounded);
      if (e.isControlDown()) {
        diagram.getGraphManager().getSelectionManager().addInverseSelection(bounded);
      } else {
        diagram.getGraphManager().getSelectionManager().setInverseSelection(bounded);
      }
      return;
    }
    if (_mode == MODUS_ZOOM) {
      Graphics2D g = (Graphics2D)getGraphics();
      g.setXORMode(Color.white);
      g.draw(dragRect);

      Rectangle r = new Rectangle(getVisibleRect());
      int x,y;
      float zoomLevel;
      if (dragRect.width > dragRect.height) {
        if (diagram.getZoomLevel() < 1) {
          zoomLevel = (r.width / (float)dragRect.width)
            * (float)diagram.getZoomLevel();
        } else {
          zoomLevel = (r.width / (float)dragRect.width)
            * (float)diagram.getZoomLevel();
        }
      } else {
        if (diagram.getZoomLevel() < 1) {
          zoomLevel = (r.height / (float)dragRect.height)
            * (float)diagram.getZoomLevel();
        } else {
          zoomLevel = (r.height / (float)dragRect.height)
            * (float)diagram.getZoomLevel();
        }
      }

      x = (int)(dragRect.x - (r.width/2.0) + (dragRect.width/2.0));
      y = (int)(dragRect.y - (r.height/2.0) + (dragRect.height/2.0));

      // push current zoom level on zoom stack; zoom in
      zoomStack.push(new ZoomRecord((float)diagram.getZoomLevel()));
      //zoomIn(zoomLevel,new Point(x,y),true);
      zoomIn(zoomLevel,new Point(dragRect.x,dragRect.y),true);
      return;
    }
  }
  public void mouseDragged(MouseEvent e) {
    mouseClicked = false;

    if (_mode == MODUS_ZOOM || _mode == MODUS_SELECT) {
      Graphics2D g = (Graphics2D)getGraphics();
      g.setXORMode(Color.white);
      g.draw(dragRect);
      if (press.x > e.getX()) {
        dragRect.width = press.x - e.getX();
        dragRect.x = press.x - dragRect.width;
      } else {
        dragRect.width = e.getX() - press.x;
        dragRect.x = press.x;
      }
      if (press.y > e.getY()) {
        dragRect.height = press.y - e.getY();
        dragRect.y = press.y - dragRect.height;
      } else {
        dragRect.height = e.getY() - press.y;
        dragRect.y = press.y;
      }
      g.draw(dragRect);
      return;
    }
    if (_mode == MODUS_PAN) {

     // set mouse cursor to drag endless for user convenience
      Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
      Point absPoint = getLocationOnScreen();
      absPoint.x += e.getPoint().x;
      absPoint.y += e.getPoint().y;
      try {
        Robot robot = new Robot();
        if (absPoint.x >= (screenSize.width-1)) {
          robot.mouseMove(0,absPoint.y);
          press.x = 0;
          return;
        } else
        if (absPoint.y >= (screenSize.height-1)) {
            robot.mouseMove(absPoint.x,0);
          press.y = 0;
          return;
        } else
        if (absPoint.y == 0) {
          robot.mouseMove(absPoint.x,screenSize.height);
          press.y = screenSize.height;
          return;
        } else
        if (absPoint.x == 0) {
          robot.mouseMove(screenSize.width,absPoint.y);
          press.x = screenSize.width;
          return;
        }
      } catch (AWTException x) {
        // platform config does not allow low level input control
      }
      Rectangle r = getVisibleRect();
      r.x -= absPoint.x - press.x;
      r.y -= absPoint.y - press.y;
      scrollRectToVisible(r);
      press.setLocation(absPoint.x,absPoint.y);
      return;
    }
  }
  public void mouseMoved(MouseEvent e) {
    lastMove = e;
  }

  // ----- KeyListener interface implementation

  public void keyTyped(KeyEvent e) {
    System.out.println("keyTyped() : "+e);
  }

  public void keyPressed(KeyEvent e) {
    System.out.println("keyPressed() : "+e);
  }

  public void keyReleased(KeyEvent e) {
    System.out.println("keyReleased() : "+e);
  }

  // ----- Scrollable interface implementation

  public Dimension getPreferredScrollableViewportSize() {
    Dimension d =
      new Dimension (Toolkit.getDefaultToolkit().getScreenSize());
    d.width *= 0.6;
    d.height *= 0.6;
    return d;
  }
  public int getScrollableBlockIncrement(Rectangle visibleRect,
                                         int orientation, int direction) {
    if (orientation == SwingConstants.HORIZONTAL)
      return visibleRect.width - 40;
    else
      return visibleRect.height - 40;
  }
  public boolean getScrollableTracksViewportHeight() {
    return false;
  }
  public boolean getScrollableTracksViewportWidth() {
    return false;
  }
  public int getScrollableUnitIncrement(Rectangle visibleRect,
                                        int orientation, int direction) {
    return 1;
  }

  private class ZoomRecord {
    float zoomLevel;
    ZoomRecord(float zoomLevel) {
      this.zoomLevel = zoomLevel;
    }
  }

  //-- Action classes (for items in popup menu)
  private class ActionCenterSteps extends AbstractAction {
    boolean centerSteps;
    public ActionCenterSteps() {
      super("Center steps");
      centerSteps = false;
    }
    public void actionPerformed(ActionEvent e) {
      centerSteps = !centerSteps;
      if (centerSteps) {
      } else {
      }
    }
    public boolean getState() {
      return centerSteps;
    }
  }
  private class ActionShowChildren extends AbstractAction {
    public ActionShowChildren() {
      super();
    }
    public void actionPerformed(ActionEvent e) {
      GraphSimul gs = (GraphSimul)focusGraph;
      if (gs.getState() == GraphStep.STATE_INDEPENDENT) {
        gs.setState(GraphStep.STATE_DRAWCHILDREN);
      } else {
        gs.setState(GraphStep.STATE_INDEPENDENT);
      }
      if (diagram.getGraphManager().dimensionsChanged()) {
        diagram.layoutAll();
        fitDiagram();
        centerGraph(focusGraph,false);
        repaint();
      }
      // since the zoom level has been drastically changed we'll delete the
      // zoom history
      zoomStack.clear();
    }
  }
}
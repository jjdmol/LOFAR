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
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

package org.astron.basesim;

/**
 * Title:        Class Main<p>
 * Description:  Main window<p>
 * Copyright:    Copyright (c) <p>
 * Company:      Astron<p>
 * @author Robbert Dam
 * @version 1.0
 */

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.awt.*;
import java.io.*;
import org.astron.util.*;
import org.astron.util.gui.*;

/** Was used to low-level test the Graph tree strucute, layout controllers */
public class TestMain extends JFrame{

  private final static String LAST_PATH = "lastPath";

  AbstractAction fileOpen = new ActionFileOpen();
  AbstractAction fileNew = new ActionFileNew();
  AbstractAction fileExit = new ActionFileExit();
  AbstractAction filePrint = new ActionFilePrint();
  AbstractAction helpAbout = new ActionHelpAbout();
  AbstractAction diagramSelect = new ActionDiagramSelect();
  AbstractAction diagramZoom = new ActionDiagramZoom();
  AbstractAction diagramPan = new ActionDiagramPan();

  JPanel _contentPane;
  JBaseSim baseSim;
  JNotifier notifier;
  JTree simulTree;

  public TestMain() {
    super ("BaseSim GUI");
        addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          try {
            Main.SETTINGS.store();
          } catch (IOException x) {
            System.out.println("failed to write config file: "+x);
          }
          System.exit(0);
        }
    });
    getContentPane().add(new JTestGraph());
    pack();
    show();
  }

  private void loadFile(File file) {/*
    notifier.clearAll();
    GraphManager manager = new GraphManager();
    manager.setContainer(baseSim);
    GraphDiagram diagram = new GraphDiagram(manager,"Base simulation");
    diagram.getGraphManager().addMessageListener(notifier);
    FileInputStream inputStream;
    try {
      inputStream = new FileInputStream(file);
    } catch (IOException e) { e.printStackTrace(); return; }
    if (diagram.parseSimul(inputStream,file.getName())) {
      baseSim.setGraph(diagram);
      setTitle("BaseSim GUI ("+file.getAbsoluteFile()+")");
      ((BaseSimTreeModel)simulTree.getModel()).setGraphDiagram(diagram);
    } else baseSim.clearDiagram();
*/  }

  private JMenuBar initMenu() {
    JMenuBar menuBar = new JMenuBar();

    JMenu menuFile = new JMenu("File");
    JMenu menuHelp = new JMenu("Help");

    JMenuItem itemFileNew = new JMenuItem(fileNew);
    JMenuItem itemFileOpen = new JMenuItem(fileOpen);
    JMenuItem itemFilePrint = new JMenuItem(filePrint);
    JMenuItem itemFileExit = new JMenuItem(fileExit);

    JMenuItem itemHelpAbout = new JMenuItem(helpAbout);

    menuBar.add(menuFile);
    menuBar.add(menuHelp);
    menuFile.add(itemFileNew);
    menuFile.add(itemFileOpen);
    menuFile.add(itemFilePrint);
    menuFile.addSeparator();
    menuFile.add(itemFileExit);
    menuHelp.add(itemHelpAbout);

    return menuBar;
  }

  private JToolBar initDiagramToolBar() {
    JToolBar toolBar = new JToolBar();

    toolBar.add(diagramSelect);
    toolBar.add(diagramPan);
    toolBar.add(diagramZoom);

    return toolBar;
  }

  private void initTree() {
    simulTree =
      new JTree(new BaseSimTreeModel((GraphDiagram)baseSim.getGraph()));
    simulTree.getSelectionModel().setSelectionMode
        (TreeSelectionModel.SINGLE_TREE_SELECTION);

    //Listen for when the selection changes.
    simulTree.addTreeSelectionListener(new TreeSelectionListener() {
      Graph lastGraph;
      public void valueChanged(TreeSelectionEvent e) {
        Object object = simulTree.getLastSelectedPathComponent();
        if (object == null) {
          if (lastGraph == null) return; else object = lastGraph;
        }
        if (object instanceof Graph) {
          Graph graph = (Graph)object;
          lastGraph = graph;
          SelectionManager selMan = graph.getGraphManager().getSelectionManager();
          selMan.setInverseSelection(graph);
          baseSim.centerGraph(graph,false);
        }
      }
    });
  }

  /** Helper function that centers a frame in the middle of the screen */
  public void centerFrame ()
  {
    //Center the window
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension frameSize = getSize();
    if (frameSize.height > screenSize.height) {
      frameSize.height = screenSize.height;
    }
    if (frameSize.width > screenSize.width) {
      frameSize.width = screenSize.width;
    }
    setLocation((screenSize.width - frameSize.width) / 2,
                (screenSize.height - frameSize.height) / 2);
    setVisible(true);
  }

  /** Send a message to the notifier */
  public void fireMessage(MessageEvent e) { notifier.note(e); }

  public static void main(String[] args) {
    try {
      UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    TestMain main1 = new TestMain();
  }

  private class ActionFileOpen extends AbstractAction {
    public ActionFileOpen() {
      super("Open...");
    }
    public void actionPerformed(ActionEvent e) {
      JFileChooser fileC = new JFileChooser();
      String lastPath = Main.SETTINGS.getProperty(LAST_PATH);
      if (lastPath != null) fileC.setCurrentDirectory(new File(lastPath));
      int retVal = fileC.showOpenDialog(_contentPane);
      if (retVal == JFileChooser.APPROVE_OPTION) {
        Main.SETTINGS.setProperty(LAST_PATH,fileC.getSelectedFile().getPath());
        loadFile(fileC.getSelectedFile());
      }
    }
  }
  private class ActionFileNew extends AbstractAction {
    public ActionFileNew() {
      super("New");
    }
    public void actionPerformed(ActionEvent e) {}
  }
  private class ActionFileExit extends AbstractAction {
    public ActionFileExit() {
      super("Exit");
    }
    public void actionPerformed(ActionEvent e) {
      System.exit(0);
    }
  }
  private class ActionFilePrint extends AbstractAction {
    public ActionFilePrint() {
      super("Print...");
    }
    public void actionPerformed(ActionEvent e) {}
  }
  private class ActionHelpAbout extends AbstractAction {
    public ActionHelpAbout() {
      super("About...");
    }
    public void actionPerformed(ActionEvent e) {}
  }

  private class ActionDiagramSelect extends AbstractAction {
    public ActionDiagramSelect() {
      super("Select",new ImageIcon("images/select.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      baseSim.setMode(JBaseSim.MODUS_SELECT);
    }
  }

  private class ActionDiagramZoom extends AbstractAction {
    public ActionDiagramZoom() {
      super("Zoom",new ImageIcon("images/zoom.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      baseSim.setMode(JBaseSim.MODUS_ZOOM);
    }
  }

  private class ActionDiagramPan extends AbstractAction {
    public ActionDiagramPan() {
      super("Pan",new ImageIcon("images/pan.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      baseSim.setMode(JBaseSim.MODUS_PAN);
    }
  }

  /** Little hack that changes the default gray color of a JViewPort */
  private class XScrollPane extends JScrollPane {

    private Color m_color;

    public XScrollPane(JComponent comp) {
        super(comp);
        //m_color = comp.getBackground();
        m_color = new Color (255,255,240);
        // Needed for Java v1.3.0
        getViewport().setBackground(m_color);
    }  // Needed for Java v1.2.2_005
    public void paintComponent(Graphics g) {
        Rectangle r = g.getClipBounds();
        g.setColor(m_color);
        g.fillRect(r.x, r.y, r.width, r.height);
    }
  }
}
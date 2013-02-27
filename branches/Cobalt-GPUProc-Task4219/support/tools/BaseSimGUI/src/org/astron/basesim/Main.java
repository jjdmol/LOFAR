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
import javax.swing.filechooser.*;
import java.awt.event.*;
import java.awt.*;
import java.io.*;
import org.astron.util.*;
import org.astron.util.gui.*;
import kiwi.io.*;
import javax.swing.plaf.basic.*;
import javax.swing.border.*;

public class Main extends JFrame implements GraphSelectionListener,
                                            ChangeListener,
                                            DocumentListener,
                                            ActionListener,
                                            Runnable {

  //-- Configuration object. Reachable from anywhere
  public static Configuration SETTINGS =
    new Configuration(new File("basesim.cfg"));

  //-- enumeration of config keys
  private final static String LAST_OPEN_PATH = "lastOpenPath";
  private final static String LAST_SAVE_PATH = "lastSavePath";

  // indexes for JTabbedPane
  final static int COMP_DIAGRAM = 0;
  final static int COMP_TEXTEDITOR = 1;

  //-- menu items
  AbstractAction fileNew = new ActionFileNew();
  AbstractAction fileOpen = new ActionFileOpen();
  AbstractAction fileSave = new ActionFileSave();
  AbstractAction fileSaveAs = new ActionFileSaveAs();
  AbstractAction fileExit = new ActionFileExit();
  AbstractAction filePrint = new ActionFilePrint();
  AbstractAction editCut = new ActionEditCut();
  AbstractAction editCopy = new ActionEditCopy();
  AbstractAction editPaste = new ActionEditPaste();
  AbstractAction editExpandAll = new ActionEditExpandAll();
  AbstractAction helpAbout = new ActionHelpAbout();

  AbstractAction diagramSelect = new ActionDiagramSelect();
  AbstractAction diagramZoom = new ActionDiagramZoom();
  AbstractAction diagramPan = new ActionDiagramPan();

  //-- graphical components
  JPanel _contentPane;
  JBaseSim baseSim;
  JNotifier notifier;
  JTreeEx simulTree;
  JPanel graphInfoPanel;
  JEditTextArea xmlEditor;
  JTabbedPane tabbedPane;
  JSplitPane splitPane1, splitPane2;
  JButtonEx newButton, openButton, saveButton;
  JToggleButton selectButton, panButton, zoomButton;

  //-- misc settings
  /** Defines behavoir of the simulTree */
  private boolean inspectionMode = false;
  private GraphDiagram diagram;
  /** File (xml) we are currently editing */
  private File currentFile;
  /** Is the XML document still in sync with the diagram? */
  private boolean outOfSync = true;
  /** Was the XML document saved to disk? */
  private boolean documentChanged = true;

  /** Variable needed by parser thread */
  private InputStream tInput;
  /** Variable needed by parser thread */
  private String tDocName;
  /** Variable needed by parser thread */
  private JDialog tParseDialog;

  public Main() {
    super ("BaseSim GUI");
    try {
      SETTINGS.load();
    } catch(Exception e) { e.printStackTrace(); }
    _contentPane = (JPanel)getContentPane();
    setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
    addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {

          if (documentChanged) {
            int retval =
            JOptionPane.showConfirmDialog(null,
                                          "Save the document before exiting?",
                                          "Document changed",
                                          JOptionPane.YES_NO_CANCEL_OPTION);
            if (retval == JOptionPane.CANCEL_OPTION) {
              return;
            }
            if (retval == JOptionPane.YES_OPTION) {
              try {
                if (currentFile == null) {
                  JFileChooser fileC = new JFileChooser();
                  fileC.addChoosableFileFilter(new FileFilterXML());
                  String lastPath = SETTINGS.getProperty(LAST_SAVE_PATH);
                  if (lastPath != null)
                    fileC.setCurrentDirectory(new File(lastPath));
                  fileC.setSelectedFile(new File("Noname.xml"));
                  int retVal = fileC.showSaveDialog(_contentPane);
                  if (retVal == JFileChooser.APPROVE_OPTION) {
                    currentFile = fileC.getSelectedFile();
                    SETTINGS.setProperty(LAST_SAVE_PATH,currentFile.getPath());
                  } else {
                    return;
                  }
                }
                FileOutputStream oStream = new FileOutputStream(currentFile);
                oStream.write(xmlEditor.getText().getBytes());
              } catch (IOException x) {
                x.printStackTrace();
              }
            }
          }

          try {
            SETTINGS.store();
          } catch(Exception x) { x.printStackTrace(); }
          System.exit(0);
        }
    });
    baseSim = new JBaseSim();
    initNotifier();
    initXMLEditor();
    initTree();

    JScrollPane graphScrollPane = new JScrollPane(baseSim);
    graphScrollPane.setVerticalScrollBarPolicy(
      JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    graphScrollPane.setHorizontalScrollBarPolicy(
      JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

    tabbedPane = new JTabbedPane(JTabbedPane.BOTTOM);
    tabbedPane.add(graphScrollPane,COMP_DIAGRAM);
    tabbedPane.add(xmlEditor,COMP_TEXTEDITOR);
    tabbedPane.setTitleAt(COMP_DIAGRAM,"Diagram");
    tabbedPane.setTitleAt(COMP_TEXTEDITOR,"XML");
    tabbedPane.addChangeListener(this);

    JScrollPane treeScrollPane = new JScrollPane(simulTree);
    treeScrollPane.setHorizontalScrollBarPolicy(
      JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    treeScrollPane.setVerticalScrollBarPolicy(
      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);

    splitPane1 = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
    splitPane1.setContinuousLayout(true);
    splitPane1.setLeftComponent(treeScrollPane);
    splitPane1.setRightComponent(tabbedPane);
    splitPane1.setOneTouchExpandable(true);
    splitPane1.setDividerLocation(0.15);
    splitPane2 = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
    splitPane2.setTopComponent(splitPane1);
    splitPane2.setBottomComponent(notifier);
    splitPane2.setOneTouchExpandable(true);
    splitPane2.setDividerLocation(0.7);
    splitPane2.setContinuousLayout(true);

    JPanel toolPanel = new JPanel(new BorderLayout());
    toolPanel.add(initMenu(),BorderLayout.NORTH);
    toolPanel.add(initToolBar(),BorderLayout.CENTER);

    _contentPane.setLayout(new BorderLayout());
    _contentPane.add(splitPane2,BorderLayout.CENTER);
    _contentPane.add(toolPanel,BorderLayout.NORTH);

    pack();
    centerFrame();
    show();
    syncToDocument();
  }

  public void setParseDialog(JParseDialog dialog) {
    tParseDialog = dialog;
  }

  private void loadDiagram(InputStream input, String docName) {
    notifier.clearAll();
    diagram = null;
    System.gc();        // good point to do garbage collection

    // start parser thread; build the "busy parsing" dialog
    tInput = input;
    tDocName = docName;
    Thread thread = new Thread(this);
    thread.start();
    //tParseDialog = new JParseDialog(this,thread);
  }

  public void run()
  {
    if (baseSim.parseSimul(tInput,notifier,tDocName)) {

      // wait till all images are loaded
      try {
        GraphWorkHolderImage.getMediaTracker().waitForAll();
      } catch(InterruptedException ie) { ie.printStackTrace(); }

      // load diagram in JBaseSim
      if (documentChanged) {
        GraphSimul gs = (GraphSimul)baseSim.getGraph();
        gs.setState(GraphStep.STATE_DRAWCHILDREN,true);
        if (gs.getGraphManager().dimensionsChanged()) {
          gs.layoutAll();
          baseSim.fitDiagram();
          baseSim.centerGraph((Graph)gs,false);
          baseSim.repaint();
        }
      }

      ((BaseSimTreeModel)simulTree.getModel())
        .setGraphDiagram((GraphDiagram)baseSim.getGraph());
      baseSim.addGraphSelectionListener(this);
    } else baseSim.clearDiagram();
  }

  private void loadFileInEditor(File file) {
    // load diagram in JTextArea (XML editor)
    byte[] b = new byte[(int)file.length()];
    try {
      FileInputStream inputStream = new FileInputStream(file);
      inputStream.read(b);
    } catch(IOException e) { e.printStackTrace(); return; }
    String xmlText = new String(b);
    outOfSync = false;
    xmlEditor.getDocument().removeDocumentListener(this);
    xmlEditor.setText(xmlText);
    xmlEditor.getDocument().addDocumentListener(this);
    documentChanged = false;
  }

  /** (re)parses the document */
  private void syncToDocument() {
    if (!outOfSync) return;
    loadDiagram(new ByteArrayInputStream(xmlEditor.getText().getBytes()),null);
    outOfSync = false;
  }

  /** Loads the specified file */
  private void loadFile(File file) {

    // load document in XML editor
    loadFileInEditor(file);

    // load document in diagram control
    FileInputStream inputStream;
    try {
      inputStream = new FileInputStream(file);
    } catch (IOException e) { e.printStackTrace(); return; }
    loadDiagram(inputStream,file.getName());

    // adjust caption of main window
    setTitle("BaseSim GUI ("+file.getAbsoluteFile()+")");
  }

  private void initNotifier() {
    notifier = new JNotifier();
  }

  private JMenuBar initMenu() {
    JMenuBar menuBar = new JMenuBar();

    JMenu menuFile = new JMenu("File");
    JMenu menuEdit = new JMenu("Edit");
    JMenu menuView = new JMenu("View");
    JMenu menuHelp = new JMenu("Help");

    JMenuItem itemFileNew = new JMenuItem(fileNew);
    JMenuItem itemFileOpen = new JMenuItem(fileOpen);
    JMenuItem itemFileSave = new JMenuItem(fileSave);
    JMenuItem itemFileSaveAs = new JMenuItem(fileSaveAs);
    JMenuItem itemFilePrint = new JMenuItem(filePrint);
    JMenuItem itemFileExit = new JMenuItem(fileExit);

    JMenuItem itemEditCut = new JMenuItem(editCut);
    JMenuItem itemEditCopy = new JMenuItem(editCopy);
    JMenuItem itemEditPaste = new JMenuItem(editPaste);
    JMenuItem itemEditExpandAll = new JMenuItem(editExpandAll);

    JMenuItem itemHelpAbout = new JMenuItem(helpAbout);

    // set mnemonics
    menuFile.setMnemonic(KeyEvent.VK_F);
    itemFileOpen.setMnemonic(KeyEvent.VK_O);
    itemFileSave.setMnemonic(KeyEvent.VK_S);
    itemFilePrint.setMnemonic(KeyEvent.VK_P);
    itemFileExit.setMnemonic(KeyEvent.VK_X);
    menuEdit.setMnemonic(KeyEvent.VK_E);
    menuView.setMnemonic(KeyEvent.VK_V);
    menuHelp.setMnemonic(KeyEvent.VK_H);

    menuBar.add(menuFile);
    menuBar.add(menuEdit);
    //menuBar.add(menuView);
    menuBar.add(menuHelp);

    menuFile.add(itemFileNew);
    menuFile.add(itemFileOpen);
    menuFile.add(itemFileSave);
    menuFile.add(itemFileSaveAs);
    menuFile.add(itemFilePrint);
    menuFile.addSeparator();
    menuFile.add(itemFileExit);

    menuEdit.add(itemEditCut);
    menuEdit.add(itemEditCopy);
    menuEdit.add(itemEditPaste);
    menuEdit.addSeparator();
    menuEdit.add(itemEditExpandAll);

    menuHelp.add(itemHelpAbout);

    return menuBar;
  }

  private JToolBar initToolBar() {
    JToolBar toolBar = new JToolBar();

    selectButton = new JToggleButtonEx(diagramSelect);
    selectButton.setText("");
    selectButton.setMnemonic(KeyEvent.VK_S);
    selectButton.setSelected(true);
    panButton = new JToggleButtonEx(diagramPan);
    panButton.setText("");
    panButton.setMnemonic(KeyEvent.VK_P);
    zoomButton = new JToggleButtonEx(diagramZoom);
    zoomButton.setText("");
    zoomButton.setMnemonic(KeyEvent.VK_Z);
    ButtonGroup buttonGroup = new ButtonGroup();
    buttonGroup.add(selectButton);
    buttonGroup.add(panButton);
    buttonGroup.add(zoomButton);

    newButton = new JButtonEx(fileNew);
    newButton.setText("");
    newButton.setToolTipText("New");
    newButton.setIcon(new ImageIcon("images/New24.gif"));
    openButton = new JButtonEx(fileOpen);
    openButton.setText("");
    openButton.setIcon(new ImageIcon("images/Open24.gif"));
    openButton.setToolTipText("Open");
    saveButton = new JButtonEx(fileSave);
    saveButton.setText("");
    saveButton.setIcon(new ImageIcon("images/Save24.gif"));
    saveButton.setToolTipText("Save");

    toolBar.add(selectButton);
    toolBar.add(panButton);
    toolBar.add(zoomButton);

    JSeparator sep = new JSeparator(JSeparator.VERTICAL);
    Dimension d1 = new Dimension(2,20);
    Dimension d2 = new Dimension(10,20);
    sep.setPreferredSize(d1);
    sep.setMaximumSize(d1);
    toolBar.addSeparator(d2);
    toolBar.add(sep);
    toolBar.addSeparator(d2);

    toolBar.add(newButton);
    toolBar.add(openButton);
    toolBar.add(saveButton);

    return toolBar;
  }

  private void initTree() {
    String imageName = SETTINGS.getString("simulTreeBk","images/tb1.jpg");
    Image i = Toolkit.getDefaultToolkit().createImage(imageName);
    MediaTracker mt = new MediaTracker(this);
    GraphWorkHolderImage.setMediaTracker(mt);
    mt.addImage(i,0);
    try {
      mt.waitForAll(); // wait for image
    } catch(InterruptedException ie) { ie.printStackTrace(); }
    simulTree =
      new JTreeEx(new BaseSimTreeModel((GraphDiagram)baseSim.getGraph()));
    simulTree.getSelectionModel().setSelectionMode
        (TreeSelectionModel.SINGLE_TREE_SELECTION);
    simulTree.setCellRenderer(new GraphCellRenderer());
    simulTree.setBackground(i);

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
            if (!graph.isVisible()) return;
            lastGraph = graph;
          if (inspectionMode) {
            if (graph instanceof GraphComposite) {
              baseSim.setGraph((GraphComposite)graph);
            }
          } else {
            SelectionManager selMan = graph.getGraphManager().getSelectionManager();
            selMan.setInverseSelection(graph);
            baseSim.centerGraph(graph,false);
          }
        }
      }
    });
  }

  private void initXMLEditor() {
    xmlEditor = new JEditTextArea();
    xmlEditor.setTokenMarker(new XMLTokenMarker());
    loadFileInEditor(new File("template.xml"));
    outOfSync = true;
    xmlEditor.getDocument().addDocumentListener(this);
    int offset = xmlEditor.getLineStartOffset(10);
    xmlEditor.setCaretPosition(offset);
    xmlEditor.registerKeyboardAction(     // register Ctrl-S (save)
      this,
      KeyStroke.getKeyStroke(KeyEvent.VK_S,Event.CTRL_MASK),
      JComponent.WHEN_IN_FOCUSED_WINDOW
    );
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

  //-- Implementation of the GraphSelectionListener interface
  /** Display info panel for this component */
  public void valueChanged(GraphSelectionEvent e) {
    //REVISIT: implement
    if (e.isSelected()) {
    } else {
    }
  }

  /** Send a message to the notifier */
  public void fireMessage(MessageEvent e) { notifier.note(e); }

  /** Sets the keyboard focus to a specific component. Is called by the
   *  MessageEvents in the JNotifier class.
   *
   *  @param component Can be COMP_DIAGRAM or COMP_TEXTEDITOR */
  public void setFocus(int component) {
    switch(component) {
      case COMP_DIAGRAM:
        tabbedPane.setSelectedIndex(0);
        break;
      case COMP_TEXTEDITOR:
        tabbedPane.setSelectedIndex(1);
        break;
    }
  }

  /** Retreive a handle to the (xml) editor component*/
  public JEditTextArea getEditor() {
    return xmlEditor;
  }

  // -- ChangeListener implementation

  public void stateChanged(ChangeEvent e) {
    if (e.getSource() == tabbedPane) {
      // if the user switched between the XML editor and the diagram we must
      // synchronize the document and the visualisation with each other.

      if (tabbedPane.getSelectedIndex() == COMP_DIAGRAM) {
        // the user switched to the diagram; reparse the document
        syncToDocument();

        // set controls states
        selectButton.setEnabled(true);
        panButton.setEnabled(true);
        zoomButton.setEnabled(true);
      }

      if (tabbedPane.getSelectedIndex() == COMP_TEXTEDITOR) {
        // the user switched to the editor; explicitly set the keyboard focus
        xmlEditor.grabFocus();

        // set controls states
        selectButton.setEnabled(false);
        panButton.setEnabled(false);
        zoomButton.setEnabled(false);
      }
    }
  }

  // -- DocumentListener implementation

  public void changedUpdate(DocumentEvent e) {
  }
  public void insertUpdate(DocumentEvent e) {
    outOfSync = true;
    documentChanged = true;
  }
  public void removeUpdate(DocumentEvent e) {
    outOfSync = true;
    documentChanged = true;
  }

  // -- ActionListener implementation

  public void actionPerformed(ActionEvent e) {
    //REVISIT: save file (ctrl-s)
    System.out.println("actionPerformed: "+e);
  }

  public static void main(String[] args) {
    try {
      UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    Main main1 = new Main();
  }
  private class ActionFileNew extends AbstractAction {
    public ActionFileNew() {
      super("New", new ImageIcon("images/New16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      if (documentChanged) {
        int retval =
          JOptionPane.showConfirmDialog(null,
                                        "Save the document before exiting?",
                                        "Document changed",
                                        JOptionPane.YES_NO_CANCEL_OPTION);
        if (retval == JOptionPane.CANCEL_OPTION) {
          return;
        }
        if (retval == JOptionPane.YES_OPTION) {
          try {
            if (currentFile == null) {
              JFileChooser fileC = new JFileChooser();
              fileC.addChoosableFileFilter(new FileFilterXML());
              String lastPath = SETTINGS.getProperty(LAST_SAVE_PATH);
              if (lastPath != null)
              fileC.setCurrentDirectory(new File(lastPath));
              fileC.setSelectedFile(new File("Noname.xml"));
              int retVal = fileC.showSaveDialog(_contentPane);
              if (retVal == JFileChooser.APPROVE_OPTION) {
                currentFile = fileC.getSelectedFile();
                SETTINGS.setProperty(LAST_SAVE_PATH,currentFile.getPath());
              } else {
                return;
              }
            }
            FileOutputStream oStream = new FileOutputStream(currentFile);
            oStream.write(xmlEditor.getText().getBytes());
          } catch (IOException x) { x.printStackTrace(); }
        }
      }
      initXMLEditor();
      outOfSync = true;
      syncToDocument();

      currentFile = null;

      // adjust caption of main window
      setTitle("BaseSim GUI");
    }
  }
  private class ActionFileOpen extends AbstractAction {
    public ActionFileOpen() {
      super("Open...", new ImageIcon("images/Open16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      JFileChooser fileC = new JFileChooser();
      fileC.addChoosableFileFilter(new FileFilterXML());
      String lastPath = SETTINGS.getProperty(LAST_OPEN_PATH);
      if (lastPath != null) fileC.setCurrentDirectory(new File(lastPath));
      int retVal = fileC.showOpenDialog(_contentPane);
      if (retVal == JFileChooser.APPROVE_OPTION) {
        currentFile = fileC.getSelectedFile();
        SETTINGS.setProperty(LAST_OPEN_PATH,currentFile.getPath());
        loadFile(currentFile);
      }
    }
  }
  private class ActionFileSave extends AbstractAction {
    public ActionFileSave() {
      super("Save", new ImageIcon("images/Save16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      try {
        if (currentFile == null) {
          JFileChooser fileC = new JFileChooser();
          fileC.addChoosableFileFilter(new FileFilterXML());
          String lastPath = SETTINGS.getProperty(LAST_SAVE_PATH);
          if (lastPath != null) fileC.setCurrentDirectory(new File(lastPath));
          fileC.setSelectedFile(new File("Noname.xml"));
          int retVal = fileC.showSaveDialog(_contentPane);
          if (retVal == JFileChooser.APPROVE_OPTION) {
            currentFile = fileC.getSelectedFile();
            SETTINGS.setProperty(LAST_SAVE_PATH,currentFile.getPath());
          } else return;
        }
        FileOutputStream oStream = new FileOutputStream(currentFile);
         oStream.write(xmlEditor.getText().getBytes());
        documentChanged = false;
      } catch (IOException x) {
        x.printStackTrace();
      }
    }
  }
  private class ActionFileSaveAs extends AbstractAction {
    public ActionFileSaveAs() {
      super("Save As...", new ImageIcon("images/SaveAs16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      try {
        JFileChooser fileC = new JFileChooser();
        fileC.addChoosableFileFilter(new FileFilterXML());
        if (currentFile == null) {
          fileC.setSelectedFile(new File("Noname.xml"));
        } else {
          fileC.setSelectedFile(currentFile);
        }
        String lastPath = SETTINGS.getProperty(LAST_SAVE_PATH);
        if (lastPath != null) fileC.setCurrentDirectory(new File(lastPath));
        int retVal = fileC.showSaveDialog(_contentPane);
        if (retVal == JFileChooser.APPROVE_OPTION) {
         currentFile = fileC.getSelectedFile();
         SETTINGS.setProperty(LAST_SAVE_PATH,currentFile.getPath());
        } else return;
        FileOutputStream oStream = new FileOutputStream(currentFile);
        oStream.write(xmlEditor.getText().getBytes());
        documentChanged = false;
      } catch (IOException x) {
        x.printStackTrace();
      }
    }
  }
  private class ActionFileExit extends AbstractAction {
    public ActionFileExit() {
      super("Exit", new ImageIcon("images/Stop16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      if (documentChanged) {
        int retval =
          JOptionPane.showConfirmDialog(null,
                                        "Save the document before exiting?",
                                        "Document changed",
                                        JOptionPane.YES_NO_CANCEL_OPTION);
        if (retval == JOptionPane.CANCEL_OPTION) {
          return;
        }
        if (retval == JOptionPane.YES_OPTION) {
          try {
            if (currentFile == null) {
              JFileChooser fileC = new JFileChooser();
              fileC.addChoosableFileFilter(new FileFilterXML());
              String lastPath = SETTINGS.getProperty(LAST_SAVE_PATH);
              if (lastPath != null)
              fileC.setCurrentDirectory(new File(lastPath));
              fileC.setSelectedFile(new File("Noname.xml"));
              int retVal = fileC.showSaveDialog(_contentPane);
              if (retVal == JFileChooser.APPROVE_OPTION) {
                currentFile = fileC.getSelectedFile();
                SETTINGS.setProperty(LAST_SAVE_PATH,currentFile.getPath());
              } else {
                return;
              }
            }
            FileOutputStream oStream = new FileOutputStream(currentFile);
            oStream.write(xmlEditor.getText().getBytes());
          } catch (IOException x) { x.printStackTrace(); }
        }
      }

      try {
        SETTINGS.store();
      } catch(Exception x) { x.printStackTrace(); }
      System.exit(0);
   }
  }
  private class ActionFilePrint extends AbstractAction {
    public ActionFilePrint() {
      super("Print...", new ImageIcon("images/Print16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      JOptionPane.showMessageDialog(null,"Not yet implemented");
    }
  }
  private class ActionEditCut extends AbstractAction {
    public ActionEditCut() {
      super("Cut", new ImageIcon("images/Cut16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      if (tabbedPane.getSelectedIndex() == COMP_TEXTEDITOR) {
        xmlEditor.cut();
      }
    }
  }
  private class ActionEditCopy extends AbstractAction {
    public ActionEditCopy() {
      super("Copy",new ImageIcon("images/Copy16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      if (tabbedPane.getSelectedIndex() == COMP_TEXTEDITOR) {
        xmlEditor.copy();
      }
    }
  }
  private class ActionEditPaste extends AbstractAction {
    public ActionEditPaste() {
      super("Paste", new ImageIcon("images/Paste16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      if (tabbedPane.getSelectedIndex() == COMP_TEXTEDITOR) {
        xmlEditor.paste();
      }
    }
  }
  private class ActionEditExpandAll extends AbstractAction {
    public ActionEditExpandAll() {
      super("Expand all");
    }
    public void actionPerformed(ActionEvent e) {

      GraphSimul gs = (GraphSimul)baseSim.getGraph();
      gs.setState(GraphStep.STATE_DRAWCHILDREN,true);

      if (gs.getGraphManager().dimensionsChanged()) {
        gs.layoutAll();
        baseSim.fitDiagram();
        baseSim.centerGraph((Graph)gs,false);
        baseSim.repaint();
      }
      // REVISIT: zoom history should be cleared
    }
  }
  private class ActionHelpAbout extends AbstractAction {
    public ActionHelpAbout() {
      super("About...", new ImageIcon("images/Help16.gif"));
    }
    public void actionPerformed(ActionEvent e) {
      JOptionPane.showMessageDialog(null,"BaseSimGUI (c) ASTRON 2001");
    }
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
      super("Zoom",new ImageIcon("images/Zoom24.gif"));
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

  private class FileFilterXML extends javax.swing.filechooser.FileFilter {
    public boolean accept (File f) {
      if (f.isDirectory()) {
        return true;
      }
      String extension = Utils.getExtension(f);
      if ("xml".equals(extension)) return true; else return false;
    }
    public String getDescription() {
      return "XML files (*.xml)";
    }
  }

  /** Our own custom tree cell renderer */
  private class GraphCellRenderer extends JLabel implements TreeCellRenderer
  {
	public GraphCellRenderer()
	{
          setOpaque(false);
	  this.setBackground(null);
	}

	public Component getTreeCellRendererComponent(JTree tree,
                                                      Object value,
                                                      boolean sel,
                                                      boolean expanded,
                                                      boolean leaf,
                                                      int row,
                                                      boolean hasFocus)
        {
          setFont(tree.getFont());
          Graph graph = null;
          if (value instanceof Graph) {
            graph = (Graph)value;
            setEnabled(graph.isVisible());
	    setText(graph.getName());
            Icon icon = graph.getIcon();
            if (icon == null) {
              if (leaf) {
                setIcon(UIManager.getIcon("Tree.leafIcon"));
                setDisabledIcon(UIManager.getIcon("Tree.leafIcon"));
	      } else if (expanded) {
	        setIcon(UIManager.getIcon("Tree.openIcon"));
                setDisabledIcon(UIManager.getIcon("Tree.openIcon"));
	      } else {
	        setIcon(UIManager.getIcon("Tree.closedIcon"));
                setDisabledIcon(UIManager.getIcon("Tree.closedIcon"));
	      }
            }
	    if(sel && graph.isVisible()) {
              setForeground(Color.white);
              setBackground(Color.blue);
              setOpaque(true);
            } else {
              setForeground(Color.black);
              setBackground(Color.white);
              setOpaque(false);
            }
          } else {
            setText((String)value);
          }
	  return this;
	}
    }

  private class JParseDialog extends JDialog implements ActionListener {

    Container content;
    Thread thread;

    public JParseDialog(Frame frame,Thread thread) {
      super(frame,"SAX Parser",true);
      content = getContentPane();
      content.setLayout(new BorderLayout(10,10));
      JLabel label = new JLabel("Parsing the document...",JLabel.CENTER);
      JButton button = new JButton("Interrupt");
      button.addActionListener(this);
      content.add(label,BorderLayout.CENTER);
      content.add(button,BorderLayout.SOUTH);
      ((Main)frame).setParseDialog(this);
      this.thread = thread;
      thread.start();
      pack();
      centerFrame();
      show();
    }

    public void actionPerformed(ActionEvent e) {
      thread.interrupt();
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
  }
}

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
 * Title:        Class ContHandlerSim<p>
 * Description:  SAX parser object<p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.*;
import java.awt.*;
import org.xml.sax.*;
import org.astron.util.*;
import org.astron.util.gui.*;
import kiwi.io.*;
import java.io.*;

/**
 * ContentHandler (SAX) implementation for parsing a BaseSim diagram
 */
public class ContHandlerSim implements ContentHandler {

  /** List of MessageListeners */
  private ArrayList _mlisteners = new ArrayList();

  final static String classMapFileName = "classMap.cfg";
  final static char DELIMITER = '.';
  /** The parent of the subtree that will be parsed */
  private GraphComposite owner = null;
  /** The Graph we are currently working on */
  private Graph cGraph;
  /** The path to cGraph */
  private String cPath;
  /** List of MessageListeners */
  private ArrayList mlisteners = new ArrayList();
  /** The root composite*/
  private GraphComposite root;
  /** Table that maps nodes to colors */
  private Vector nodeColorMap = new Vector(30);
  /** Min. color intensity */
  private final int MIN_C_INT = 180;
  /** Max. color intensity */
  private final int MAX_C_INT = 240;
  /** Difference factor between colors of parent and child that run on
   *  same node */
  private final double CHILD_COLOR_ADJUST = 0.85;
  /** Base color table */
  private Color distColor[];
  /** Make sure loopback warning appears only once */
  private boolean noLoopWarning = true;
  /** A table of names with Graph objects for relative fast lookup of names */
  private Hashtable graphtable;
  /** A list of connection pairs. First column contains instance of source
   *  Graph, the second column contains a (still unresolved) name of a Graph. */
  private LinkedList conList;
  /** Properties for mapping BaseSim classes to BaseSimGUI classes. */
  private ConfigFile classMap = new ConfigFile(new File(classMapFileName));
  private Vector nodeToColor = new Vector(20); // [node,color][20]
  private static int colorFollowup = 0;
  private GraphStep exStep = null;
  private String exStepName = null;
  private GraphWorkHolder currentWorker = null;
  private ArrayList exDHList = new ArrayList();

  /** Constructs a new parser handler. */
  public ContHandlerSim()
  {
    cGraph = null;
    cPath = new String();
    conList = new LinkedList();
    graphtable = new Hashtable();
    initDistColors();
    try {
      classMap.load();
    } catch(IOException e) {
      fireMessage(new MessageApp(e,"Cannot open " + classMapFileName));
    }
  }
  /** Construct a new (partial) parser handler.
   * @param owner The parent Graph to which the parsed step will be attached.
   * @param step The name of the Step or Simul that should be parsed from the file.
   */
  public ContHandlerSim(GraphComposite owner, String step)
  {
    this.owner = owner;
    exStepName = step;
    cGraph = null;
    cPath = new String();
    conList = new LinkedList();
    graphtable = new Hashtable();
    initDistColors();
    try {
      classMap.load();
    } catch(IOException e) {
      fireMessage(new MessageApp(e,"Cannot open " + classMapFileName));
    }
  }

  public GraphComposite getRoot() { return root; }

  public void setDocumentLocator(Locator locator)
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  public void startDocument() throws SAXException
  {
  }

  public void endDocument() throws SAXException
  {
    createConnections();
  }

  public void startPrefixMapping(String prefix, String uri) throws SAXException
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  public void endPrefixMapping(String prefix) throws SAXException
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  public void startElement(String namespaceURI, String localName, String qName,
                           Attributes atts) throws SAXException
  {
    Graph graph;
    String className = null;
    int index;
    if ("basesim".equals(qName)) {

      // -------------------------------------------
      // Begin of <basesim> element
      // -------------------------------------------

      if (owner != null) {
        cGraph = owner;
      } else {
        GraphManager manager = new GraphManager();
        cGraph = new GraphDiagram(manager,"Base simulation");
        root = (GraphComposite)cGraph;
        MessageListener ml [] = getMessageListeners();
        for (int i=0; i<ml.length; i++) {
          root.getGraphManager().addMessageListener(ml[i]);
        }
      }
    } else if ("step".equals(qName)) {

      // -------------------------------------------
      // Begin of <step> element
      // -------------------------------------------

      if (!(cGraph instanceof GraphSimul)) return;

      // load attributes
      String node = atts.getValue("node");
      String name = atts.getValue("name");

      graph = instGraphFromName(atts,GraphStep.class);

      if ( (owner != null) && (exStep == null) ) {
        // partial parsing mode
        if (name.equals(exStepName)) {
          exStep = (GraphStep)graph;
          root = exStep;
        } else {
          return; // skip
        }
      }

      ((GraphSimul)cGraph).addStep((GraphStep)graph);
      if (node != null) {
        Color color = genColorFromNode(node);
        ((GraphStep)graph).setNode(node);
        String parentNode = ((GraphStep)cGraph).getNode();
        if (parentNode != null && node.equals(parentNode)) {
          // child is slightly darker
          graph.setBackground(darker((Color)cGraph.getBackground()));
        } else {
          graph.setBackground(color);
        }
      }

      cGraph = graph;
      graphtable.put(new Reference(cGraph.getName(),cPath),cGraph);
      addToPath(cGraph);
    } else if ("simul".equals(qName)) {

      // -------------------------------------------
      // Begin of <simul> element
      // -------------------------------------------

      // load attributes
      String node = atts.getValue("node");
      String name = atts.getValue("name");

      graph = instGraphFromName(atts,GraphSimul.class);
      ((GraphSimul)graph).setState(GraphSimul.STATE_INDEPENDENT);

      if ( (owner != null) && (exStep == null) ) {
        // partial parsing mode
        if (name.equals(exStepName)) {
          exStep = (GraphStep)graph;
          root = exStep;
        } else {
          return; // skip
        }
      }

      if (node != null) {
        Color color = genColorFromNode(node);
        ((GraphStep)graph).setNode(node);
        String parentNode = ((GraphStep)cGraph).getNode();
        if (parentNode != null && node.equals(parentNode)) {
          // child is slightly darker
          graph.setBackground(darker((Color)cGraph.getBackground()));
        } else {
          graph.setBackground(color);
        }
      }

      ((GraphSimul)cGraph).addStep((GraphStep)graph);

      cGraph = graph;
      graphtable.put(new Reference(cGraph.getName(),cPath),cGraph);
      addToPath(cGraph);

    } else if ("workholder".equals(qName)) {

      // -------------------------------------------
      // Begin of <workholder> element
      // -------------------------------------------

      if ( (owner != null) && (exStep == null) ) return; // skip

      if (cGraph instanceof GraphSimul) {
        graph = instGraphFromName(atts,GraphWorkHolderImage.class);
      } else {
        graph = instGraphFromName(atts,GraphWorkHolder.class);
      }
      ((GraphWorkHolder)graph).setClassName(atts.getValue("class"));
      ((GraphStep)cGraph).setWorkHolder((GraphWorkHolder)graph);
      currentWorker = (GraphWorkHolder)graph;
      graphtable.put(new Reference(graph.getName(),cPath),graph);

    } else if ("dataholder".equals(qName)) {

      // -------------------------------------------
      // Begin of <dataholder> element
      // -------------------------------------------

      if ( (owner != null) && (exStep == null) ) return; // skip
      boolean inputDH = "in".equals(atts.getValue("type"));

      graph = currentWorker.buildDataHolder(inputDH);
      graph.setGraphManager(cGraph.getGraphManager());
      if ((index = atts.getIndex("name")) != -1) {
        graph.setName(atts.getValue(index));
      }
      ((GraphDataHolder)graph).setInput(inputDH);
      ((GraphStep)cGraph).addDataHolder((GraphDataHolder)graph);
      graphtable.put(new Reference(graph.getName(),cPath),graph);
      if (cGraph == root) {
        exDHList.add(new RefGraph(graph,new Reference(graph.getName(),cPath)));
      }

    } else if ("connect".equals(qName)) {

      // -------------------------------------------
      // Begin of <connect> element
      // -------------------------------------------

      if ( (owner != null) && (exStep == null) ) return; // skip

      // load attributes
      String src = atts.getValue("src");
      String dest = atts.getValue("dest");
      String rate = atts.getValue("rate");
      String itsClass = atts.getValue("class");

      if ( (src != null) && (dest != null) ) {
        NameConnection n = new NameConnection(src,dest,cPath);
        if (rate != null) {
          n.setRate(Integer.parseInt(rate));
        }
        if (itsClass != null) {
          n.setItsClass(itsClass);
        }
        conList.add(n);
      }
    } else if ("exstep".equals(qName)) {

      // -------------------------------------------
      // Begin of <exstep> element
      // -------------------------------------------

      if ( (owner != null) && (exStep == null) ) return; // skip

      // load attributes
      String src = atts.getValue("src");
      String name = atts.getValue("name");
      String exName = atts.getValue("exname");
      String node = atts.getValue("node");

      if ( (src != null) && (name != null)
           && (cGraph instanceof GraphComposite)
           && (exName != null) ) {

        // start a new parser
        File file;
        FileInputStream input;
        try {
          file = new File(src);
          input = new FileInputStream(file);
        } catch (FileNotFoundException e) {
          fireMessage(new MessageXML(this,"Can't load step. File not found: " + src,
                                       MessageEvent.ERROR));
          return;
        } catch (SecurityException e) {
          fireMessage(new MessageXML(this,"Cannot access file " + src,
                                       MessageEvent.ERROR));
          return;
        }

        ContHandlerSim contentHandler =
          new ContHandlerSim((GraphComposite)cGraph,exName);
        ErrorHandlerSim errorHandler = new ErrorHandlerSim();
        errorHandler.changeDocumentName(src);
        MessageListener ml [] = cGraph.getGraphManager().getMessageListeners();
        for (int i=0; i<ml.length; i++) errorHandler.addMessageListener(ml[i]);
        try {
          XMLReader parser =
            (XMLReader)Class.forName(Main.SETTINGS.getString("xmlParser"))
                                     .newInstance();
          parser.setContentHandler(contentHandler);
          parser.setErrorHandler(errorHandler);
          parser.setFeature( "http://xml.org/sax/features/validation",true);
          parser.setFeature( "http://xml.org/sax/features/namespaces",true);
          parser.setFeature( "http://apache.org/xml/features/validation/schema",true);
          parser.parse(new InputSource(input));
        } catch (Exception e) { e.printStackTrace(); }

        graph = contentHandler.getRoot();

        if (graph == null) {
          fireMessage(new MessageXML(this,"External step not found: " + exName,
                                     MessageEvent.ERROR));
          return;
        }

        if (node != null) {
          Color color = genColorFromNode(node);
          ((GraphStep)graph).setNode(node);
          String parentNode = ((GraphStep)cGraph).getNode();
          if (parentNode != null && node.equals(parentNode)) {
            // child is slightly darker
            graph.setBackground(darker((Color)cGraph.getBackground()));
          } else {
            graph.setBackground(color);
          }
        }

        graph.setName(name);
        graphtable.put(new Reference(name,cPath),graph);

        ArrayList list = contentHandler.getEdgeDH();
        for (int i=0; i<list.size(); i++) {
          RefGraph rg = (RefGraph)list.get(i);
          Reference r = new Reference(rg.ref.getName(),cPath+"."+name);
          graphtable.put(r,rg.graph);
        }
      }
     }
  }

  public void endElement(String namespaceURI, String localName, String qName)
    throws SAXException
  {
    if (cGraph == null) {
      System.out.println("cGraph == null");
      return;
    }
    if ("step".equals(qName)) {
      if ( (owner != null) && (exStep == null) ) return; // skip
      if (cGraph == exStep) exStep = null;
      removeFromPath(cGraph);
      cGraph = cGraph.getOwner().getOwner();
    } else if ("simul".equals(qName)) {
      if ( (owner != null) && (exStep == null) ) return; // skip
      removeFromPath(cGraph);
      cGraph = cGraph.getOwner().getOwner();
      if (cGraph == exStep) exStep = null;
    }
  }

  public void characters(char[] ch, int start, int length) throws SAXException
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  public void ignorableWhitespace(char[] ch, int start, int length) throws SAXException
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  public void processingInstruction(String target, String data) throws SAXException
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  public void skippedEntity(String name) throws SAXException
  {
    //TODO: Implement this org.xml.sax.ContentHandler method
  }

  /** This returns a lists of dataholders that are on the edges of a parsed
   *  Step in partial parsing mode. Call only after parsing completed. */
  public ArrayList getEdgeDH() { return exDHList; }

  /** Helper function that instantiates a connection object using the specified
   *  class name and base class. */
  private Connection instConnectionFromName(String name,
                                            Class baseClass) {
    Connection conn;
    String className = null;
    if (name != null) className = classMap.getProperty(name);
    if (className == null) {
      // nope; instantiate base class
      try {
        conn = (Connection)baseClass.newInstance();
      } catch (Exception e) { e.printStackTrace(); return null; }
    } else {
      // yes; try to instantiate specified class.
      Object object = null;
      try {
        object = Class.forName(className).newInstance();
        // check if this object is an instance of baseClass
        if (baseClass.isInstance(object)) {
          conn = (Connection)object;
        } else {
          try { conn = (Connection)baseClass.newInstance(); }
          catch (Exception e) { e.printStackTrace(); return null; }
            fireMessage(new MessageEvent(this,"Cannot instantiate object "
              + className + ". It's not a " + baseClass.getName() + "."));
          }
      } catch(Exception e) {
        try { conn = (Connection)baseClass.newInstance(); }
        catch (Exception x) { x.printStackTrace(); return null; }
        fireMessage(new MessageEvent(this,"Cannot instantiate object."
          + ". A " + e + " was raised. Base class is used.",
          MessageEvent.WARNING));
      }
    }
    return conn;
  }

  /** Helper function that instantiates a graphical object using a specified class
   *  and the 'class' attribute. In addition it will also set 'name' and
   *  GraphManager properties. */
  private Graph instGraphFromName(Attributes atts, Class baseClass) {
    Graph graph;
    String className;
    String classAtt = atts.getValue("class");
    if (classAtt != null) {
      className = classMap.getProperty(classAtt);
    } else {
      className = null;
    }
    if (className == null) {
      // nope; instantiate base class
      try {
        graph = (Graph)baseClass.newInstance();
      } catch (Exception e) { e.printStackTrace(); return null; }
    } else {
      // yes; try to instantiate specified class.
      Object object = null;
      try {
        object = Class.forName(className).newInstance();
        // check if this object is an instance of baseClass
        if (baseClass.isInstance(object)) {
          graph = (Graph)object;
        } else {
        try { graph = (Graph)baseClass.newInstance(); }
        catch (Exception e) { e.printStackTrace(); return null; }
          fireMessage(new MessageGraph(graph,"Cannot instantiate object "
            + className + ". It's not a " + baseClass.getName() + "."));
        }
      } catch(Exception e) {
        try { graph = (Graph)baseClass.newInstance(); }
        catch (Exception x) { x.printStackTrace(); return null; }
        fireMessage(new MessageGraph(graph,"Cannot instantiate object."
          + ". A " + e + " was raised. Base class is used.",
          MessageEvent.WARNING));
      }
    }
    graph.setGraphManager(cGraph.getGraphManager());
    int index;
    if ((index = atts.getIndex("name")) != -1) {
      graph.setName(atts.getValue(index));
    }
    return graph;
  }

  /** Create physical connections between Graph objects using the
   *  NameConnection array. */
  private void createConnections ()
  {
    String srcName;
    String destName;
    Object sourceGraph;
    Object destGraph;
    NameConnection nameCon;
    Enumeration keys;
    for (int i=0; i<conList.size(); i++)
    {
      nameCon = (NameConnection)conList.get(i);

      sourceGraph = null;
      destGraph = null;

      keys = graphtable.keys();

      int destHitCount = 0;
      int srcHitCount = 0;
      while (keys.hasMoreElements()) {
        Reference r = (Reference)keys.nextElement();
        if (r.equals(nameCon.source)) {
          sourceGraph = graphtable.get(r);
          srcHitCount++;
        }
        if (r.equals(nameCon.dest)) {
          destGraph = graphtable.get(r);
          destHitCount++;
        }
      }

      if (srcHitCount > 1) {
        fireMessage(new MessageGraph(sourceGraph,
          "Cannot connect " + nameCon.source + " with " + nameCon.dest
            + " (reference to " + nameCon.source + " is ambiguous).",
            MessageEvent.WARNING));
        if (destHitCount <= 1) continue;
      }
      if (destHitCount > 1) {
        fireMessage(new MessageGraph(destGraph,
          "Cannot connect " + nameCon.source + " with " + nameCon.dest
            + " (reference to " + nameCon.dest + " is ambiguous).",
            MessageEvent.WARNING));
        continue;
      }

      if (sourceGraph == null) {
        if (destGraph != null) {
          fireMessage(new MessageGraph(destGraph,
                      "Cannot connect "+nameCon.source+" with " + nameCon.dest
                      + " (the referenced object does not exist, or is not in scope).",
                      MessageEvent.WARNING));
        } else {
          fireMessage(new MessageEvent(this,
                      "Cannot connect "+nameCon.source+" with " + nameCon.dest
                      + " (the referenced objects do not exist, or is not in scope).",
                      MessageEvent.WARNING));
        }
        continue;
      }

      if (destGraph == null) {
        if (sourceGraph != null) {
          fireMessage(new MessageGraph(sourceGraph,
                      "Cannot connect "+sourceGraph+" with " + nameCon.dest
                      + " (the referenced object does not exist).",
                      MessageEvent.WARNING));
        } else {
          fireMessage(new MessageEvent(this,
                      "Cannot connect "+nameCon.source+" with " + nameCon.dest
                      + " (the referenced objects do not exist).",
                      MessageEvent.WARNING));
        }
        continue;
      }

      if (sourceGraph.toString() == destGraph.toString()) {

        fireMessage(new MessageGraph(sourceGraph,"Cannot connect "+sourceGraph
                    + " with itself.",MessageEvent.WARNING));
        continue;
      }

      if (sourceGraph instanceof GraphStep) {
        if (!(destGraph instanceof GraphStep)) {
          fireMessage(new MessageGraph(sourceGraph,destGraph,
                      "Cannot connect "+sourceGraph+" with " + destGraph
                      + " (the elements are incompatible).",
                      MessageEvent.WARNING));
          continue;
        }
        GraphStep sStep = (GraphStep)sourceGraph;
        GraphStep dStep = (GraphStep)destGraph;
        if ( (sStep.getOwner().getOwner() != dStep) &&
             (dStep.getOwner().getOwner() != sStep) &&
             (sStep.getOwner() != dStep.getOwner())) {
          fireMessage(new MessageGraph(sStep,dStep,"Cannot connect "
                      + sourceGraph + " with "
                      + destGraph + " (they have different parents).",
                      MessageEvent.WARNING));
          continue;
        }

        Connection c = instConnectionFromName(nameCon.getItsClass(),
                                              Connection.class);
        sStep.connectTo(dStep);
        continue;
      }

      if (sourceGraph instanceof GraphDataHolder) {
        if (!(destGraph instanceof GraphDataHolder)) {
          fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                      + sourceGraph + " with " + destGraph
                      + " (the elements are incompatible).",
                      MessageEvent.WARNING));
          continue;
        }
        GraphDataHolder sDH = (GraphDataHolder)sourceGraph;
        GraphDataHolder dDH = (GraphDataHolder)destGraph;
        boolean sgin = sDH.isInput();
        boolean dgin = dDH.isInput();

        if ( (sDH.getOwnerStep() !=
          (GraphStep)dDH.getOwnerStep().getOwnerSimul()) &&
          (dDH.getOwnerStep() !=
          (GraphStep)sDH.getOwnerStep().getOwnerSimul())) {

          // this is not a child->parent connection

          if (sgin) {
            fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                        + sourceGraph + " with " + destGraph
                        + " (Source should be of type input).",
                        MessageEvent.WARNING));
            continue;
          }

          if (!dgin) {
            fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                        + sourceGraph + " with " + destGraph
                        + " (Destination should be of type output).",
                        MessageEvent.WARNING));
            continue;
          }

          if (sgin == dgin) {
            fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                        + sourceGraph + " with " + destGraph
                        + " (DataHolders are of same type).",
                        MessageEvent.WARNING));
            continue;
          }

        } else {

          // this is a child->parent connection
          if (sgin != dgin) {
            fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                        + sourceGraph + " with " + destGraph
                        + " (Invalid child-parent connection).",
                        MessageEvent.WARNING));
            continue;
          }

          if (sgin) {
            if ( (sDH.getOwnerStep() !=
                 (GraphStep)dDH.getOwnerStep().getOwnerSimul()) ) {

              fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                          + sourceGraph + " with " + destGraph
                          + " (Invalid child-parent connection, reverse?).",
                          MessageEvent.WARNING));
              continue;
            }
          } else {
             if ( (dDH.getOwnerStep() !=
                 (GraphStep)sDH.getOwnerStep().getOwnerSimul()) ) {

              fireMessage(new MessageGraph(sourceGraph,destGraph,"Cannot connect "
                          + sourceGraph + " with " + destGraph
                          + " (Invalid child-parent connection, reverse?).",
                          MessageEvent.WARNING));
              continue;
            }
          }
        }

        if (causesLoop(sDH,dDH)) {
          if (noLoopWarning) {
            fireMessage(new MessageGraph(sourceGraph,destGraph,"Warning: Auto-"
                        + "layout of simulations with loops are not"
                        + " well supported!",
                        MessageEvent.ERROR));
            noLoopWarning = false;
          }
        }
        Connection c = instConnectionFromName(nameCon.getItsClass(),
                                              Connection.class);
        ((Graph)sourceGraph).connectTo((Graph)destGraph,c);
      }
    }
  }

  /** Will a connection of source to dest result in a loop? */
  private boolean causesLoop (GraphDataHolder source, GraphDataHolder dest) {
    if (source.isInput() &&
        (source.getOwner().getOwner() == dest.getOwner().getOwner())) {
      return false;                     // special case: internal dh connection
    }
    return findDataHolderInChain(source,dest);
  }

  /** This method returns true if a connection between to DataHolders results
   *  in a cyclic connection.*/
  private boolean findDataHolderInChain(GraphDataHolder theDH,
                                        GraphDataHolder tryChain) {
    GraphStep step = (GraphStep)tryChain.getOwner().getOwner();
    if (theDH.getOwner().getOwner() == step) return true;         // found it
    GraphDataHolder output[] = step.getOutputDataHolders();
    for (int i=0; i<output.length; i++) {
      if (output[i].hasOutputConnections()) {
        if (findDataHolderInChain(theDH,
            (GraphDataHolder)output[i].getOutConnection(0).graphIn)) {
          return true;
        }
      }
    }
    return false;
  }

  private void initDistColors() {
    distColor = new Color[8];
    distColor[0] = new Color(MIN_C_INT,MIN_C_INT,MIN_C_INT);
    distColor[1] = new Color(MIN_C_INT,MIN_C_INT,MAX_C_INT);
    distColor[2] = new Color(MIN_C_INT,MAX_C_INT,MIN_C_INT);
    distColor[3] = new Color(MIN_C_INT,MAX_C_INT,MAX_C_INT);
    distColor[4] = new Color(MAX_C_INT,MIN_C_INT,MIN_C_INT);
    distColor[5] = new Color(MAX_C_INT,MIN_C_INT,MAX_C_INT);
    distColor[6] = new Color(MAX_C_INT,MAX_C_INT,MIN_C_INT);
    distColor[7] = new Color(MAX_C_INT,MAX_C_INT,MAX_C_INT);
  }

  private Color genColorFromNode(String node) {
    for (int i=0; i<nodeColorMap.size(); i++) {
      String n = ((NodeColor)nodeColorMap.get(i)).node;
      if (n.equals(node)) {
        return ((NodeColor)nodeColorMap.get(i)).color;
      }
    }
    Color baseColor;
    int baseColorIx = nodeColorMap.size()%8;
    baseColor = distColor[baseColorIx];
    int round = nodeColorMap.size()/8+1;
    // interpolate between base colors
    int r = distColor[baseColorIx].getRed();
    int g = distColor[baseColorIx].getGreen();
    int b = distColor[baseColorIx].getBlue();
    Color newColor = new Color(r,g,b);
    nodeColorMap.add(new NodeColor(node,newColor));
    return newColor;
  }

  /** Darkens specified color with CHILD_COLOR_ADJUST %. */
  private Color darker(Color color) {
    int r,g,b;
    r = (int)(color.getRed() * CHILD_COLOR_ADJUST);
    g = (int)(color.getGreen() * CHILD_COLOR_ADJUST);
    b = (int)(color.getBlue() * CHILD_COLOR_ADJUST);
    return new Color(r,g,b);
  }

  /** Dispatch the specified MessageEvent to all the registered listeners. */
  protected void fireMessage(MessageEvent event) {
    if (root.getGraphManager() != null) {
      root.getGraphManager().fireMessage(event);
    }
  }

  /** Register a MessageListener. All XML parser messages will be send to the
   *  listener. The messages contain extra information (e.g. line number) */
  public void addMessageListener (MessageListener messageListener) {
    _mlisteners.add(messageListener);
  }
  /** Unregisters the specified MessageListener */
  public void removeMessageListener (MessageListener messageListener) {
    _mlisteners.remove(messageListener);
  }

  public MessageListener[] getMessageListeners () {
    MessageListener ml [] = new MessageListener[_mlisteners.size()];
    for (int i=0; i<ml.length; i++) {
      ml[i] = (MessageListener)_mlisteners.get(i);
    }
    return ml;
  }

  private void addToPath(Graph graph) {
    if (cPath.length() == 0) {
      cPath = new String(graph.getName());
    } else {
      cPath += Reference.getDelimiter() + graph.getName();
    }
  }

  private void removeFromPath(Graph graph) {
    int i = cPath.toString().indexOf(graph.getName());
    if (i == -1) {
      System.out.println("ContHandlerSim.removeFromPath(): error");
      return;
    }
    StringBuffer temp = new StringBuffer(cPath);
    if (i == 0) {
      temp.setLength(i);
    } else {
      temp.setLength(i-1);
    }
    cPath = temp.toString();
  }

  private class RefGraph {
    public Graph graph;
    public Reference ref;
    RefGraph(Graph graph, Reference ref) {
      this.ref = ref;
      this.graph = graph;
    }
  }

  /** Structure in which unresolved connections can be placed. Connection
   *  names should be passed fully scoped (e.g. "mySimul.myDataHolder1") */
  private class NameConnection {
    public Reference source;
    public Reference dest;
    int rate;
    String itsClass;
    NameConnection (String s, String d, String scope) {
      source = new Reference(s,scope);
      dest = new Reference(d,scope);
      rate = -1;
    }
    /** Set the rate that will be set when the actual connection is made */
    public void setRate(int rate) { this.rate = rate; }
    public int getRate() { return rate; }
    public void setItsClass(String itsClass) { this.itsClass = itsClass; }
    public String getItsClass() { return itsClass; }
    public String getSourceName() { return source.getName(); }
    public String getDestName() { return dest.getName(); }
    public String toString() { return "NameConnection: "
                               + source + " -> " + dest; }
  }

  private class NodeColor {
    public String node;
    public Color color;
    NodeColor(String node, Color color) {
      this.node = node;
      this.color = color;
    }
  }
}

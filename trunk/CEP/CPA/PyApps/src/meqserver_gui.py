#!/usr/bin/python

import meqserver
from app_proxy_gui import *
from dmitypes import *
import app_pixmaps as pixmaps
import weakref
import math
import sets
import re
import meqds
import app_browsers 
from app_browsers import *

_dbg = verbosity(3,name='meqgui');
_dprint = _dbg.dprint;
_dprintf = _dbg.dprintf;

# global symbol: meqserver object; initialized when a meqserver_gui
# is constructed
mqs = None;

# ---------------- TODO -----------------------------------------------------
# Bugs:
#   Tree browser not always enabled! (Hello message lost??)
#   + In dmi_repr, array stats do not always use the right func
#   + Numarray indexing order is different! fixed converter
#   + Refresh of tree view with an empty forest list throws exception
#
# Minor fixes:
#   Create viewers as children of some widget (maybe a stack, again?) that
#     can be thrown out on exception together with any unclean children.
#   Do not compare result records, only state updates (and only if too recent)
#   Improve result labels, name them snapshots
#   Disorderly thread error or SEGV on exit
#   Why can't we exit with CTRL+C?
#   + Disallow drag-and-drop from a viewer onto the same cell
#   + Fix comparison of records and snapshots in Node Browser
#   + Enable drop on "show viewer" button
#
# Enhancements:
#   When first switching to the Trees panel, and have connection, and no nodelist
#     is loaded, request one
#   If all nodes are de-published, notify the browser somehow   
#   Context menu for Tree Browser
#   Option to specify udi directly in HierBrowser
#   Drop of a dataitem can create a cell with multiple items (think,
#       e.g., several 1D plots), if the viewer object supports it.
#   User-defined node groups in tree viewer
#   + Enhanced 'verbosity' interface (look for option parsing modules?)
#   + Right-button actions
#   + Enable views/drags/drops of sub-items (i.e. "nodestate:name/cache_result")
#   + Viewer plugin interface
#   + Update contents of HierBrowser on-the-fly, without closing expanded
#     sub-items (good for, e.g., node state updates)
#   + When looking at node state, open something useful by default (i.e.,
#     cache_result/vellsets/0 or smth)
#   + drag-and-drop
# ---------------------------------------------------------------------------

class NodeBrowser(HierBrowser,BrowserPlugin):
  _icon = pixmaps.treeviewoblique;
  viewer_name = "Node Browser";
  
  def __init__(self,parent,dataitem=None,default_open=None,**opts):
    HierBrowser.__init__(self,parent,"value","field",
        udi_root=(dataitem and dataitem.udi));
    # parse the udi
    (name,ni) = meqds.parse_node_udi(dataitem.udi);
    if ni is None:
      node = meqds.nodelist[name or dataitem.data.name];
    else:
      node = meqds.nodelist[ni];
    self._default_open = default_open;
    self._state = None;
    # at this point, _node is a very basic node record: all it has is a list
    # of children nodeindices, to which we'll dispatch update requests
    # construct basic view items
    lv = self.wlistview();
    self.set_udi_root(dataitem.udi);
    # Node state
    self._item_state = HierBrowser.Item(lv,'Current state','',udi=dataitem.udi,udi_key='state');
    # Node children
    # note that dataitem.data may be a node state or a node stub record,
    # depending on whether it is already available to us, so just to make sure
    # we always go back to meqds for the children list
    if len(node.children):
      childroot = HierBrowser.Item(lv,'Children (%d)'%len(node.children),'',udi_key='child');
      self._child_items = {};
      for (cid,child) in node.children: 
        # this registers out callback for whenever a child's state is sent over
        meqds.subscribe_node_state(child,self.set_child_state);
        # this initiates a state request for the child
        meqds.request_node_state(child);
        # create a listview item for that child
        self._child_items[child] = HierBrowser.Item(childroot,str(cid),'#'+str(child));
    else:
      self._child_items = None;
    # State snapshots
    meqds.subscribe_node_state(ni,self.set_own_state);
    sslist = meqds.get_node_snapshots(ni);
    self._item_snapshots = HierBrowser.Item(lv,'','',udi_key='snapshot');
    self._last_snapshot = None;
    nss = 0;
    for (stateref,event,timestamp) in sslist:
      st = stateref();
      if st is not None:
        item = HierBrowser.Item(self._item_snapshots, \
                time.strftime('%H:%M:%S',time.localtime(timestamp)),\
                str(event),udi_key=str(nss));
        item.cache_content(st);
        nss += 1;
    self._item_snapshots.setText(0,'Snapshots (%d)'%nss);
    # If we already have a full state record, go use it
    # Note that this will not always be the case; in the general case,
    # the node state will arrive later (perhaps even in between child
    # states)
    if dataitem.data is not None:
      self.set_data(dataitem);
    lv.setCurrentItem(None);

  # our own state is added to snapshots here (and to the main view
  # in set_data below)
  def set_own_state (self,state,event):
    if state is self._last_snapshot or getattr(state,'__nochange',False):
      return;
    # add snapshot
    nss = self._item_snapshots.childCount();
    item = HierBrowser.Item(self._item_snapshots, \
           time.strftime('%H:%M:%S'),str(event),udi_key=str(nss));
    item.cache_content(state);
    self._last_snapshot = state;
    # change label on snapshots item
    self._item_snapshots.setText(0,'Snapshots (%d)'%(nss+1));
      
  # this callback is registered for all child node state updates
  def set_child_state (self,node,event):
    _dprint(3,'Got state for child',node.name,node.field_names());
    _dprint(3,'Event is',event);
    if not self._child_items:
      raise RuntimeError,'no children expected for this node';
    item = self._child_items.get(node.nodeindex,None);
    if not item:
      raise ValueError,'this is not our child';
    # store node name in item
    item.setText(2,"%s (%s)"%(node.name,node['class'].lower()));
    item.set_udi(meqds.node_udi(node));
    self.change_item_content(item,node,\
      make_data=curry(makeNodeDataItem,node));
    
  def set_data (self,dataitem,default_open=None,**opts):
    # open items (use default first time round)
    openitems = default_open or self._default_open;
    if self._state is not None:
      # do nothing if state has already been marked as unchanged
      if getattr(dataitem.data,'__nochange',False):
        return;
      # if something is already open, use that
      openitems = self.get_open_items() or openitems;
    # at this point, dataitem.data is a valid node state record
    _dprint(3,'Got state for node',dataitem.data.name,dataitem.data.field_names());
    self.change_item_content(self._item_state,dataitem.data,viewable=False);
    # apply saved open tree
    self.set_open_items(openitems);
    self._state = dataitem.data;
    
class TreeBrowser (object):
  def __init__ (self,parent):
    self._parent = weakref.proxy(parent);
    # construct GUI
    nl_vbox = self._wtop = QVBox(parent);
    nl_control = QWidget(nl_vbox);
    nl_control_lo = QHBoxLayout(nl_control);
    # add refresh button
    self._nl_update = nl_update = QToolButton(nl_control);
    nl_update.setIconSet(pixmaps.refresh.iconset());
    nl_update.setAutoRaise(True);
    nl_update.setDisabled(True);
    QToolTip.add(nl_update,"refresh the node list");
    #    nl_update.setMinimumWidth(30);
    #    nl_update.setMaximumWidth(30);
    nl_control_lo.addWidget(nl_update);
    nl_label = QLabel("Tree Browser",nl_control);
    nl_control_lo.addWidget(nl_label);
    nl_control_lo.addStretch();
    QObject.connect(nl_update,SIGNAL("clicked()"),self._request_nodelist);
    # node list
    self._nlv = nlv = DataDraggableListView(nl_vbox);
    nlv.setRootIsDecorated(True);
    nlv.setTreeStepSize(12);
    # nlv.setSorting(-1);
    nlv.setResizeMode(QListView.NoColumn);
    nlv.setFocus();
    nlv.connect(nlv,SIGNAL('expanded(QListViewItem*)'),self._expand_node);
    nlv.connect(nlv,SIGNAL('mouseButtonClicked(int,QListViewItem*,const QPoint &,int)'),
                     self._node_clicked);
    nlv.connect(nlv,SIGNAL('contextMenuRequested(QListViewItem*,const QPoint &,int)'),
                     self._show_context_menu);
    # map the get_data_item method
    nlv.get_data_item = self.get_data_item;
    
  # init empty set of node actions
  _node_actions = [];
  
  def add_node_action (action):
    """Registers a node action for the tree browser context menu.""";
    TreeBrowser._node_actions.append(action);
  add_node_action = staticmethod(add_node_action);
    
  def get_data_item (self,udi):
    (name,ni) = meqds.parse_node_udi(udi);
    if ni is None:
      if name is None:
        return None;
      if not len(name):
        raise ValueError,'bad udi (either name or nodeindex must be supplied): '+udi;
      node = meqds.nodelist[name];
    else:
      try: 
        node = meqds.nodelist[ni];
      except ValueError: # can't convert nodeindex to int: malformed udi
        raise ValueError,'bad udi (nodeindex must be numeric): '+udi;
    # create and return dataitem object
    return makeNodeDataItem(node);
 
  def wtop (self):
    return self._wtop;
    
  def clear (self):
    self._nlv.clear();
    
  def connected (self,conn):
    self._nl_update.setDisabled(not conn);

  def _request_nodelist (self):
    self._parent.mqs.meq('Get.Node.List',meqds.NodeList.RequestRecord,wait=False);
    
  def make_node_item (self,node,name,parent,after):
    item = QListViewItem(parent,after,name);
    item.setText(self._icol_class,str(node.classname));
    item.setText(self._icol_index,str(node.nodeindex));
    item.setDragEnabled(True);
    item._node = weakref.proxy(node);
    item._expanded = False;
    item._udi  = meqds.node_udi(node);
    if node.children:
      item.setExpandable(True);
    self._node_items.setdefault(node.nodeindex,[]).append(item);
    self.update_node_item(item);
    return item;

  def update_nodelist (self):
    # init columns if calling for the first time
    # (we don't do it in the constructor because the number of columns
    # depends on the available node actions)
    # init one column per registered action; first one is always node name
    if not hasattr(self,'_node_items'): 
      self._node_actions = TreeBrowser._node_actions;
      self._nlv.addColumn('node');
      for (num,act) in enumerate(self._node_actions):
        if act.display:
          act._icol = num;
          if num:
            self._nlv.addColumn('');
      # add class and index columns
      self._icol_class = self._nlv.columns();
      self._nlv.addColumn('class');
      self._icol_index = self._nlv.columns();
      self._nlv.addColumn('index');
      for icol in range(self._nlv.columns()):
        self._nlv.setColumnWidthMode(icol,QListView.Maximum);
    # reset the nodelist view
    nodelist = meqds.nodelist;
    self._node_items = {};
    self._nlv.clear();
    all_item  = QListViewItem(self._nlv,"All Nodes (%d)"%len(nodelist));
    all_item._iter_nodes = nodelist.iternodes();
    all_item.setExpandable(True);
    rootnodes = nodelist.rootnodes();
    rootitem  = QListViewItem(self._nlv,all_item,"Root Nodes (%d)"%len(rootnodes));
    rootitem._iter_nodes = iter(rootnodes);
    rootitem.setExpandable(True);
    classes = nodelist.classes();
    cls_item  = item = QListViewItem(self._nlv,rootitem,"By Class (%d)"%len(classes));
    for (cls,nodes) in classes.iteritems():
      if len(nodes) == 1:
        item = self.make_node_item(nodes[0],nodes[0].name,cls_item,item);
      else:
        item = QListViewItem(cls_item,item,"(%d)"%len(nodes));
        item.setText(self._icol_class,cls);
        item.setExpandable(True);
        item._iter_nodes = iter(nodes);

  def update_node_state (self,nodestate):
    items = self._node_items.get(nodestate.nodeindex,[]);
    for item in items:
      self.update_node_item(item);
      
  def update_node_item (self,item):
    node = item._node;
    # update status pixmaps
    for act in self._node_actions:
      # if action has a column assigned to it, and a state callback...
      try: (icol,state) = (act._icol,act.state);
      except AttributeError: pass;
      else:
        # cll it to set the pixmap in the column
        if state(node):
          item.setPixmap(icol,act.iconset().pixmap());
        else:
          item.setPixmap(icol,QPixmap());
    # update context menu
    try: menu = item._context_menu;
    except AttributeError:
      return;
    for (act_id,act) in menu._actions.iteritems():
      if hasattr(act,'state'):
        menu.setItemChecked(act_id,act.state(node));
  
  # slot: called to show a context menu for a browser item
  def _show_context_menu (self,item,point,col):
    try: menu = item._context_menu;
    except AttributeError:
      # if item does not have a _node attribute, exit without a menu
      try: node = item._node;
      except AttributeError:
        item._context_menu = None;
        return;
      # create menu on the fly when first called for this item
      menu = item._context_menu = QPopupMenu();
      menu._callbacks = [];
      # insert title
      menu.insertItem("%s: %s"%(node.name,node.classname));
      # insert viewer list submenus
      viewer_list = gridded_workspace.getViewerList(meqds.NodeClass(node.classname));
      if viewer_list: 
        menu.insertSeparator();
        # create display submenus
        menu1 = item._display_menu1 = QPopupMenu();
        menu2 = item._display_menu2 = QPopupMenu();
        menu.insertItem(pixmaps.view_split.iconset(),"Display with",menu1);
        menu.insertItem(pixmaps.view_right.iconset(),"New display with",menu2);
        for v in viewer_list:
          # create entry for viewer
          name = getattr(v,'viewer_name',v.__name__);
          try: icon = v.icon();
          except AttributeError: icon = QIconSet();
          # add entry to both menus ("Display with" and "New display with")
          cb1 = xcurry(self.wtop().emit,(PYSIGNAL("view_node()"),(node,v)),_argslice=slice(0));
          cb2 = xcurry(self.wtop().emit,(PYSIGNAL("view_node()"),(node,v,dict(newcell=True))),_argslice=slice(0));
          menu1.insertItem(icon,name,cb1);
          menu2.insertItem(icon,name,cb2);
          menu._callbacks.append(cb1);
          menu._callbacks.append(cb2);
      # add node actions
      if self._node_actions:
        menu._actions = {};
        menu.insertSeparator();
        for act in self._node_actions:
          if act.applies_to(node):
            act_id = act.add_to_menu(menu,node);
            # add to action map
            menu._actions[act_id] = act;
    # display menu if defined
    if menu is not None:
      menu.exec_loop(point);
      
  def _node_clicked (self,button,item,point,col):
    if button == 1 and hasattr(item,'_node'):
      self.wtop().emit(PYSIGNAL("view_node()"),(item._node,None));
  
  def _expand_node (self,item):
    i1 = item;
    # populate list when first opened, if an iterator is present as an attribute
    try: iter_nodes = item._iter_nodes;
    except: pass;
    else:
      for node in iter_nodes:
        i1 = self.make_node_item(node,node.name,item,i1);
      delattr(item,'_iter_nodes');
    # populate node children when first opened
    try: node = item._node;
    except: pass;
    else:
      if not item._expanded:
        for (key,ni) in item._node.children:
          node = meqds.nodelist[ni];
          name = str(key) + ": " + node.name;
          i1 = self.make_node_item(node,name,item,i1);
        item._expanded = True;
  _expand_node = busyCursorMethod(_expand_node);
  
class NodeAction (object):
  """NodeAction is a class describing a node-associated action.
  """;
  # these class attributes are meant to be redefined by subclasses
  text = 'unknown action';
  iconset = None;
  nodeclass = None;  # None means all nodes; else assign meqds.NodeClass('class');
  def applies_to_all (self):
    return self.nodeclass is None;
  def applies_to (self,node):
    return self.applies_to_all() or issubclass(meqds.NodeClass(node.nodeclass),self.nodeclass);
  def add_to_menu (self,menu,node):
    try: _callbacks = menu._callbacks;
    except AttributeError:
      _callbacks = menu._callbacks = [];
    cb = xcurry(self.activate,(node,),_argslice=slice(0));
    menu._callbacks.append(cb);
    if self.iconset:
      return menu.insertItem(self.iconset(),self.text,cb);
    else:
      return menu.insertItem(self.text,cb);
  def activate (self,node):
    raise "activate() not defined in NodeAction "+str(type(self));
    
class NodeToggleAction (NodeAction):
  # if true, toggle state is included in the TreeBrowser view
  display = True;
  def add_to_menu (self,menu,node):
    act_id = NodeAction.add_to_menu(self,menu,node);
    menu.setItemChecked(act_id,self.state(node));
    return act_id;
  def state (self,node):
    raise "state() not defined in NodeToggleAction "+str(type(self));
    
class NTA_NodeDisable (NodeToggleAction):
  text = "Disable";
  iconset = pixmaps.cancel.iconset;
  state = lambda self,node:not node.is_active();
  def activate (self,node):
    cs = node.control_state ^ meqds.CS_ACTIVE;
    meqds.set_node_state(node,control_state=cs);

class NTA_NodePublish (NodeToggleAction):
  text = "Publish";
  iconset = pixmaps.publish.iconset;
  state = lambda self,node:node.is_publishing();
  def activate (self,node):
    cmd = srecord(nodeindex=node.nodeindex,get_state=True,enable=not node.is_publishing());
    mqs.meq('Node.Publish.Results',cmd,wait=False);

TreeBrowser.add_node_action(NTA_NodeDisable());
TreeBrowser.add_node_action(NTA_NodePublish());
                       
class meqserver_gui (app_proxy_gui):
  def __init__(self,app,*args,**kwargs):
    meqds.set_meqserver(app);
    global mqs;
    self.mqs = mqs = app;
    self.mqs.track_results(False);
    # init standard proxy GUI
    app_proxy_gui.__init__(self,app,*args,**kwargs);
    # add handlers for result log
    self._add_ce_handler("node.result",self.ce_NodeResult);
    self._add_ce_handler("app.result.node.get.state",self.ce_NodeState);
    self._add_ce_handler("app.result.get.node.list",self.ce_LoadNodeList);
    self._add_ce_handler("hello",self.ce_mqs_Hello);
    self._add_ce_handler("bye",self.ce_mqs_Bye);
    
  def populate (self,main_parent=None,*args,**kwargs):
    app_proxy_gui.populate(self,main_parent=main_parent,*args,**kwargs);
    self.set_verbose(self.get_verbose());
    _dprint(2,"meqserver-specifc init"); 
    # add Tree browser panel
    self.treebrowser = TreeBrowser(self);
    self.maintab.insertTab(self.treebrowser.wtop(),"Trees",1);
    self.connect(self.treebrowser.wtop(),PYSIGNAL("view_node()"),self._view_node);
    
    # add Result Log panel
    self.resultlog = Logger(self,"node result log",limit=1000,
          udi_root='noderes');
    self.maintab.insertTab(self.resultlog.wtop(),"Results",2);
    self.resultlog.wtop()._default_iconset = QIconSet();
    self.resultlog.wtop()._default_label   = "Results";
    self.resultlog.wtop()._newres_iconset  = pixmaps.check.iconset();
    self.resultlog.wtop()._newres_label    = "Results";
    self.resultlog.wtop()._newresults      = False;
    QWidget.connect(self.resultlog.wlistview(),PYSIGNAL("displayDataItem()"),self.display_data_item);
    QWidget.connect(self.maintab,SIGNAL("currentChanged(QWidget*)"),self._reset_resultlog_label);

  # override handleAppEvent to catch node state updates, whichever event they
  # may be in
  def handleAppEvent (self,ev,value):
    # update node state
    if isinstance(value,record):
      try: 
        state = value.node_state;
        name  = state.name;
      except AttributeError: pass;
      else: 
        _dprint(5,'got state for node ',name);
        self.update_node_state(state,ev);
    # call top-level handler
    app_proxy_gui.handleAppEvent(self,ev,value);
    
  def ce_mqs_Hello (self,ev,value):
    self.treebrowser.clear();
    self.treebrowser.connected(True);  
    self.resultlog.clear();
    wtop = self.resultlog.wtop();
    self.maintab.changeTab(wtop,wtop._default_iconset,wtop._default_label);
    
  def ce_mqs_Bye (self,ev,value):
    app_proxy_gui.ce_Hello(self,ev,value);
    self.treebrowser.connected(False);  
    
  def ce_NodeState (self,ev,value):
    if hasattr(value,'name'):
      _dprint(5,'got state for node ',value.name);
      self.update_node_state(value,ev);
  
  def ce_NodeResult (self,ev,value):
    # no need to update anymore: handleAppEvent() does it for us automagically
    #     self.update_node_state(value,ev);
    if self.resultlog.enabled:
      txt = '';
      name = ('name' in value and value.name) or '<unnamed>';
      cls  = ('class' in value and value['class']) or '?';
      rqid = 'request_id' in value and str(value.request_id);
      txt = ''.join((name,' <',cls.lower(),'>'));
      desc = 'result';
      if rqid:
        txt = ''.join((txt,' rqid:',rqid));
        desc = desc + ':' + rqid;
      self.resultlog.add(txt,content=value,category=Logger.Event, 
        name=name,desc=desc,viewopts=_defaultResultViewopts);
      wtop = self.resultlog.wtop();
      if self.maintab.currentPage() is not wtop and not wtop._newresults:
        self.maintab.changeTab(wtop,wtop._newres_iconset,wtop._newres_label);
        wtop._newresults = True;
        
  def ce_LoadNodeList (self,ev,meqnl):
    try:
      meqds.nodelist.load(meqnl);
    except ValueError:
      _dprint(2,"got nodelist but it is not valid, ignoring");
      return;
    _dprintf(2,"loaded %d nodes into nodelist\n",len(meqds.nodelist));
    self.treebrowser.update_nodelist();
      
  def update_node_state (self,node,event=None):
    meqds.reclassify_nodestate(node);
    meqds.add_node_snapshot(node,event);
    udi = meqds.node_udi(node);
    self.gw.update_data_item(udi,node);
    self.treebrowser.update_node_state(node);
    
  def _view_node (self,node,viewer=None,kws={}):
    _dprint(2,"node clicked, adding item");
    self.gw.add_data_item(makeNodeDataItem(node,viewer),**kws);
    self.show_gridded_workspace();
    
  def _reset_resultlog_label (self,tabwin):
    if tabwin is self.resultlog.wtop() and tabwin._newresults:
      self._reset_maintab_label(tabwin);
    tabwin._newresults = False;

def makeNodeDataItem (node,viewer=None,viewopts={}):
  """creates a GridDataItem for a node""";
  udi = meqds.node_udi(node);
  nodeclass = meqds.NodeClass(node);
  vo = viewopts.copy();
  vo.update(_defaultNodeViewopts);
  # curry is used to create a call for refreshing its state
  return GridDataItem(udi,(node.name or '#'+str(node.nodeindex)),
            desc=nodeclass.__name__,data=None,datatype=nodeclass,
            refresh=curry(meqds.request_node_state,node),
            viewer=viewer,viewopts=vo);



_default_state_open =  ({'cache_result':({'vellsets':None},None), \
                        'request':None },None);

_defaultNodeViewopts = { \
  RecordBrowser: { 'default_open': _default_state_open },
  NodeBrowser:   { 'default_open': ({'state':_default_state_open},None) } };

_defaultResultViewopts = { \
  RecordBrowser: { 'default_open': _default_state_open }, \
  };


gridded_workspace.registerViewer(meqds.NodeClass(),NodeBrowser,priority=10);

# register reloadables
reloadableModule(__name__);
# reloadableModule('meqds');


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

package org.astron.util.gui;

/**
 * Title: Graphical Notifier
 * Description: Swing component that can display messages.
 * Copyright:    Copyright (c) 2001
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import javax.swing.*;
import javax.swing.event.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.plaf.basic.*;

public class JNotifier extends JPanel implements MessageListener {

  /** Default message icon */
  final static ImageIcon defaultIcon = new ImageIcon("images/copy16.gif");

  //private MessageListModel _model = new MessageListModel();
  private MessageListModel _model = new MessageListModel();
  public JList _list;
  private JScrollPane _scrollPane;
  JNotifier _this;

  public JNotifier() {
    _list = new JList(_model);
    _list.setVisibleRowCount(6);
    _list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    _list.addMouseListener(new MyMouseListener());
    _list.setCellRenderer(new MyCellRenderer());
    _scrollPane = new JScrollPane(_list,
      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
      JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    setLayout(new GridLayout(1,1));
    add(_scrollPane);
    _this = this;
  }

  public void note(MessageEvent event) {
    _model.addMessageEvent(event);
    _list.ensureIndexIsVisible(_model.getSize()-1);
  }

  public void clearAll() { _model.clearAll(); }

  /** The Data Model for the JList */
  private class MessageListModel extends DefaultListModel {
    ArrayList events;
    MessageListModel () { events = new ArrayList(30); }
    public void addMessageEvent (MessageEvent event) {
      events.add(event);
      EventListener el[]=getListeners(ListDataListener.class);
      fireContentsChanged(this,0,events.size()-1);
      fireIntervalAdded(this,0,events.size()-1);
    }
    public void clearAll() {
      events.clear();
      fireIntervalRemoved(this,0,events.size());
    }
    public void clearInvalid() {
      boolean change = false;
      for (int i=0; i<events.size(); i++) {
        if (getMessageEventAt(i).valid()) {
          events.remove(i);
          change = true;
        }
      }
      if (change) fireIntervalRemoved(this,0,events.size()-1);
    }
    public MessageEvent getMessageEventAt(int index) {
      return (MessageEvent)events.get(index);
    }
    public Object getElementAt(int index) {
      return events.get(index);
    }
    public int getSize() {
      return events.size();
    }
  }

  /** Handles mouse clicks on the JList */
  private class MyMouseListener implements MouseListener {
    public void mouseClicked(MouseEvent e) {
      if (((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0)
            && (e.getClickCount()==2)) {
        // left button double-clicked
        JList list = (JList)e.getSource();
        int ix = list.locationToIndex(e.getPoint());
        if (ix != -1) {
          MessageEvent mEvent = (MessageEvent)
                                list.getModel().getElementAt(ix);
          mEvent.onClick(_this);             // take message specific action
        }
      }
    }
    public void mouseEntered(MouseEvent e) {
    }
    public void mouseExited(MouseEvent e) {
    }
    public void mousePressed(MouseEvent e) {
    }
    public void mouseReleased(MouseEvent e) {
    }
  }

  /** Calls MessageEvent onClick() handler when a list item is selected */
  private class SelectionListener implements ListSelectionListener {
    public void valueChanged(ListSelectionEvent e) {
      if (e.getValueIsAdjusting()) return;
      JList list = (JList)e.getSource();
      if (list.isSelectionEmpty()) return;
      MessageEvent mevent = (MessageEvent)list.getSelectedValue();
      if (mevent == null) {
        System.err.println("MessageEvent==null in SelectionListener");
        return;
      }
      mevent.onClick(_this); // take message specific action.
    }
  }

  /** Custom JList item renderer that renders an icon followed by a string */
  class MyCellRenderer extends JLabel implements ListCellRenderer {

     public MyCellRenderer () {
        setIconTextGap(3);
        setOpaque(false);
     }

     // We just reconfigure the JLabel each time we're called.
     public Component getListCellRendererComponent(JList list,Object value,
      int index,boolean isSelected,boolean cellHasFocus)
     {
        if (! (value instanceof MessageEvent) ) {
          setText(value.toString());
          return this;
        }
        MessageEvent event = (MessageEvent)value;
        setText(event.getMessage());
        setIcon((event.getIcon() == null) ? defaultIcon : event.getIcon());
   	if (isSelected) {
          setBackground(list.getSelectionBackground());
	  setForeground(list.getSelectionForeground());
          setOpaque(true);
	} else {
          setOpaque(false);
          if (event.getType() == event.ERROR) setForeground(Color.red);
          else if (event.getType() == event.WARNING) setForeground(Color.blue);
          else setForeground(Color.black);
	  setBackground(list.getBackground());
        }
        setEnabled(list.isEnabled());
	setFont(list.getFont());
        return this;
     }
 }
}
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
import java.awt.event.*;
import java.util.Vector;

// JFC 1.0.X
/*import com.sun.java.swing.*;
import com.sun.java.swing.event.ListSelectionListener;*/

// JFC 1.1.X
import javax.swing.*;
import javax.swing.event.ListSelectionListener;

/**
 * This component contains extended functionality that JList doesn't have. It's
 * written so that you only have to change your JList declaration from JList to
 * JListEx. The rest of the changes are transparent. These are the extra
 * functionalities: <BR>
 * <UL>
 * <LI><B>DataTips</B>: When a cell isn't completely visible, it displays a
 * JToolTip when the user holds the cursor over it.</LI>
 * <LI><B>JScrollPane</B>: In order for the datatips to work, JListEx is already
 * in a JScrollPane. You don't have to add JListEx to a scrollpane like you had
 * to do for JList. You can get access to the scrollpane by using the
 * getScrollPane() method.</LI>
 * <LI><B>Background image:</B> You can change the background of the component
 * from a dull white color to a nice image. You can also set the component to
 * either scroll the image when you scroll the JList or to have the background
 * remain fixed while you scroll.</LI></UL>
 *
 * <B>Notes :</B><UL>
 * <LI>There's a bug in Swing 1.1 & 1.1.1 beta1 that causes the tooltips to be
 * be displayed as a panel. The result of this bug is that when JListEx is
 * located near the border of a Frame, the tooltip doesn't display outside the
 * frame. The location provided by list_getToolTipLocation isn't correctly used
 * to position the tooltip either. The workaround is to modify ToolTipManager.java
 * to use a WindowTip when the lightWeightPopupEnabled boolean is set to false.
 * This can be done by changing the code in the method <B>void showTipWindow()</B>
 * to the following code:<BR><PRE>
 *   if (insideComponent.getRootPane() == null){
 *      tipWindow = new WindowPopup((frameForComponent(insideComponent)),tip,size);
 *      heavyWeightPopupEnabled = true;
 *   }
 *   else if (lightWeightPopupEnabled){
 *      heavyWeightPopupEnabled = false;
 *      tipWindow = new JPanelPopup(tip,size);
 *   }
 *   else {
 *      // The following code has been changed due to a bug in the source from Sun.
 *      heavyWeightPopupEnabled = true;
 *      tipWindow = new WindowPopup((frameForComponent(insideComponent)),tip,size);
 *   }
 * <PRE></LI></UL>
 *
 * Any suggestions on extra functionality are welcome!
 *
 * @author Jeroen Zwartepoorte (<A HREF="mailto:Jeroen@xs4all.nl">Jeroen@xs4all.nl</A>)
 * @author used source by Zafir Anjum (<A HREF="http://www.codeguru.com/java/articles/123.shtml">www.codeguru.com/java/articles/123.shtml</A>)
 * @version 1.0 (13-02-1999)
 */
public class JListEx extends JComponent
{
    // Private variables.
	private JScrollPaneEx scrollpane = null;
    private JList list = null;
    private ImageIcon ImBackground = null;
    private int nDismissDelay = 0;
    private boolean bDataTips = false;
    private boolean bScrollableBackground = true;

    /**
     * Parameterless constructor.
     * #see javax.swing.JList#JList()
     */
	public JListEx()
	{
        	list = new JList();
        	init();
	}

    /**
     * Object array constructor.
     * #see javax.swing.JList#JList(Object[])
     */
    public JListEx(final Object[] listData)
    {
      list = new JList();
      init();
    }

    /**
     * Vector constructor.
     * #see javax.swing.JList#JList(Vector)
     */
    public JListEx(final Vector listData)
    {
    	list = new JList();
    	init();
    }

    /**
     * ListModel constructor.
     * #see javax.swing.JList#JList(ListModel)
     */
    public JListEx(ListModel dataModel)
    {
    	list = new JList();
    	init();
    }

    /**
     * Intialize class variables.
     */
    protected void init()
    {
    	// Set ToolTipManager to use heavyweight tooltips. (workaround for
        // bug in ToolTipManager.java in Swing 1.1 & Swing 1.1.1 beta1.
        // In order for workaround to work, you need to change ToolTipManager.java
        // and recompile ToolTipManager classes and replace them in Swingall.jar.
    	ToolTipManager.sharedInstance().setLightWeightPopupEnabled(false);

        // Set list opaque and fore & background values. These need to be set in
        // order for the ListCellRenderer to render the component without obscuring
        // the background image.
    	list.setOpaque(false);
        list.setBackground(null);
        list.setForeground(null);
        list.setCellRenderer(new DefaultTransparentListCellRenderer());

        // Setup layout.
    	scrollpane = new JScrollPaneEx(list);
    	this.setLayout(new BorderLayout());
        this.add(scrollpane, BorderLayout.CENTER);
    }

    /**
     * Returns wether the datatips are enabled.
     */
    public boolean getDataTips()
    {
    	return bDataTips;
    }

    /**
     * Enable or disable datatips.
     */
    public void setDataTips(boolean bDataTips)
    {
    	this.bDataTips = bDataTips;
        if (bDataTips)
        	list.setToolTipText("text");
        else
        	list.setToolTipText("");
    }

    /**
     * Returns scrollpane that contains JList.
     */
    public JScrollPaneEx getScrollPane()
    {
    	return scrollpane;
    }

    /**
     * This method is called when the JList is asked for the tooltip text.
     * JList.getToolTipText refers to this method. This method is declared here
     * because it needs access to the JScrollPane that the JList is in.
     */
    protected String list_getToolTipText(MouseEvent event)
    {
    	int idx = list.locationToIndex(event.getPoint());

        if (idx == -1)
        	return null;

        Object obj = list.getModel().getElementAt(idx);

        boolean bSelected = obj.equals(list.getSelectedValue());

        Component comp = list.getCellRenderer().getListCellRendererComponent(list,
        obj, idx, bSelected, bSelected);

        if (obj == null)
        	return null;
        if (obj.toString().equals(""))
        	return null;

        if (comp.getPreferredSize().width < scrollpane.getSize().width)
        	return null;

        return obj.toString();
    }

    /**
     * This method is called when the JList is asked for the position of the
     * tooltip. JList.getToolTipLocation refers to this method. This method is
     * declared here because it needs access to the JScrollPane that the JList
     * is in.
     */
    protected Point list_getToolTipLocation(MouseEvent event)
    {
    	int idx = list.locationToIndex(event.getPoint());

        if (idx == -1)
        	return null;

        Object obj = list.getModel().getElementAt(idx);

        boolean bSelected = obj.equals(list.getSelectedValue());

        Component comp = list.getCellRenderer().getListCellRendererComponent(list,
        obj, idx, bSelected, bSelected);

        if (obj == null)
        	return null;
        if (obj.toString().equals(""))
        	return null;

        if (comp.getPreferredSize().width < scrollpane.getSize().width)
        	return null;

        Point pt = list.getCellBounds(idx, idx).getLocation();
        if (pt == null)
        	return null;

        pt.translate(-2, -1);

        return pt;
    }

    /**
     * This method is called when the JList is repainted. The list_paint method
     * draws a background image when ImBackground != null and if the
     * bScrollableBackground is set to true. If that property is set to false,
     * the background image is painted by the JScrollPane that contains the
     * JList. It checks if the icon size != -1 to see if the image is really
     * loaded.
     */
    protected void list_paint(Graphics g)
    {
    	if (ImBackground != null)
        {
        	if (bScrollableBackground)
            {
            	// Make sure image is loaded.
            	if ((ImBackground.getIconWidth() == -1) ||
                    (ImBackground.getIconHeight() == -1))
                    return;

        		Dimension dim = list.getSize();

                // Tile image.
                for (int x = 0; x < dim.width; x += ImBackground.getIconWidth())
                	for (int y = 0; y < dim.height; y += ImBackground.getIconHeight())
                    	g.drawImage(ImBackground.getImage(), x, y, null, null);
            }
        }
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
     * This sets the background image and sets the scrollable property too. We
     * need to repaint the component after the properties are set.
     */
    public void setBackgroundImage(ImageIcon ImBackground, boolean bScrollableBackground)
    {
    	this.ImBackground = ImBackground;
        setScrollableBackground(bScrollableBackground);
    }

    /**
     * This returns the background image currently used for painting the
     * background of the JList.
     */
    public ImageIcon getBackgroundImage()
    {
    	return ImBackground;
    }

    /**
     * This sets wether the background image scrolls with the JList or if it
     * remains fixed.
     */
    public void setScrollableBackground(boolean bScrollableBackground)
    {
    	this.bScrollableBackground = bScrollableBackground;

        if (bScrollableBackground)
        	scrollpane.setBackgroundImage(null);
        else
        	scrollpane.setBackgroundImage(ImBackground);
        repaint();
    }

    /**
     * This returns wether the background scrolls with the JList.
     */
    public boolean getScrollableBackground()
    {
    	return bScrollableBackground;
    }

    /**
     * MouseListener methods. These methods change the dismissDelay integer
     * of the ToolTipManager to make sure the datatips don't disappear after
     * a while when you still have the mouse over them. This happens when the
     * mouse enters JListEx, and the value is set back to it's original value
     * on mouseExited.
     */
    public void this_mouseEntered(MouseEvent e)
    {
    	mouseEntered(e);
    }

    public void this_mouseExited(MouseEvent e)
    {
    	mouseExited(e);
    }

    public void mouseEntered(MouseEvent e)
    {
    	nDismissDelay = ToolTipManager.sharedInstance().getDismissDelay();
    	ToolTipManager.sharedInstance().setDismissDelay(Integer.MAX_VALUE);
    }

    public void mouseExited(MouseEvent e)
    {
    	ToolTipManager.sharedInstance().setDismissDelay(nDismissDelay);
    }

    public void mouseClicked(MouseEvent e) {}
    public void mousePressed(MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}


    /**
     * JList methods.
     *
     * These methods are declared here so that the programmer can use JListEx
     * like any normal JList. Since this class extends JComponent we need to
     * add all the public methods in JList here and call the appropriate method
     * in the JList.
     */
    public void addListSelectionListener(ListSelectionListener listener) {
    	list.addListSelectionListener(listener);
    }

    public void addSelectionInterval(int anchor, int lead) {
		list.addSelectionInterval(anchor, lead);
    }

    public void clearSelection() {
    	list.clearSelection();
    }

    public void ensureIndexIsVisible(int index) {
    	list.ensureIndexIsVisible(index);
    }

    public int getAnchorSelectionIndex() {
    	return list.getAnchorSelectionIndex();
    }

    public Rectangle getCellBounds(int index1, int index2) {
    	return list.getCellBounds(index1, index2);
    }

    public ListCellRenderer getCellRenderer() {
    	return list.getCellRenderer();
    }

    public int getFirstVisibleIndex() {
    	return list.getFirstVisibleIndex();
    }

    public int getFixedCellHeight() {
    	return list.getFixedCellHeight();
    }

    public int getFixedCellWidth() {
    	return list.getFixedCellWidth();
    }

    public int getLastVisibleIndex() {
    	return list.getLastVisibleIndex();
    }

    public int getLeadSelectionIndex() {
    	return list.getLeadSelectionIndex();
    }

    public int getMaxSelectionIndex() {
    	return list.getMaxSelectionIndex();
    }

    public int getMinSelectionIndex() {
    	return list.getMinSelectionIndex();
    }

    public ListModel getModel() {
    	return list.getModel();
    }

    public Dimension getPreferredScrollableViewportSize() {
    	return list.getPreferredScrollableViewportSize();
    }

    public Object getPrototypeCellValue() {
    	return list.getPrototypeCellValue();
    }

    public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
    	return list.getScrollableBlockIncrement(visibleRect, orientation, direction);
    }

    public boolean getScrollableTracksViewportHeight() {
    	return list.getScrollableTracksViewportHeight();
    }

    public boolean getScrollableTracksViewportWidth() {
    	return list.getScrollableTracksViewportWidth();
    }

    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) {
    	return list.getScrollableUnitIncrement(visibleRect, orientation, direction);
    }

    public int getSelectedIndex() {
    	return list.getSelectedIndex();
    }

    public int[] getSelectedIndices() {
    	return list.getSelectedIndices();
    }

    public Object getSelectedValue() {
    	return list.getSelectedValue();
    }

    public Object[] getSelectedValues() {
    	return list.getSelectedValues();
    }

    public Color getSelectionBackground() {
    	return list.getSelectionBackground();
    }

    public Color getSelectionForeground() {
    	return list.getSelectionForeground();
    }

    public int getSelectionMode() {
    	return list.getSelectionMode();
    }

    public ListSelectionModel getSelectionModel() {
    	return list.getSelectionModel();
    }

    public boolean getValueIsAdjusting() {
    	return list.getValueIsAdjusting();
    }

    public int getVisibleRowCount() {
    	return list.getVisibleRowCount();
    }

    public Point indexToLocation(int index) {
    	return list.indexToLocation(index);
    }

    public boolean isSelectedIndex(int index) {
    	return list.isSelectedIndex(index);
    }

    public boolean isSelectionEmpty() {
    	return list.isSelectionEmpty();
    }

    public int locationToIndex(Point location) {
    	return list.locationToIndex(location);
    }

    public void removeListSelectionListener(ListSelectionListener listener) {
    	list.removeListSelectionListener(listener);
    }

    public void removeSelectionInterval(int index0, int index1) {
    	list.removeSelectionInterval(index0, index1);
    }

    public void setCellRenderer(ListCellRenderer cellRenderer) {
    	list.setCellRenderer(cellRenderer);
    }

    public void setFixedCellHeight(int height) {
    	list.setFixedCellHeight(height);
    }

    public void setFixedCellWidth(int width) {
    	list.setFixedCellWidth(width);
    }

    public void setListData(final Object[] listData) {
    	list.setListData(listData);
    }

    public void setListData(final Vector listData) {
    	list.setListData(listData);
    }

    public void setModel(ListModel model) {
    	list.setModel(model);
    }

    public void setPrototypeCellValue(Object prototypeCellValue) {
    	list.setPrototypeCellValue(prototypeCellValue);
    }

    public void setSelectedIndex(int index) {
    	list.setSelectedIndex(index);
    }

    public void setSelectedIndices(int[] indices) {
    	list.setSelectedIndices(indices);
    }

    public void setSelectedValue(Object anObject,boolean shouldScroll) {
    	list.setSelectedValue(anObject, shouldScroll);
    }

    public void setSelectionBackground(Color selectionBackground) {
    	list.setSelectionBackground(selectionBackground);
    }

    public void setSelectionForeground(Color selectionForeground) {
    	list.setSelectionForeground(selectionForeground);
    }

    public void setSelectionInterval(int anchor, int lead) {
    	list.setSelectionInterval(anchor, lead);
    }

    public void setSelectionMode(int selectionMode) {
    	list.setSelectionMode(selectionMode);
    }

    public void setSelectionModel(ListSelectionModel selectionModel) {
    	list.setSelectionModel(selectionModel);
    }

    public void setValueIsAdjusting(boolean b) {
    	list.setValueIsAdjusting(b);
    }

    public void setVisibleRowCount(int visibleRowCount) {
    	list.setVisibleRowCount(visibleRowCount);
    }
}

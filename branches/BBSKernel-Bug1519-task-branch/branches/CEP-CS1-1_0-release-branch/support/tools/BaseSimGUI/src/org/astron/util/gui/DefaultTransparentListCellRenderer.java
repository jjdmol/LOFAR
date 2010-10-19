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
import java.awt.Component;
import javax.swing.*;

/**
 * This component contains extended functionality that DefaultListCellRenderer
 * doesn't have. The DefaultListCellRenderer doesn't work when using a background
 * image in a JList. The opaque (transparency) value is set to true (not
 * transparent) when the cell is selected. It's set to false (transparent) when
 * the is not selected.<BR><BR>
 *
 * <B>Notes :</B> Any suggestions on extra functionality are welcome!
 *
 * @author Jeroen Zwartepoorte (<A HREF="mailto:Jeroen@xs4all.nl">Jeroen@xs4all.nl</A>)
 * @version 1.0 (13-02-1999)
 */
public class DefaultTransparentListCellRenderer extends DefaultListCellRenderer
{
	/**
     * Constructor.
     */
	public DefaultTransparentListCellRenderer()
    {
    	super();
    }

    /**
     * Set the cells opaque value to true when selected, false when not selected.
     * Source used of DefaultListCellRenderer.java.
     */
    public Component getListCellRendererComponent(JList list, Object value,
    int index, boolean isSelected, boolean cellHasFocus)
    {
		if (isSelected)
        {
    		setOpaque(true);
		    setBackground(list.getSelectionBackground());
		    setForeground(list.getSelectionForeground());
		}
		else
        {
	    	setOpaque(false);
		    setBackground(list.getBackground());
	    	setForeground(list.getForeground());
		}

		if (value instanceof Icon)
	    	setIcon((Icon)value);
		else
	    	setText((value == null) ? "" : value.toString());

		setEnabled(list.isEnabled());
		setFont(list.getFont());
		setBorder((cellHasFocus) ? UIManager.getBorder("List.focusCellHighlightBorder") : noFocusBorder);

		return this;
    }
}

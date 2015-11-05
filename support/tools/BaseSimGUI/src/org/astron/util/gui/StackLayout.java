package org.astron.util.gui;

import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.LayoutManager2;

import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

public class StackLayout implements LayoutManager2
{
	private List _components;
	private List _constraints;

	private int					_vgap;
	private HorizontalBehaviour	_defaultBehaviour;

	public StackLayout()
	{
		this(5);
	}

	public StackLayout(int vgap)
	{
		this(vgap, LEFT);
	}

	public StackLayout(int vgap, HorizontalBehaviour defaultHorizontalBehaviour)
	{
		super();

		_vgap 				= vgap;
		_defaultBehaviour	= defaultHorizontalBehaviour;

		_components		= new ArrayList();
		_constraints	= new ArrayList();
	}

	public int getVerticalGap()
	{
		return _vgap;
	}

	public void setVerticalGap(int vgap)
	{
		_vgap = vgap;
	}

	public void removeLayoutComponent(Component component)
	{
		synchronized (component.getTreeLock())
		{
			int index = _components.indexOf(component);

			_components.remove(index);
			_constraints.remove(index);
		}
	}

	public void addLayoutComponent(Component component, Object constraint)
	{
		synchronized (component.getTreeLock())
		{
			HorizontalBehaviour behaviour;

			if(constraint == null)
			{
				behaviour = _defaultBehaviour;
			}
			else
			{
				behaviour = (HorizontalBehaviour) constraint;
			}

			_components.add(component);
			_constraints.add(behaviour);
		}
	}

	public void addLayoutComponent(String name, Component comp)
	{
		System.out.println("This method is not supported by the StackLayout");
	}

	public float getLayoutAlignmentX(Container target)
	{
		return 0.5f;
	}

	public float getLayoutAlignmentY(Container target)
	{
		return 0.5f;
	}

	public void invalidateLayout(Container target)
	{
		//no info is cached
	}

	// returns all preferred heigths of the components and the maximum preferred widths
	public Dimension preferredLayoutSize(Container container)
	{
		synchronized (container.getTreeLock())
		{
			Dimension result = new Dimension(0, 0);

			Iterator componentIterator = _components.iterator();
			while(componentIterator.hasNext())
			{
				Component component = (Component) componentIterator.next();

				if (component.isVisible())
				{
					Dimension dimension = component.getPreferredSize();

					result.height += dimension.height + _vgap;
					result.width = Math.max(result.width, dimension.width);
				}
			}

			Insets insets = container.getInsets();
			result.width	+= insets.left	+ insets.right;
			result.height	+= insets.top	+ insets.bottom;
			result.height	-= _vgap;
			return result;
		}
	}

	// returns all preferred heigths of the components and the maximum of the minimum widths
	public Dimension minimumLayoutSize(Container container)
	{
		synchronized (container.getTreeLock())
		{
			Dimension result = new Dimension(0, 0);

			Iterator componentIterator = _components.iterator();
			while(componentIterator.hasNext())
			{
				Component component = (Component) componentIterator.next();

				if (component.isVisible())
				{
					Dimension dimension				= component.getMinimumSize();
					Dimension preferredDimension	= component.getPreferredSize();

					result.height += preferredDimension.height + _vgap;
					result.width = Math.max(result.width, dimension.width);
				}
			}

			Insets insets = container.getInsets();
			result.width	+= insets.left	+ insets.right;
			result.height	+= insets.top	+ insets.bottom;
			result.height	-= _vgap;

			return result;
		}
	}

	// returns all preferred heigths of the components and the minimum of the maximum widths
	public Dimension maximumLayoutSize(Container container)
	{
		synchronized (container.getTreeLock())
		{
			Dimension result = new Dimension(Integer.MAX_VALUE, 0);

			Iterator componentIterator	= _components.iterator();
			Iterator constraintIterator	= _constraints.iterator();
			while(componentIterator.hasNext())
			{
				Component component = (Component) componentIterator.next();
				HorizontalBehaviour constraint = (HorizontalBehaviour) constraintIterator.next();

				if (component.isVisible())
				{
					Dimension dimension				= component.getMaximumSize();
					Dimension preferredDimension	= component.getPreferredSize();

					result.height += preferredDimension.height + _vgap;

					if(constraint.doesMaximumSizeMatter())
					{
						result.width = Math.min(result.width, dimension.width);
					}
				}
			}

			Insets insets = container.getInsets();
			result.width	+= insets.left	+ insets.right;
			result.height	+= insets.top	+ insets.bottom;
			result.height	-= _vgap;

			return result;
		}
	}

	public void layoutContainer(Container container)
	{
		synchronized (container.getTreeLock())
		{
			Dimension 	targetDimension	= container.getSize();
			Insets 		insets			= container.getInsets();

			int maximumWidth = targetDimension.width - (insets.left + insets.right);

			int x = insets.left;
			int y = insets.top;

			Iterator componentIterator	= _components.iterator();
			Iterator constraintIterator	= _constraints.iterator();

			while(componentIterator.hasNext())
			{
				Component component = (Component) componentIterator.next();
				HorizontalBehaviour constraint = (HorizontalBehaviour) constraintIterator.next();

				if(component.isVisible())
				{
					constraint.positionComponent(component, x, y, maximumWidth);
					y += component.getPreferredSize().getHeight() + _vgap;
				}
			}
		}
	}

	public static final HorizontalBehaviour STRETCH = new StretchHorizontalBehaviour();
	public static final HorizontalBehaviour LEFT	= new LeftPreferredSizeBehaviour();
	public static final HorizontalBehaviour RIGHT	= new RightPreferredSizeBehaviour();
	public static final HorizontalBehaviour CENTER	= new CenterPreferredSizeBehaviour();

	public interface HorizontalBehaviour
	{
		public void positionComponent(Component component, int x, int y, int maximumWidth);
		public boolean doesMaximumSizeMatter();
	}

	public static class StretchHorizontalBehaviour implements HorizontalBehaviour
	{
		public void positionComponent(Component component, int x, int y, int maximumWidth)
		{
			component.setSize(maximumWidth, component.getPreferredSize().height);
			component.setLocation(x, y);
		}

		public boolean doesMaximumSizeMatter()
		{
			return true;
		}
	}

	public static abstract class PreferredSizeHorizontalBehaviour implements HorizontalBehaviour
	{
		public int sizeComponent(Component component, int maximumWidth)
		{
			Dimension preferredSize = component.getPreferredSize();

			int width;
			int height = preferredSize.height;

			if(preferredSize.getWidth() > maximumWidth)
			{
				width = maximumWidth;
			}
			else
			{
				width = preferredSize.width;
			}

			component.setSize(width, height);

			return width;
		}

		public boolean doesMaximumSizeMatter()
		{
			return false;
		}
	}

	public static class LeftPreferredSizeBehaviour extends PreferredSizeHorizontalBehaviour
	{
		public void positionComponent(Component component, int x, int y, int maximumWidth)
		{
			int width = sizeComponent(component, maximumWidth);
			component.setLocation(x, y);
		}
	}

	public static class RightPreferredSizeBehaviour extends PreferredSizeHorizontalBehaviour
	{
		public void positionComponent(Component component, int x, int y, int maximumWidth)
		{
			int width = sizeComponent(component, maximumWidth);
			component.setLocation(x + (maximumWidth - width), y);
		}
	}

	public static class CenterPreferredSizeBehaviour extends PreferredSizeHorizontalBehaviour
	{
		public void positionComponent(Component component, int x, int y, int maximumWidth)
		{
			int width = sizeComponent(component, maximumWidth);
			component.setLocation(x + ((maximumWidth - width)/2), y);
		}
	}
}

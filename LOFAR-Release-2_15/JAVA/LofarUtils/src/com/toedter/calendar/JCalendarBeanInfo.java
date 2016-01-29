/*
 *  JcalendarBeanInfo.java - Bean Info for JCalendar Java Bean
 *  Copyright (C) 2004 Kai Toedter
 *  kai@toedter.com
 *  www.toedter.com
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
package com.toedter.calendar;

import com.toedter.components.GenericBeanInfo;


/**
 * A BeanInfo class for JCalendar.
 * 
 * @author Kai Toedter
 * @version $LastChangedRevision$
 * @version $LastChangedDate$
 */
public class JCalendarBeanInfo extends GenericBeanInfo {

	/**
	 * Constructs a new BeanInfo class for the JCalendar bean.
	 */
	public JCalendarBeanInfo() {
	    super("JCalendar", true);
	}
}
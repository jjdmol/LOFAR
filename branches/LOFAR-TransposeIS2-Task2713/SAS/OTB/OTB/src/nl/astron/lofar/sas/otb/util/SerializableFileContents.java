/*
 * SerializableFileContents.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otb.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.Serializable;

/**
 * This class provides a way to copy large files over RMI connections.
 * based upoin code found on the java developers network written by ejs.
 * @created 13-06-2006
 * @author ejs
 * @version $Id$
 * @updated
 */


public class SerializableFileContents implements Serializable
{
    public static final long serialVersionUID = -3306792878610064241L;
    private File file;
 
    public SerializableFileContents(File file)
    {
	this.file = file;
    }
 
    private void writeObject(java.io.ObjectOutputStream out)
     throws IOException
    {
	out.defaultWriteObject();
	FileInputStream in = new FileInputStream(file);
	byte[] buffer = new byte[8192];
	int count;
	while ((count = in.read(buffer)) > 0)
	    {
		out.writeInt(count);
		out.write(buffer,0,count);
	    }
	out.writeInt(count);
	in.close();
    }
 
    private void readObject(java.io.ObjectInputStream in)
	throws IOException, ClassNotFoundException
    {
	in.defaultReadObject();
	FileOutputStream out = new FileOutputStream(file);
	int count;
	while ((count = in.readInt()) > 0)
	    {
		byte[] buffer = new byte[count];
		in.readFully(buffer);
		out.write(buffer,0,count);
	    }
	out.close();
    }
}


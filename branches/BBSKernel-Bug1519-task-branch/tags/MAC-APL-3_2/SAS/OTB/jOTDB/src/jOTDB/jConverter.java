//#  jConverter.java: Logging.
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#

package jOTDB;

public class jConverter
{
   public jConverter ()
     {
	initConverter ();
     }
   
   private native void initConverter ();
   
   public native short getClassif (String aConv);
   public native String getClassif (short aConv);
   
   public native short getParamType (String aConv);
   public native String getParamType (short aConv);

   public native short getTreeState (String aConv);
   public native String getTreeState (short aConv);

   public native short getTreeType (String aConv);
   public native String getTreeType (short aConv);

   public native short getUnit (String aConv);
   public native String getUnit (short aConv);
}

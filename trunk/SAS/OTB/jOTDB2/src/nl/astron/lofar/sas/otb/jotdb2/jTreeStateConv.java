//#  jTreeStateConv.java : TreeState converter
//#
//#  Copyright (C) 2002-2007
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

package nl.astron.lofar.sas.otb.jotdb2;

import java.util.HashMap;

public class jTreeStateConv {

    public jTreeStateConv ()
    {
	initTreeStateConv();
    }

    private native void    initTreeStateConv();
    public  native long    get(String jarg1);
    public  native String  get(long jarg1);

    // java doesn't implement reference arguments, so the
    // initial call bool get(long type, string typename)
    // can't be made easily here/ Choice has been made to gather the complete
    // list in the c++ wrapper and return it completely here.
    public  native HashMap getTypes();
    public  native void    top();
    public  native boolean next();
} 

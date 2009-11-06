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
        try {
            initTreeStateConv();
        } catch (Exception ex) {
            System.out.println("Error during TreeStateConv init :" + ex);
        }
    }

    private native void    initTreeStateConv() throws Exception;
    public  native short   get(String jarg1) throws Exception;
    public  native String  get(short jarg1) throws Exception;

    // java doesn't implement reference arguments, so the
    // initial call bool get(long type, string typename)
    // can't be made easily here/ Choice has been made to gather the complete
    // list in the c++ wrapper and return it completely here.
    public  native HashMap<Short,String> getTypes() throws Exception;
    public  native void    top() throws Exception;
    public  native boolean next() throws Exception;
} 

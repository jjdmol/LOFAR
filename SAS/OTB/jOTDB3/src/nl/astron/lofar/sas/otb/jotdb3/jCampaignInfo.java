//#  CampaignInfo.h: Structure containing a campaign
//#
//#  Copyright (C) 2010
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


package nl.astron.lofar.sas.otb.jotdb3;
// A CampaignInfo struct describes one item/element of the OTDB. An item can
// be node or an parameter.
// Note: it does NOT contain the value of the item.

public class jCampaignInfo implements java.io.Serializable
{
    public jCampaignInfo ()
    {
	itsID = 0;
    }

    public jCampaignInfo (int id)
    {
	itsID = id;
    }



    public jCampaignInfo (String name, String title, String PI, String CO_I, String contact)
    {
	itsName = name;
	itsTitle  = title;
	itsPI = PI;
	itsCO_I = CO_I;
	itsContact = contact;
    }

   
    public int ID()
    {
	return (itsID); 
    }

   
    public String itsName;
    public String itsTitle;
    public String itsPI;
    public String itsCO_I;
    public String itsContact;
    private int itsID;

}

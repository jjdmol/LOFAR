//#  Campaign.java: Interface for access to the campaign record
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

package nl.astron.lofar.sas.otb.jotdb2;

import java.util.Vector;

public class jCampaign
{

    public jCampaign ()
    {
        try {
            initCampaign ();
        } catch (Exception ex) {
            System.out.println("Error during jCampaign init : " +ex);
        }
    }

    private native void initCampaign () throws Exception;

    // Get one campaign record.
    public native jCampaignInfo    getCampaign(String name) throws Exception;
    public native jCampaignInfo    getCampaign(int ID) throws Exception;

    // Get all campaign records
    public native Vector<jCampaignInfo> getCampaignList() throws Exception;

    // Update or insert a campaign record
    public native int saveCampaign(jCampaignInfo aCampaign) throws Exception;

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public native String errorMsg() throws Exception;


}

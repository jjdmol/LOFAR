/*
 *  IBBSStepOperationPanel.java
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
 *
 */

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations;

import java.util.HashMap;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepData;

/**
 * Interface that allows the BBS Step Explorer panel to utilize specific Step Operation Panels generically.
 *
 * @version $Id$
 * @created August 18, 2006, 12:16 PM
 * @author pompert
 */
public interface IBBSStepOperationPanel{
    /**
     * Sets the BBSStepData that contains the Step Operation attributes, including inherited data
     *
     * @parm stepData the BBSStepData that contains the Step Operation attributes
     * @parm inheritedData the inherited BBSStepData that contains Step Operation attributes should stepData not contain them.
     */
    public void setBBSStepContent(BBSStepData stepData, BBSStepData inheritedData);
    
    /**
     * Retrieves a HashMap of Step Operation attributes that are present in the input fields of a Step Operation Panel implementation.
     *
     * @return HashMap of Step Operation attributes entered in the Step Operation Panel implementation.
     */      
    public HashMap<String,String> getBBSStepOperationAttributes();
    
    /**
     * Tells the Step Operation Panel implementation to validate the input
     *
     * @return the outcome of the validation of the Step Operation Panel implementation
     */
    public boolean validateInput();
}

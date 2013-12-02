/*
 * ITreeManager.java
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

package nl.astron.lofar.sas.otb.util.treemanagers;

import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;

/**
 * Base TreeManager Interface
 *
 * @created 23-05-2006, 14:15
 *
 * @author pompert
 *
 * @version $Id$
 *
 * @updated
 */
public interface ITreeManager {
    
    public String getNameForNode(TreeNode aNode);
    
    public boolean isNodeLeaf(TreeNode aNode);
    
    public void defineChildsForNode(TreeNode aNode);
    
    public TreeNode getRootNode(Object arguments);
}

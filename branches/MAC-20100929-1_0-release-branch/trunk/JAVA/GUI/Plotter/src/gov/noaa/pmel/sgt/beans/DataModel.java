/*
 * $Id$
 *
 * This software is provided by NOAA for full, free and open release.  It is
 * understood by the recipient/user that NOAA assumes no liability for any
 * errors contained in the code.  Although this software is released without
 * conditions or restrictions in its use, it is expected that appropriate
 * credit be given to its author and to the National Oceanic and Atmospheric
 * Administration should the software be included by the recipient as an
 * element in other product development.
 */

package gov.noaa.pmel.sgt.beans;

import java.util.List;
import java.util.Vector;
import java.util.Iterator;

import java.beans.PropertyChangeSupport;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import gov.noaa.pmel.sgt.dm.SGTData;
import gov.noaa.pmel.sgt.Attribute;

/**
 * A model that supplies the relationship between <code>SGTData</code> objects,
 * its <code>Attribute</code> and the
 * <code>Panel</code> and <code>DataGroup</code> in which it is displayed and the
 * <code>Legend</code>.
 * <p> Some classes have been omitted for display purposes.
 * <P ALIGN="CENTER"><IMG SRC="images/DataModelSimple.png" ALIGN="BOTTOM" BORDER="0">
 *
 * @author Donald Denbo
 * @version $Revision$, $Date$
 * @since 3.0
 * @stereotype bean
 **/
public class DataModel {
  private PropertyChangeSupport support_ = new PropertyChangeSupport(this);
  private List dataList_;

  /**
   * @label page
   */
  private Page page;

  /** @link aggregation
   * @supplierCardinality 1..*
   * @label dataList*/
  /*#DataHolder lnkDataHolder;*/

  /**
   * Default constructor.
   */
  public DataModel() {
    dataList_ = new Vector();
  }

  /**
   * Add data to the <code>DataModel</code>.  Throws a "addData" property change.
   * @param data SGTData
   * @param attr Attribute for data
   * @param pHolder PanelHolder
   * @param dataGroup DataGroup
   * @param legend Legend
   */
  public void addData(SGTData data, Attribute attr,
                      PanelHolder pHolder, DataGroup dataGroup,
                      Legend legend) {
    DataHolder dh = new DataHolder(this, data, attr, pHolder,
                                   dataGroup, legend);
    dataList_.add(dh);
    support_.firePropertyChange("addData", null, dh);
  }

  /**
   * Get <code>Iterator</code> of the <code>DataHolder</code> objects.
   * @return
   */
  public Iterator dataIterator() {
    return dataList_.iterator();
  }

  /**
   * Add property change listener.
   * @param l property change listener
   */
  public void addPropertyChangeListener(PropertyChangeListener l) {
    support_.addPropertyChangeListener(l);
  }

  /**
   * Listen for specific property change.
   * @param name property name
   * @param l property change listner
   */
  public void addPropertyChangeListener(String name, PropertyChangeListener l) {
    support_.addPropertyChangeListener(name, l);
  }

  /**
   * Remove property change listener.
   * @param l property change listener
   */
  public void removePropertyChangeListener(PropertyChangeListener l) {
    support_.removePropertyChangeListener(l);
  }

  /**
   * Remove specific property change listener
   * @param name property name
   * @param l property change listener
   */
  public void removePropertyChangeListener(String name, PropertyChangeListener l) {
    support_.removePropertyChangeListener(name, l);
  }

  /**
   * Set <code>Page</code>.
   * @param page Page
   */
  public void setPage(Page page) {
    this.page = page;
  }

  /**
   * Get Page.
   * @return Page
   */
  public Page getPage() {
    return page;
  }
}

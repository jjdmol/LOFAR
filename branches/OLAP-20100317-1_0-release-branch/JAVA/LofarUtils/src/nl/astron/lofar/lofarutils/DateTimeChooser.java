/*
 * DateTimeChooser.java
 *
 * Created on 28 juni 2007, 10:58
 */

package nl.astron.lofar.lofarutils;

import com.toedter.components.JSpinField;
import java.awt.Component;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import javax.swing.JOptionPane;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.EventListenerList;

/**
 * A Swing component that lets choose date and time. It is just a thin
 * wrapper around an open-source package <em>jcalendar</em>, written
 * by <a href="http://www.toedter.com">Kai Toedter</a>, and released
 * under GNU Lesser General Public License. <p>
 *
 * Regarding functionality, the wrapper is similar to the standard
 * Java <tt>JColorChooser</tt>. Which means that a date-time chooser
 * can be either instantiate as a standalone component, or as part of
 * a modal dialog. In former case, the user can listen to the
 * date-time changes in the chooser by implementing a standard
 * <tt>ChangeListener</tt>, in the later case the returned value of
 * {@link #showDialog showDialog} contains the selected date-time. <p>
 *
 * It also allows to add a user-defined <em>preview panel</em> that
 * can indicate (in any user-defined format) directly in the chooser
 * window what date is selected. <p>
 *
 * @author <A HREF="mailto:martin.senger@gmail.com">Martin Senger</A>
 * @version $Id$
 * 
 * Minor changes to adepts to  LOFAR
 */
public class DateTimeChooser 
        extends javax.swing.JPanel {
    

    
    protected Date initialDate = null;
    protected transient ChangeEvent changeEvent = null;
    protected EventListenerList lList = new EventListenerList();
    protected Locale itsLocale;
    protected boolean isLinked=true;
    protected boolean initialised=false;
    
    /** Creates a DateTimeChooser with the current date as an initial date */
    public DateTimeChooser() {
        this(null);
        initComponents();
    }
    
    /** Creates a DateTimeChooser with the given date as an initial date */
    public DateTimeChooser(Date aDate) {
        this.initialDate = aDate;
        initComponents();
        createItself();
    }
    
    /** Creates this component */
    protected void createItself() {
        
        hours.loopEnabled(true);
        mins.loopEnabled(true);
        secs.loopEnabled(true);
        calendar.setWeekOfYearVisible(false);
           
        setDate (initialDate == null ? new Date() : initialDate,true);
        initialised=true;
    }
    
    /** gets the current date from the date-time chooser */
    public Date getDate() {
        Calendar cal = Calendar.getInstance();
        cal.setTime(calendar.getDate());
        cal.set(Calendar.HOUR_OF_DAY,hours.getValue());
        cal.set(Calendar.MINUTE,mins.getValue());
        cal.set(Calendar.SECOND,secs.getValue());

        
        return cal.getTime();
    }
    
        
     /** sets the given date as a new value for the date-time chooser <p>
     *
     *@param newDate to be set into the chooser
     */
    public void setDate(Date newDate,boolean isOld) {
        Calendar cal = Calendar.getInstance();
        cal.setTime(newDate);
        calendar.setDate(newDate);
        hours.setValue(cal.get(Calendar.HOUR_OF_DAY));
        mins.setValue(cal.get(Calendar.MINUTE));
        secs.setValue(cal.get(Calendar.SECOND));
    }
    

    /** Returns a date that was used to initiate this date-time chooser instance. 
     *  It can be null.
     */
    public Date getInitialDate() {
        return initialDate;
    }

    /*************************************************************************
     * Shows a modal date-time chooser dialog and blocks until the
     * dialog is hidden. The dialog has three buttons: OK, Empty,
     * Cancel. <p>
     *
     * If the user presses the "OK" button, then this method
     * hides/disposes the dialog and returns the selected date. <p>
     *
     * If the user presses the "Cancel" button or closes the dialog
     * without pressing "OK", then this method hides/disposes the
     * dialog and returns the initial date (which could have been
     * null). <p>
     * 
     * If the user presses the "Empty" button, then this method
     * hides/disposes the dialog and returns null. It indicates that
     * no date is selected (even though that might have been a date as
     * an initial value). <p>
     *
     * @param parent is the parent Component for the dialog
     * @param title contains the dialog's title
     * @param initialDate is shown when the dialog starts; if this is
     * null the current date is shown
     *
     * @return the selected date (if OK pressed), the initial date (if
     * Cancel presed), or null (if Empty pressed)
     *
     *************************************************************************/
    public static Date showDialog (Component parent,
				   String title,
				   Date initialDate) {
	return showDialog (parent, title, new DateTimeChooser (initialDate));
    }

    /*************************************************************************
     * Shows a modal date-time chooser dialog and blocks until the
     * dialog is hidden. The dialog has three buttons: OK, Empty,
     * Cancel. See details how the buttons are dealt with in {@link
     * #showDialog(Component,String,Date) showDialog}. <p>
     *
     * This method allows to create an instance of a date-time chooser
     * separately, and perhaps to customize it (e.g. by calling
     * <tt>chooser.setPreviewPanel (myPreviewPanel)</tt>) before it is
     * used in a modal dialog. <p>
     *
     * @param parent is the parent Component for the dialog
     * @param title contains the dialog's title
     * @param chooser is the chooser instance that was created
     * separately and will be used in this dialog
     *
     * @return the selected date (if OK pressed), the initial date (if
     * Cancel presed), or null (if Empty pressed)
     *
     *************************************************************************/
    public static Date showDialog (Component parent,
				   String title,
				   DateTimeChooser chooser) {
	String[] buttons = new String[] { "Cancel", "OK"};
	int selected =
	    JOptionPane.showOptionDialog (parent,
					  chooser,
					  title,
					  JOptionPane.YES_NO_OPTION,
					  JOptionPane.PLAIN_MESSAGE,
					  null,
					  buttons,
					  null);
	if (selected == 0)
	    return chooser.getInitialDate();  // cancelled
	else
            return chooser.getDate();         // 'ok' selected
	    
    }   
    
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        itsLocale= new Locale("en");
        calendar = new com.toedter.calendar.JCalendar(itsLocale);
        hours = new JSpinField(0,23);
        hours.adjustWidthToMaximumValue();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        mins = new JSpinField(0,59);
        mins.adjustWidthToMaximumValue();
        now = new javax.swing.JButton();
        zero = new javax.swing.JButton();
        inputLinked = new javax.swing.JCheckBox();
        jLabel3 = new javax.swing.JLabel();
        secs = new JSpinField(0,59);
        mins.adjustWidthToMaximumValue();

        jLabel1.setText("Hours:");

        jLabel2.setText("Minutes:");

        mins.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                minsPropertyChange(evt);
            }
        });

        now.setText("Set to now");
        now.setToolTipText("Set Date & Time to now");
        now.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                nowActionPerformed(evt);
            }
        });

        zero.setText("Clear time");
        zero.setToolTipText("Empty settings");
        zero.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                zeroActionPerformed(evt);
            }
        });

        inputLinked.setSelected(true);
        inputLinked.setText("Link Hours, Minutes & Seconds");
        inputLinked.setToolTipText("If checked the hours will change when minutes get over or under boundaries. And minutes will change when seconds get over or under bounderies");
        inputLinked.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputLinkedActionPerformed(evt);
            }
        });

        jLabel3.setText("Seconds:");

        secs.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                secsPropertyChange(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(calendar, javax.swing.GroupLayout.DEFAULT_SIZE, 250, Short.MAX_VALUE)
                        .addContainerGap())
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(inputLinked)
                        .addContainerGap(89, Short.MAX_VALUE))
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(now, javax.swing.GroupLayout.PREFERRED_SIZE, 102, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(zero)
                        .addContainerGap(71, Short.MAX_VALUE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGap(44, 44, 44))
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(hours, javax.swing.GroupLayout.PREFERRED_SIZE, 49, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(27, 27, 27)))
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel2)
                            .addComponent(mins, javax.swing.GroupLayout.PREFERRED_SIZE, 57, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(28, 28, 28)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel3)
                            .addComponent(secs, javax.swing.GroupLayout.PREFERRED_SIZE, 57, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(42, 42, 42))))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(calendar, javax.swing.GroupLayout.PREFERRED_SIZE, 145, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(inputLinked)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(jLabel1)
                            .addComponent(jLabel2))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(hours, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(mins, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(jLabel3)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(secs, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 21, Short.MAX_VALUE)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(now)
                    .addComponent(zero))
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void zeroActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_zeroActionPerformed
        hours.setValue(0);
        mins.setValue(0);
        secs.setValue(0);
    }//GEN-LAST:event_zeroActionPerformed

    private void nowActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_nowActionPerformed
        SimpleDateFormat aDate = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
        SimpleDateFormat aGMT = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        aGMT.setTimeZone(TimeZone.getTimeZone("GMT"));
        Date aD = new Date();
        String  aS = aGMT.format(aD);
        
        try {
            aD=aDate.parse(aS);
        } catch (ParseException ex) {
            ex.printStackTrace();
        }
        DateTimeChooser.this.setDate(aD,false);
    }//GEN-LAST:event_nowActionPerformed

    private void inputLinkedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputLinkedActionPerformed
        isLinked = inputLinked.isSelected();
    }//GEN-LAST:event_inputLinkedActionPerformed

    private void minsPropertyChange(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_minsPropertyChange
        if (initialised && isLinked && evt.getPropertyName().equals("value")) {

            if (((Integer)evt.getOldValue()).intValue() == 0 && ((Integer)evt.getNewValue()).intValue() == 59) {
                int hr = hours.getValue();
                hr -= 1;
                if (hr < 0) {
                    hr=23;
                }
                hours.setValue(hr);
            } else if(((Integer)evt.getOldValue()).intValue() == 59 && ((Integer)evt.getNewValue()).intValue() == 0 ) {
                int hr = hours.getValue();
                hr += 1;
                if (hr > 23) {
                    hr=0;
                }
                hours.setValue(hr);
            }
        }
    }//GEN-LAST:event_minsPropertyChange

    private void secsPropertyChange(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_secsPropertyChange
        if (initialised && isLinked && evt.getPropertyName().equals("value")) {

            if (((Integer)evt.getOldValue()).intValue() == 0 && ((Integer)evt.getNewValue()).intValue() == 59) {
                int min = mins.getValue();
                min -= 1;
                if (min < 0) {
                    min=59;
                }
                mins.setValue(min);
            } else if(((Integer)evt.getOldValue()).intValue() == 59 && ((Integer)evt.getNewValue()).intValue() == 0 ) {
                int min = mins.getValue();
                min += 1;
                if (min > 59) {
                    min=0;
                }
                mins.setValue(min);
            }
        }
}//GEN-LAST:event_secsPropertyChange
   

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private com.toedter.calendar.JCalendar calendar;
    private com.toedter.components.JSpinField hours;
    private javax.swing.JCheckBox inputLinked;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private com.toedter.components.JSpinField mins;
    private javax.swing.JButton now;
    private com.toedter.components.JSpinField secs;
    private javax.swing.JButton zero;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field holding the ChangeListener.
     */
    private transient javax.swing.event.ChangeListener changeListener =  null;

    /**
     * Registers ChangeListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addChangeListener(javax.swing.event.ChangeListener listener) throws java.util.TooManyListenersException {
        if (changeListener != null) {
            throw new java.util.TooManyListenersException ();
        }
        lList.add(ChangeListener.class,listener);
    }

    /**
     * Removes ChangeListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeChangeListener(javax.swing.event.ChangeListener listener) {
        lList.remove (ChangeListener.class, listener);
    }
    
    /*************************************************************************
     * Returns an array of all the <code>ChangeListener</code>s. <p>
     *
     * @return all of the <code>ChangeListener</code>s added, or an empty
     *         array if no listeners have been added
     *************************************************************************/
    public ChangeListener[] getChangeListeners() {
            return (ChangeListener[])lList
	    .getListeners (ChangeListener.class);
    }  
    
    /**
     * Notifies the registered listener about the event.
     * 
     * @param event The event to be fired
     */
    private void fireStateChanged() {
        Object[] listeners = lList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -=2 ) {
            if (listeners[i] == ChangeListener.class) {
                if (changeEvent == null) {
                    changeEvent = new ChangeEvent (this);
                }
                ((ChangeListener)listeners [i+1]).stateChanged(changeEvent);
            }
        }
    }


}



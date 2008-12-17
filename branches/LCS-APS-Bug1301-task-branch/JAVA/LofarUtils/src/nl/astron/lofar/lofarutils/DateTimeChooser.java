/*
 * DateTimeChooser.java
 *
 * Created on 28 juni 2007, 10:58
 */

package nl.astron.lofar.lofarutils;

import com.toedter.components.JSpinField;
import java.awt.Component;
import java.beans.PropertyChangeEvent;
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
        extends javax.swing.JPanel 
        implements java.beans.PropertyChangeListener {
    

    
    protected Date initialDate = null;
    protected transient ChangeEvent changeEvent = null;
    protected EventListenerList lList = new EventListenerList();
    protected Locale itsLocale;
    
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
        hours.addPropertyChangeListener(this);
        mins.addPropertyChangeListener(this);
        
        calendar.setWeekOfYearVisible(false);
        calendar.addPropertyChangeListener(this);
        calendar.getYearChooser().addPropertyChangeListener(this);
           
        setDate (initialDate == null ? new Date() : initialDate,true);
    }
    
    /** gets the current date from the date-time chooser */
    public Date getDate() {
        Calendar cal = Calendar.getInstance();
        cal.setTime(calendar.getDate());
        cal.set(Calendar.HOUR_OF_DAY,hours.getValue());
        cal.set(Calendar.MINUTE,mins.getValue());

        
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

        jLabel1.setText("Hours:");

        jLabel2.setText("Minutes:");

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

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGap(89, 89, 89))
                            .addComponent(now, javax.swing.GroupLayout.PREFERRED_SIZE, 102, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(layout.createSequentialGroup()
                                .addContainerGap()
                                .addComponent(hours, javax.swing.GroupLayout.PREFERRED_SIZE, 88, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addGroup(layout.createSequentialGroup()
                                .addGap(10, 10, 10)
                                .addComponent(jLabel2)
                                .addGap(38, 38, 38))
                            .addGroup(javax.swing.GroupLayout.Alignment.LEADING, layout.createSequentialGroup()
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(zero)
                                    .addComponent(mins, javax.swing.GroupLayout.PREFERRED_SIZE, 84, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                    .addComponent(calendar, javax.swing.GroupLayout.DEFAULT_SIZE, 210, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(calendar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(jLabel1))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(mins, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(hours, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(20, 20, 20)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(zero)
                    .addComponent(now))
                .addGap(24, 24, 24))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void zeroActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_zeroActionPerformed
        hours.setValue(0);
        mins.setValue(0); 
    }//GEN-LAST:event_zeroActionPerformed

    private void nowActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_nowActionPerformed
        SimpleDateFormat aDate = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        SimpleDateFormat aGMT = new SimpleDateFormat("yyyy-MMM-d HH:mm");
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
   

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private com.toedter.calendar.JCalendar calendar;
    private com.toedter.components.JSpinField hours;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private com.toedter.components.JSpinField mins;
    private javax.swing.JButton now;
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

    
    /*************************************************************************
     * Implementing PropertyChangeListener. This implementation only
     * propagates all property change events as a ChangeEvent.
     *************************************************************************/
    public void propertyChange (PropertyChangeEvent evt) {
	fireStateChanged();
    }

}

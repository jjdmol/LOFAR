/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * inputFieldBuilder.java
 *
 * Created on 25-aug-2010, 10:28:42
 */

package nl.astron.lofar.lofarutils.inputfieldbuilder;

import java.awt.CardLayout;
import java.awt.Color;
import javax.swing.JFrame;
import javax.swing.JPanel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.lofarutils.validation.AbstractValidator;
import nl.astron.lofar.lofarutils.validation.BoolValidator;
import nl.astron.lofar.lofarutils.validation.BoolVectorValidator;
import nl.astron.lofar.lofarutils.validation.DateValidator;
import nl.astron.lofar.lofarutils.validation.DateVectorValidator;
import nl.astron.lofar.lofarutils.validation.DoubleValidator;
import nl.astron.lofar.lofarutils.validation.DoubleVectorValidator;
import nl.astron.lofar.lofarutils.validation.FloatValidator;
import nl.astron.lofar.lofarutils.validation.FloatVectorValidator;
import nl.astron.lofar.lofarutils.validation.IntVectorValidator;
import nl.astron.lofar.lofarutils.validation.IntegerValidator;
import nl.astron.lofar.lofarutils.validation.LongValidator;
import nl.astron.lofar.lofarutils.validation.LongVectorValidator;
import nl.astron.lofar.lofarutils.validation.NodeValidator;
import nl.astron.lofar.lofarutils.validation.TextValidator;
import nl.astron.lofar.lofarutils.validation.TextVectorValidator;
import nl.astron.lofar.lofarutils.validation.TimeValidator;
import nl.astron.lofar.lofarutils.validation.TimeVectorValidator;


/**
 *
 * @author coolen
 */
public class inputFieldBuilder extends javax.swing.JPanel {

    public static inputFieldBuilder currentInputField = null;


    public inputFieldBuilder() {
        itsPanel=null;
        itsFrame=null;
        itsName="";
        itsType="";
        itsUnit="";
        itsFormat="";
        itsNodeValue="";
        itsParamValue="";
        initComponents();
    }

    /** Creates new form inputFieldBuilder */
    public void setContent(JFrame aFrame,JPanel aPanel,String aName,String type,String unit, String nodevalue, String format, String paramvalue) {
        itsPanel=aPanel;
        itsFrame=aFrame;
        itsName=aName;
        itsType=type;
        itsUnit=unit;
        itsNodeValue=nodevalue;
        itsFormat=format;
        itsParamValue=paramvalue;
        fillFields();
    }

    private void fillFields() {
        // select on type
        // types starts with p means we need a combobox
        CardLayout cl=(CardLayout)getLayout();
        if (itsType.startsWith("p")) {
            cl.show(this, "ComboCard");
            isCombo=true;
            LofarUtils.setPopupComboChoices(comboInput, itsParamValue);
            comboInput.setSelectedItem(itsNodeValue);
            return;
        }

        // else a textfield is involved
        // later check for existing format, but first take the simple formats
        cl.show(this, "TextCard");
        isCombo=false;

        setValidators();

        textInput.setText(itsNodeValue);

    }

    private void setValidators() {

        if (textInput.isVisible() && textInput.isEditable()) {
            // int
            if (itsType.equals("int")) {              // signed integer
                textInput.setInputVerifier(new IntegerValidator(itsFrame, textInput, true));
            } else if (itsType.equals("uint")) {      // unsigned integer
                textInput.setInputVerifier(new IntegerValidator(itsFrame, textInput, false));
            } else if (itsType.equals("long")) {      // signed integer
                textInput.setInputVerifier(new LongValidator(itsFrame, textInput, true));
            } else if (itsType.equals("ulng")) {      // unsigned integer
                textInput.setInputVerifier(new LongValidator(itsFrame, textInput, false));
            } else if (itsType.equals("dbl")) {       // double
                textInput.setInputVerifier(new DoubleValidator(itsFrame, textInput));
            } else if (itsType.equals("flt")) {       // float
                textInput.setInputVerifier(new FloatValidator(itsFrame, textInput));
            } else if (itsType.equals("date")) {      // date
                textInput.setInputVerifier(new DateValidator(itsFrame, textInput));
            } else if (itsType.equals("time")) {      // time
                textInput.setInputVerifier(new TimeValidator(itsFrame, textInput));
            } else if (itsType.equals("bool")) {      // bool
                textInput.setInputVerifier(new BoolValidator(itsFrame, textInput,""));
            } else if (itsType.equals("text")) {      // text
                textInput.setInputVerifier(new TextValidator(itsFrame, textInput));
            } else if (itsType.equals("node")) {      // text
                textInput.setInputVerifier(new NodeValidator(itsFrame, textInput));

                // Vectors
            } else if (itsType.equals("vtext")) {     // Vector of Textfields
                textInput.setInputVerifier(new TextVectorValidator(itsFrame, textInput));
            } else if (itsType.equals("vbool")) {      // Vector of booleans
                textInput.setInputVerifier(new BoolVectorValidator(itsFrame, textInput));
            } else if (itsType.equals("vdbl")) {      // Vector of doubles
                textInput.setInputVerifier(new DoubleVectorValidator(itsFrame, textInput));
            } else if (itsType.equals("vflt")) {      // Vector of floats
                textInput.setInputVerifier(new FloatVectorValidator(itsFrame, textInput));
            } else if (itsType.equals("vint")) {       // Vector of signed integers
                textInput.setInputVerifier(new IntVectorValidator(itsFrame, textInput, true));
            } else if (itsType.equals("vuint")) {      // Vector of unsigned integers
                textInput.setInputVerifier(new IntVectorValidator(itsFrame, textInput, false));
            } else if (itsType.equals("vlong")) {      // Vector of signed longs
                textInput.setInputVerifier(new LongVectorValidator(itsFrame, textInput, true));
            } else if (itsType.equals("vulng")) {      // Vector of unsigned longs
                textInput.setInputVerifier(new LongVectorValidator(itsFrame, textInput, false));
            } else if (itsType.equals("vdate")) {      // Vector of dates
                textInput.setInputVerifier(new DateVectorValidator(itsFrame, textInput));
            } else if (itsType.equals("vulng")) {      // Vector of Times
                textInput.setInputVerifier(new TimeVectorValidator(itsFrame, textInput));
            }
            currentInputField=this;
            textInput.setName(itsName);
            setCaller(itsPanel);
        } else {
            textInput.setInputVerifier(null);
            currentInputField=null;
            textInput.setName("");
        }
    }

    public String getValue() {
        if (isCombo) {
            return comboInput.getSelectedItem().toString();
        } else {
            return textInput.getText();
        }
    }

    public void setButtonsEnabled(boolean flag) {
        comboInput.setEnabled(flag);
        textInput.setEnabled(flag);
        setValidators();
    }

    public void setButtonsEditable(boolean flag) {
        comboInput.setEditable(flag);
        textInput.setEditable(flag);
        setValidators();
    }

    public boolean checkInput() {
        boolean b = true;
        if (!isCombo) {
            b=textInput.getInputVerifier().shouldYieldFocus(textInput);
            if (!b) {
                textInput.requestFocusInWindow();
            }
        }
        return b;
    }

    public void checkPopup() {
        if (!isCombo) {
//           System.out.println("InputFieldBuilder checkPopup");
            ((AbstractValidator)textInput.getInputVerifier()).checkPopup();
            textInput.setBackground(Color.WHITE);
        }
    }

    public void setCaller(Object aCaller) {
        if (!isCombo) {
            ((AbstractValidator)textInput.getInputVerifier()).setCaller(aCaller);
        }
    }

    private String itsName="";
    private String itsUnit="";
    private String itsNodeValue="";
    private String itsType="";
    private String itsFormat="";
    private String itsParamValue="";
    private JPanel itsPanel=null;
    private JFrame itsFrame=null;
    private boolean isCombo=false;

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        textInput = new javax.swing.JTextField();
        comboInput = new javax.swing.JComboBox();

        setPreferredSize(new java.awt.Dimension(625, 25));
        setLayout(new java.awt.CardLayout());

        textInput.setText("None");
        textInput.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                textInputFocusGained(evt);
            }
            public void focusLost(java.awt.event.FocusEvent evt) {
                textInputFocusLost(evt);
            }
        });
        add(textInput, "TextCard");

        comboInput.setEditable(true);
        comboInput.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        comboInput.setVerifyInputWhenFocusTarget(false);
        add(comboInput, "ComboCard");
    }// </editor-fold>//GEN-END:initComponents

    private void textInputFocusLost(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_textInputFocusLost
        if (!isCombo) {
//            System.out.println("InputFieldBuilder Focus Lost");
            if (((AbstractValidator)textInput.getInputVerifier()).isVisible()) {
//                System.out.println("InputFieldBuilder Focus Lost: popup was visible, Focus back to text" );
               textInput.requestFocusInWindow();
            } else {
//                System.out.println("InputFieldBuilder Focus Lost: popup was not visible." );
                return;
            }
        }
    }//GEN-LAST:event_textInputFocusLost

    private void textInputFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_textInputFocusGained
//        System.out.println("InputFieldBuilder Focus Gained:" );
        if (!((AbstractValidator)textInput.getInputVerifier()).isVisible()) {
//            System.out.println("InputFieldBuilder Gained: popup was NOT visible, reset CurrentInputField" );
            currentInputField=this;
        } else {
//            System.out.println("InputFieldBuilder Gained: popup was visible, keep CurrentInputField" );
        }
    }//GEN-LAST:event_textInputFocusGained


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JComboBox comboInput;
    private javax.swing.JTextField textInput;
    // End of variables declaration//GEN-END:variables

}
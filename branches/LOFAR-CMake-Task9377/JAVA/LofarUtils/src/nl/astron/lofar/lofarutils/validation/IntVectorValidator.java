package nl.astron.lofar.lofarutils.validation;

import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JTextField;
import nl.astron.lofar.lofarutils.LofarUtils;

/**
 * A class for performing pre-validation on Integers
 *
 * @author Arthur Coolen
 */

public class IntVectorValidator extends AbstractValidator {

    boolean isSigned=false;
    public IntVectorValidator(JDialog parent, JTextField c, boolean signed) {
        super(parent, c, "");
        isSigned=signed;
    }

    public IntVectorValidator(JFrame parent, JTextField c, boolean signed) {
        super(parent, c, "");
        isSigned=signed;
    }

    protected boolean validationCriteria(JComponent c) {
        String input = ((JTextField)c).getText();

        //int vectors can contain x..y notation, so expand the string first
        String in = LofarUtils.expandedArrayString(input);
        String msg = Validators.validateIntVector(in,isSigned);
        if (msg.isEmpty() ) {
            return true;
        } else {
            setMessage(msg);
            return false;
        }


    }

}


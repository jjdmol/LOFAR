package nl.astron.lofar.lofarutils.validation;

import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JTextField;

/**
 * A class for performing pre-validation on Integers
 *
 * @author Arthur Coolen
 */

public class LongVectorValidator extends AbstractValidator {

    boolean isSigned=false;
    public LongVectorValidator(JDialog parent, JTextField c, boolean signed) {
        super(parent, c, "");
        isSigned=signed;
    }

    public LongVectorValidator(JFrame parent, JTextField c, boolean signed) {
        super(parent, c, "");
        isSigned=signed;
    }

    protected boolean validationCriteria(JComponent c) {
        String input = ((JTextField)c).getText();

        String msg = Validators.validateLongVector(input,isSigned);
        if (msg.isEmpty() ) {
            return true;
        } else {
            setMessage(msg);
            return false;
        }
    }

}


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

public class BoolValidator extends AbstractValidator {


    public BoolValidator(JDialog parent, JTextField c, String message) {
        super(parent, c, message);
    }

    public BoolValidator(JFrame parent, JTextField c, String message) {
        super(parent, c, message);
    }

    @Override
    protected boolean validationCriteria(JComponent c) {
        String input = ((JTextField)c).getText();


        String msg = Validators.validateBoolean(input);
        if (msg.isEmpty() ) {
            return true;
        } else {
            setMessage(msg);
            return false;
        }
    }

}


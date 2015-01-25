
/*
 That's all there is too it. To use the validator on a text field in your application, all you have to to is something like this:

JTextField textField = new JTextField();
textField.setInputVerifier(new NotEmptyValidator(parent, textField, "Field cannot be null."));

 */



package nl.astron.lofar.lofarutils.validation;

import javax.swing.JComponent;
import javax.swing.JTextField;
import javax.swing.JDialog;
import javax.swing.JFrame;

/**
 * A class for performing basic validation on text fields. All it does is make
 * sure that they are not null.
 *
 * @author Michael Urban
 */

public class NotEmptyValidator extends AbstractValidator {
    public NotEmptyValidator(JDialog parent, JTextField c, String message) {
        super(parent, c, message);
    }

    public NotEmptyValidator(JFrame parent, JTextField c, String message) {
        super(parent, c, message);
    }

    protected boolean validationCriteria(JComponent c) {
        if (((JTextField)c).getText().equals(""))
            return false;
        return true;
    }
}


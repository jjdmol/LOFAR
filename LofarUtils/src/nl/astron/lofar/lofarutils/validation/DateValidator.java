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

public class DateValidator extends AbstractValidator {

       public DateValidator(JDialog parent, JTextField c) {
        super(parent, c, "");
    }

   public DateValidator(JFrame parent, JTextField c) {
        super(parent, c, "");
    }

       protected boolean validationCriteria(JComponent c) {
        String input = ((JTextField)c).getText();


        String msg = Validators.validateDate(input);
        if (msg.isEmpty() ) {
            return true;
        } else {
            setMessage(msg);
            return false;
        }

    }

}


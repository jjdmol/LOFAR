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

public class RegexValidator extends AbstractValidator {

    String itsRegex="";
    public RegexValidator(JDialog parent, JTextField c, String regex) {
        super(parent, c, "");
        itsRegex=regex;
    }

    public RegexValidator(JFrame parent, JTextField c, String regex) {
        super(parent, c, "");
        itsRegex=regex;
    }
    protected boolean validationCriteria(JComponent c) {
        String input = ((JTextField)c).getText();

        String msg = Validators.validateWithRegex(input,itsRegex);
        if (msg.isEmpty() ) {
            return true;
        } else {
            setMessage(msg);
            return false;
        }


    }

}


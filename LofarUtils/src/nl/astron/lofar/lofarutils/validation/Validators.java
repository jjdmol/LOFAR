/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package nl.astron.lofar.lofarutils.validation;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author coolen
 */
public class Validators {
    private static final String EMPTYMESSAGE         = "Field cannnot be blank.";

    // All types of fields can alternatively contain deref strings. Those strings start with >>
    private static final String RDREF                = "^>>[a-zA-Z0-9\\.]+";

    // double quoted string surrounded by possible whitespace
    private static final String DQSTR                = "\\s*\"[^\"]*\"\\s*";

    // single quoted string surrounded by possible whitespace
    private static final String SQSTR                = "\\s*'[^']*'\\s*";

    // unquoted string surrounded by possible whitespace  ' " not allowed
    private static final String USTR                = "\\s*[^'\"]*\\s*";

    // Yearnotation YYYY (for now only 1900-2099)
    private static final String YEAR                 = "(19|20)\\d\\d";

    // Monthnotation MM (01-12)
    private static final String MONTH                = "(0[1-9]|1[012])";

    // Daynotation DD (01-31)
    private static final String DAY                  = "(0[1-9]|[12][0-9]|3[01])";

    // short time notations
    private static final String STIME                = "([0-9]+[HhMmSs]?)";

    // Hour notation (00-23)
    private static final String HOUR                 = "(0[0-9]|1[0-9]|2[0-3])";

    // minute notation (00-59)
    private static final String MINUTE               = "([0-5][0-9])";

    // second notation (00-59)
    private static final String SECOND               = "([0-5][0-9])";

    // Double/Float notation
    private static final String DF                   = "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";

    // Int/Long notation
    private static final String IL                   = "((([-+][0-9])?[0-9]*))";

    // Unsigned Int/Long notation
    private static final String UIL                  = "([0-9]*)";

    // Unsigned Int/Long notation Vectors allow 1..9 kind notations
    private static final String VUIL                 = "(([0-9]+\\.\\.[0-9]+)|([0-9]*))";

    private static final String NODE                 = "<<[a-zA-Z][0-9a-zA-Z]*";

    // Boolean notation
    private static final String BOOL                 = "(\\s*(TRUE|FALSE)|(1|0)|(YES|NO)|(T|F)|(Y|N)\\s*)";

    private static final String NODEREGEX            = "^"+NODE+"$";
    private static final String NODEMESSAGE          = "Node references start with << and must refer to an existing node";
    
    private static final String TEXTVECTORREGEX      = "("+RDREF+")|(^\\[(("+DQSTR+")(,"+DQSTR+")*|("+SQSTR+")(,"+SQSTR+")*|("+USTR+")(,"+USTR+")*)\\]$)";
    private static final String TEXTVECTORMESSAGE    = "Vectors are enclosed in [] and all values seperated by , either \"\" or '' or not quoted strings";

    private static final String TEXTREGEX            = "("+RDREF+")|(^(("+DQSTR+")|("+SQSTR+")|("+USTR+"))$)";
    private static final String TEXTMESSAGE          = "Text format can be enclosed in \" or \' and when not enclosed it can't contain ' or \" ";

    private static final String BOOLVECTORREGEX      = "("+RDREF+")|(^\\[\\s*("+BOOL+")\\s*(,\\s*("+BOOL+"))*)?\\s*\\]$)";
    private static final String BOOLVECTORMESSAGE    = "Boolean Vector format is (ignorecase) TRUE/FALSE, T/F, 1/0, YES/NO or Y/N values seperated by ,";

    private static final String BOOLEANREGEX         = "("+RDREF+")|(^"+BOOL+"$)";
    private static final String BOOLEANMESSAGE       = "Boolean format is (ignorecase) TRUE/FALSE, T/F, 1/0, YES/NO or Y/N";

    private static final String DOUBLEVECTORREGEX    = "("+RDREF+")|(^\\[\\s*(("+DF+")\\s*(,\\s*("+DF+"))*)?\\s*\\]$)";
    private static final String DOUBLEVECTORMESSAGE  = "Double Vector can only contain comma seperated Double values [+-][0-9].[eE][+-][0-9] between brackets ";

    private static final String DOUBLEREGEX          = "("+RDREF+")|(^"+DF+"$)";
    private static final String DOUBLEMESSAGE        = "Double can only contain numbers, e, E, \".\", \"+\" and \"-\" ";

    private static final String FLOATVECTORREGEX    = "("+RDREF+")|(^\\[\\s*(("+DF+")\\s*(,\\s*("+DF+"))*)?\\s*\\]$)";
    private static final String FLOATVECTORMESSAGE  = "Float Vector can only contain comma seperated Float values [+-][0-9].[eE][+-][0-9] between brackets";

    private static final String FLOATREGEX           = "("+RDREF+")|(^"+DF+"$)";
    private static final String FLOATMESSAGE         = "Float can only contain numbers, e, E, \".\", \"+\" and \"-\" ";

    private static final String INTVECTORREGEX       = "("+RDREF+")|(^\\[\\s*(("+IL+")\\s*(,\\s*("+IL+"))*)?\\s*\\]$)";
    private static final String INTVECTORMESSAGE     = "Signed integer Vector can only contain comma seperated integers [+-][0-9] between brackets";

    private static final String INTREGEX             = "("+RDREF+")|(^"+IL+"$)";
    private static final String INTMESSAGE           = "Signed integers can only contain numbers and \"+-\"";

    private static final String UINTVECTORREGEX      = "("+RDREF+")|(^\\[\\s*(("+VUIL+")\\s*(,\\s*("+VUIL+"))*)?\\s*\\]$)";
    private static final String UINTVECTORMESSAGE    = "unsigned integer Vector can only contain comma seperated unsigned integers [0-9] between brackets";

    private static final String UINTREGEX            = "("+RDREF+")|(^"+UIL+"$)";
    private static final String UINTMESSAGE          = "Unsigned integers can only contain numbers";

    private static final String LONGVECTORREGEX      = "("+RDREF+")|(^\\[\\s*(("+IL+")\\s*(,\\s*("+IL+"))*)?\\s*\\]$)";
    private static final String LONGVECTORMESSAGE    = "Signed long Vector can only contain comma seperated longs [+-][0-9] between brackets";

    private static final String LONGREGEX            = "("+RDREF+")|(^"+IL+"$)";
    private static final String LONGMESSAGE          = "Signed longs can only contain numbers and \"+-\"";

    private static final String ULONGVECTORREGEX     = "|("+RDREF+")|(^\\[\\s*(("+VUIL+")\\s*(,\\s*("+VUIL+"))*)?\\s*\\]$)";
    private static final String ULONGVECTORMESSAGE   = "unsigned long Vector can only contain comma seperated unsigned longs [0-9] between brackets";

    private static final String ULONGREGEX           = "("+RDREF+")|(^"+UIL+"$)";
    private static final String ULONGMESSAGE         = "Unsigned longs can only contain numbers";


    private static final String DATEVECTORREGEX      = "("+RDREF+")|(^\\[\\s*(("+YEAR+"-"+MONTH+"-"+DAY+")\\s*(,\\s*("+YEAR+"-"+MONTH+"-"+DAY+"))*)?\\s*\\]$)";
    private static final String DATEVECTORMESSAGE    = "Vector Dateformat is comma seperated values of yyyy-mm-dd between brackets";

    private static final String DATEREGEX            = "("+RDREF+")|(^"+YEAR+"-"+MONTH+"-"+DAY+"$)";
    private static final String DATEMESSAGE          = "Dateformat is yyyy-mm-dd";
    private static final SimpleDateFormat DATEFORMAT = new SimpleDateFormat("yyyy-MM-dd");


    private static final String TIMEVECTORREGEX      = "("+RDREF+")|(^\\[\\s*(("+STIME+")\\s*(,\\s*("+STIME+"))*)?\\s*\\]$)|(^\\[\\s*(("+HOUR+":"+MINUTE+":"+SECOND+")\\s*(,\\s*("+HOUR+":"+MINUTE+":"+SECOND+"))*)?\\s*\\]$)";
    private static final String TIMEVECTORMESSAGE    = "Vector Timeformat is comma seperated values of HH:mm:ss between brackets";

    private static final String TIMEREGEX            = "("+RDREF+")|(^"+STIME+"$)|(^"+HOUR+":"+MINUTE+":"+SECOND+"$)";
    private static final String TIMEMESSAGE          = "Timeformat is HH:mm:ss";
    private static final SimpleDateFormat TIMEFORMAT = new SimpleDateFormat("HH:mm:ss");


    private static final String REGEXMESSAGE         = "Your regex  and input did not match";

    
    static public String validateNode(String input) {

        if (!validateAgainstRegex(NODEREGEX,input)) {
            return NODEMESSAGE;
        }

        return "";
    }

    static public String validateText(String input) {

        if (!validateAgainstRegex(TEXTREGEX,input)) {
            return TEXTMESSAGE;
        }

        return "";
    }

    static public String validateTextVector(String input) {

        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        if (!validateAgainstRegex(TEXTVECTORREGEX,input)) {
            return TEXTVECTORMESSAGE;
        }

        return "";
    }

    static public String validateBoolean(String input) {

        if (input.equals("")) {
            return "";
        }


        if (!validateAgainstRegex(BOOLEANREGEX,input.toUpperCase())) {
            return BOOLEANMESSAGE;
        }

        return "";
    }

    static public String validateBoolVector(String input) {

        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        if (!validateAgainstRegex(BOOLVECTORREGEX,input.toUpperCase())) {
            return BOOLVECTORMESSAGE;
        }

        return "";
    }

    static public String validateDouble(String input) {

        if (input.equals("")) {
            return "";
        }


        if (!validateAgainstRegex(DOUBLEREGEX,input)) {
            return DOUBLEMESSAGE;
        }

        if (input.startsWith(">>")) return "";

        try {
            double i = Double.parseDouble(input);
        } catch (NumberFormatException ex) {
            return "Wrong double format";
        }

        return "";
    }

    static public String validateDoubleVector(String input) {
        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        if (!validateAgainstRegex(DOUBLEVECTORREGEX,input)) {
            return DOUBLEVECTORMESSAGE;
        }

        String aS=input.replaceAll("[\\[\\]\\s]", "");
        String[] doubles = aS.split(",");

        try {
            for (int i=0;i<doubles.length;i++) {
                if (!doubles[i].isEmpty()) {
                    double d = Double.parseDouble(doubles[i]);
                }
            }
        } catch (NumberFormatException ex) {
            return "Wrong double format";
        }

        return "";
    }

    static public String validateFloat(String input) {

        if (input.equals("")) {
            return "";
        }

        if (!validateAgainstRegex(FLOATREGEX,input)) {
            return FLOATMESSAGE;
        }

        if (input.startsWith(">>")) return "";

        try {
            float i = Float.parseFloat(input);
        } catch (NumberFormatException ex) {
            return "Wrong float format";
        }
        return "";
    }

    static public String validateFloatVector(String input) {
        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        if (!validateAgainstRegex(FLOATVECTORREGEX,input)) {
            return FLOATVECTORMESSAGE;
        }

        String aS=input.replaceAll("[\\[\\]\\s]", "");
        String[] floats = aS.split(",");

        try {
            for (int i=0;i<floats.length;i++) {
                if (!floats[i].isEmpty()) {
                    float d = Float.parseFloat(floats[i]);
                }
            }
        } catch (NumberFormatException ex) {
            return "Wrong float format";
        }

        return "";
    }

    static public String validateInteger(String input,boolean signed) {

        if (input.equals("")) {
            return "";
        }

        String reg;
        if (signed) {
            reg=INTREGEX;
        } else {
            reg=UINTREGEX;
        }

        if (!validateAgainstRegex(reg,input)) {
            if (signed) {
                return INTMESSAGE;
            } else {
                return UINTMESSAGE;
            }
        }

        if (input.startsWith(">>")) return "";
        try {

            int i = Integer.parseInt(input);
        } catch (NumberFormatException ex) {
            return "Wrong (un)signed integer format";
        }
        return "";
    }

    static public String validateIntVector(String input,boolean signed) {

        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        String reg;
        if (signed) {
            reg=INTVECTORREGEX;
        } else {
            reg=UINTVECTORREGEX;
        }

        if (!validateAgainstRegex(reg,input)) {
            if (signed) {
                return INTVECTORMESSAGE;
            } else {
                return UINTVECTORMESSAGE;
            }
        }

        String aS=input.replaceAll("[\\[\\]\\s]", "");
        String[] ints = aS.split(",");

        try {
            for (int i=0;i<ints.length;i++) {
                if (!ints[i].isEmpty()) {
                    int ii = Integer.parseInt(ints[i]);
                }
            }
        } catch (NumberFormatException ex) {
            return "Wrong (un)signed integer format";
        }
        return "";
    }

    static public String validateLong(String input,boolean signed) {
        if (input.equals("")) {
            return "";
        }

        String reg;
        if (signed) {
            reg=LONGREGEX;
        } else {
            reg=ULONGREGEX;
        }

        if (!validateAgainstRegex(reg,input)) {
            if (signed) {
                return LONGMESSAGE;
            } else {
                return ULONGMESSAGE;
            }
        }

        if (input.startsWith(">>")) return "";

        try {
            long i = Long.parseLong(input);
        } catch (NumberFormatException ex) {
            return "Wrong (un)signed long format";
        }
        return "";
    }

    static public String validateLongVector(String input,boolean signed) {

        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        String reg;
        if (signed) {
            reg=LONGVECTORREGEX;
        } else {
            reg=ULONGVECTORREGEX;
        }

        if (!validateAgainstRegex(reg,input)) {
            if (signed) {
                return LONGVECTORMESSAGE;
            } else {
                return ULONGVECTORMESSAGE;
            }
        }

        String aS=input.replaceAll("[\\[\\]\\s]", "");
        String[] longs = aS.split(",");

        try {
            for (int i=0;i<longs.length;i++) {
                if (!longs[i].isEmpty()) {
                    long l = Long.parseLong(longs[i]);
                }
            }
        } catch (NumberFormatException ex) {
            return "Wrong (un)signed long format";
        }
        return "";
    }

    static public String validateDate(String input) {
        if (input.equals("")) {
            return "";
        }

        if (!validateAgainstRegex(DATEREGEX,input)) {
            return DATEMESSAGE;
        }

        if (input.startsWith(">>")) return "";

        try {
            Date i = DATEFORMAT.parse(input);
        } catch (ParseException ex) {
            return "Wrong date format";
        }
        return "";
    }

    static public String validateDateVector(String input) {
        if (input.equals("")) {
            return EMPTYMESSAGE;
        }

        if (!validateAgainstRegex(DATEVECTORREGEX,input)) {
            return DATEVECTORMESSAGE;
        }

        String aS=input.replaceAll("[\\[\\]\\s]", "");
        String[] dates = aS.split(",");
        try {
            for (int i=0;i<dates.length;i++) {
                if (!dates[i].isEmpty()) {
                    Date d = DATEFORMAT.parse(dates[i]);
                }
            }
        } catch (ParseException ex) {
            return "Wrong date format";
        }
        return "";
    }

    static public String validateTime(String input) {
        if (input.equals("")) {
            return "";
        }


        if (!validateAgainstRegex(TIMEREGEX,input)) {
            return TIMEMESSAGE;
        }

        if (input.startsWith(">>")||!input.contains(":")) return "";

        try {
            Date i = TIMEFORMAT.parse(input);
        } catch (ParseException ex) {
            return "Wrong time format";
        }
        
        return "";
    }

    static public String validateTimeVector(String input) {
        if (input.equals("")) {
            return EMPTYMESSAGE;
        }


        if (!validateAgainstRegex(TIMEVECTORREGEX,input)) {
            return TIMEVECTORMESSAGE;
        }
        if (!input.contains(":")) return "";

        String aS=input.replaceAll("\\[\\]\\s", "");
        String[] times = aS.split(",");
        try {
            for (int i=0;i<times.length;i++) {
                if (!times[i].isEmpty()) {
                    Date d = TIMEFORMAT.parse(times[i]);
                }
            }
        } catch (ParseException ex) {
            return "Wrong time format";
        }

        return "";
    }

    static public String validateWithRegex(String input,String regex) {
        if (input.equals("")) {
            return "";
        }


        if (!validateAgainstRegex(regex,input)) {
            return REGEXMESSAGE;
        }

        return "";
    }

    static public boolean validateAgainstRegex(String regex,String input) {
        Matcher m = Pattern.compile(regex).matcher(input);
        return m.matches();
    }
}

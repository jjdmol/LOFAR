package MessageBus;

import java.util.HashMap;
import java.util.Map;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.TextMessage;
import javax.jms.BytesMessage;
import javax.jms.Session;

public class LOFARMessage {

    public Message msg;
    public Map<String, String> Header;
    public String Payload;

    private String PayloadAsString() throws JMSException {
        if (msg instanceof TextMessage) {
            return ((TextMessage) msg).getText();
        }
        if (msg instanceof BytesMessage) {
            byte[] buf = new byte[2048];
            int len = 0;
            StringBuffer strBuf = new StringBuffer();
            while ((len = ((BytesMessage) msg).readBytes(buf)) > 0) {
                strBuf.append(new String(buf, 0, len));
            }
            return strBuf.toString();
        }
        return new String("");
    }

    private Map<String, String> HeaderFromContent() throws JMSException {
        String content = PayloadAsString();
        Map<String, String> map = new HashMap<String, String>() ;

        // retrieve fields here.. (the XML like structure is a bit cumbersome to retrieve)

        return map;
    }

    public LOFARMessage(Message message) {
        msg = message;
        try {
            @SuppressWarnings("unchecked")
            // retrieve header
            Map<String, String> objectProperty = (Map<String, String>) message.getObjectProperty("LOFARHeader");
            Header = objectProperty;
            
            // did the message have 'old' LOFAR message format? 
            if (Header == null) 
            {
                Header = HeaderFromContent();
                Payload = Header.get("Payload");
            } else // set payload from content.
            {
                Payload = PayloadAsString();
            }
        } catch (JMSException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        System.out.print(Payload);
    }
}

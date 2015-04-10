package MessageBus;

/*
 *    Wrapper to implement a bridge between JMS onMessage and LOFAR processmessage
 *    
 *    instantiate with a LOFARMessageListener Object so that the processmessage() method
 *    is called when a message has been fetched.
 * 
 */

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageListener;

public class LofarListenProxy implements MessageListener {

    private LOFARMessageListener listener;

    @Override
    public void onMessage(Message message) {
        int ret = 0;
        LOFARMessage msg = new LOFARMessage(message);
        ret = listener.processmessage(msg);
        // ret==0 means success.
        // Handling more reasoncodes can be done, we could even wrap the above in a try/catch.
        if (ret == 0) {
            try {
                message.acknowledge();
            } catch (JMSException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        } else {
            // we need to decide what to do if something goes wrong..
        }
    }

    public LofarListenProxy(LOFARMessageListener listener) {
        this.listener = listener;
    }
}
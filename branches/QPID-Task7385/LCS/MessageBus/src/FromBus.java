package MessageBus;

/*
 *     Class FromBus allows receiving of LOFAR messages from the messagebus.
 *     It provides a means to add a LOFARMessageListener to each queue. 
 *     For each incoming message LOFARMessageListener.processmessage() is called with the
 *     incoming LOFARMessage as an argument.
 *     Depending on the returnvalue an acknowledge will be done on the message.
 *     returnvalue == 0 means "Message is processed correctly, this message may be acknowledged."
 *  
 *     In future this may be extended to more methods with for example LOFARMessage receive(float timeout);
 *     
 */
import java.util.HashMap;
import java.util.Properties;
import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MessageConsumer;
import javax.jms.Session;
import javax.naming.Context;
import javax.naming.InitialContext;

public class FromBus {

    private Properties properties;
    private ConnectionFactory connectionFactory;
    private HashMap<String, MessageConsumer> Consumers;
    private Context context;
    private Connection connection;
    private Session session;
    public boolean SuccessConnect = false;

    public FromBus(String PropFile) {
        try {
            Consumers = new HashMap<String, MessageConsumer>();
            properties = new Properties();

            // Setup from properties file
            properties.load(this.getClass().getResourceAsStream(PropFile));
            context = new InitialContext(properties);

            // create connection to messagebroker
            connectionFactory = (ConnectionFactory) context.lookup("qpidConnectionfactory");
            connection = connectionFactory.createConnection();
            connection.start();

            // start session on connection with client acknowledge
            session = connection.createSession(false, Session.CLIENT_ACKNOWLEDGE);

            SuccessConnect = true;
        } catch (Exception e) {
            System.out.println("FromBus setup failed for properties file " + PropFile);
            e.printStackTrace();  //TODO.
        }
    }

    public boolean AddListener(String Key, LOFARMessageListener MyLOFARListener) {
        boolean ret = false;
        if (SuccessConnect) {
            try {
                // setup endpoint (queues)
                LofarListenProxy MyListener = new LofarListenProxy(MyLOFARListener);
                Destination destination = (Destination) context.lookup(Key);

                // create message queue handlers 
                Consumers.put(Key, this.session.createConsumer(destination));

                // attach listener to incoming queue
                if (MyListener != null) {
                    Consumers.get(Key).setMessageListener(MyListener);
                }
                
                ret = true;
            } catch (Exception e) {
                System.out.println("AddListener for queue " + Key + " Failed.");
                e.printStackTrace();  //TODO.
            }
        }
        return ret;
    }

    public void StopListening() {
        if (SuccessConnect) {
            for (MessageConsumer messageConsumer : this.Consumers.values()) {
                try {
                    messageConsumer.close();
                } catch (Exception e) {
                    System.out.println("StopListening failed.");
                    e.printStackTrace();  //TODO.
                }
            }
            try {
                session.close();
                connection.close();
            } catch (JMSException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            SuccessConnect = false;
        }
    }

    protected void finalize() {
        this.StopListening();
    }
}
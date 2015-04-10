package MessageBus;

/*
 *     Class ToBus allows sending of LOFAR messages to the messagebus.
 *     It provides a means to create LOFAR messages too. 
 *     Apparently only Session objects know how to create a new TextMessage.
 */
import java.util.Map;
import java.util.Properties;
import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MessageProducer;
import javax.jms.Session;
import javax.jms.TextMessage;
import javax.naming.Context;
import javax.naming.InitialContext;

public class ToBus {

    private Properties properties = new Properties();
    private ConnectionFactory connectionFactory;
    private Destination destination;
    private MessageProducer producer;
    private Context context;
    private Connection connection;
    private Session session;
    public boolean SuccessConnect = false;

    public ToBus(String PropFile, String Key) {
        try {
            // Setup from properties file
            this.properties.load(this.getClass().getResourceAsStream(PropFile));
            this.context = new InitialContext(properties);
            
            // create connection to messagebroker
            this.connectionFactory = (ConnectionFactory) this.context.lookup("qpidConnectionfactory");
            this.connection = this.connectionFactory.createConnection();
            this.connection.start();

            // start session on connection with client acknowledge
            this.session = this.connection.createSession(false, Session.CLIENT_ACKNOWLEDGE);

            // setup endpoint (queues)
            this.destination = (Destination) this.context.lookup(Key);

            // setup Producer
            this.producer = this.session.createProducer(destination);

            this.SuccessConnect = true;
        } catch (Exception e) {
            System.out.println("ToBus setup failed for properties file " + PropFile + " with key " + Key);
            e.printStackTrace();  //TODO.
        }
    }

    public boolean SendMsg(LOFARMessage message) {
        boolean ret = false;
        try {
            this.producer.send(message.msg);
            ret = true;
        } catch (Exception e) {
            System.out.println("Send Message for queue " + this.destination + " failed, message not sent. ");
            e.printStackTrace();  //TODO.
        }
        return ret;
    }

    public LOFARMessage CreateMsg(Map<String, String> header, String payload) throws JMSException {
        TextMessage tmesg = null;
        tmesg = this.session.createTextMessage(payload);
        tmesg.setObjectProperty("LOFARHeader", header);
        return new LOFARMessage(tmesg);
    }
}

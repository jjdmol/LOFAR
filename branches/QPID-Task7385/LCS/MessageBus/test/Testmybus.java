/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package testmybus;

import MessageBus.FromBus;
import MessageBus.LOFARMessage;
import MessageBus.LOFARMessageListener;

/**
 *
 * @author janrinze
 */
public class Testmybus {

    public class DEMO_listener_ONE implements LOFARMessageListener {

        @Override
        public int processmessage(LOFARMessage msg) {

            // do all activities for messages of this specific type. 
            System.out.println(msg.Payload);
            return 0;
        }
        DEMO_listener_ONE()
        {
            
        }
    }

    public class DEMO_listener_TWO implements LOFARMessageListener {

        @Override
        public int processmessage(LOFARMessage msg) {
            System.out.println(msg.Payload);
            // do all activities for messages of this specific type. 
            return 0;
        }
    }
    private String propfile = "/testmybus/demo.properties";
    private String KeyOne = "inbus_one";
    private String KeyTwo = "inbus_two";
    private FromBus frombus;
    public boolean Connected;

    Testmybus() {
        frombus = new FromBus(propfile);
        frombus.AddListener(KeyOne, new DEMO_listener_ONE());
        frombus.AddListener(KeyTwo, new DEMO_listener_TWO());
        Connected = frombus.SuccessConnect;
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here

        Testmybus test = new Testmybus();
        int bla = 0;
        while (test.Connected) {
            bla += 1;
        }


    }
}

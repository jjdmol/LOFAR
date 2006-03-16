package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * This class is the main entry point for this project.
 * @author  balken
 */
public class Main
{
    /**
     * Main method.
     * @param   args
     */
    public static void main(String[] args)
    {
        Board b = new Board();
        BoardStatus bs = b.getStatus();
        
        System.out.println("volt 1V2: " + bs.voltage1V2);
        System.out.println("volt 2V5: " + bs.voltage2V5);
        System.out.println("volt 3V3: " + bs.voltage3V3);
        System.out.println("pcb temp: " + bs.pcbTemp);
        System.out.println("bp temp: " + bs.bpTemp);
        System.out.println("ap0 temp: " + bs.ap0Temp);
        System.out.println("ap1 temp: " + bs.ap1Temp);
        System.out.println("ap2 temp: " + bs.ap2Temp);
        System.out.println("ap3 temp: " + bs.ap3Temp);      
        System.out.println("bp clock: " + bs.bpClock);
        System.out.println("nof frames: " + bs.nofFrames);
        System.out.println("nof errors: " + bs.nofErrors);
        System.out.println("last error: " + bs.lastError);
        System.out.println("seq nr: " + bs.seqNr);
        System.out.println("error: " + bs.error);
        System.out.println("interface: " + bs.ifUnderTest);
        System.out.println("mode: " + bs.mode);
        System.out.println("ri errors: " + bs.riErrors);
        System.out.println("rcux errors: " + bs.rcuxErrors);
        System.out.println("lcu errors: " + bs.lcuErrors);
        System.out.println("cep errors: " + bs.cepErrors);
        System.out.println("serdes errors: " + bs.serdesErrors);
        System.out.println("ap0ri errors: " + bs.ap0RiErrors);
        System.out.println("ap1ri errors: " + bs.ap1RiErrors);
        System.out.println("ap2ri errors: " + bs.ap2RiErrors);
        System.out.println("ap3ri errors: " + bs.ap3RiErrors);
        System.out.println("blp0 sync: " + bs.blp0Sync);
        System.out.println("blp1 sync: " + bs.blp1Sync);
        System.out.println("blp2 sync: " + bs.blp2Sync);
        System.out.println("blp3 sync: " + bs.blp3Sync);
        System.out.println("blp0 rcu: " + bs.blp0Rcu);
        System.out.println("blp1 rcu: " + bs.blp1Rcu);
        System.out.println("blp2 rcu: " + bs.blp2Rcu);
        System.out.println("blp3 rcu: " + bs.blp3Rcu);
        System.out.println("cp status: " + bs.cpStatus);
        System.out.println("blp0 adc offset: " + bs.blp0AdcOffset);
        System.out.println("blp1 adc offset: " + bs.blp1AdcOffset);
        System.out.println("blp2 adc offset: " + bs.blp2AdcOffset);
        System.out.println("blp3 adc offset: " + bs.blp3AdcOffset);      
    }
}
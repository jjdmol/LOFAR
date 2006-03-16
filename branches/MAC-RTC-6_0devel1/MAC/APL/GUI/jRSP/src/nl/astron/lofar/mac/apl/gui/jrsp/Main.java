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
        
        System.out.println("volt 1V2: " + bs.getVoltage1V2());
        System.out.println("volt 2V5: " + bs.getVoltage2V5());
        System.out.println("volt 3V3: " + bs.getVoltage3V3());
        System.out.println("pcb temp: " + bs.getPcbTemp());
        System.out.println("bp temp: " + bs.getBpTemp());
        System.out.println("ap0 temp: " + bs.getAp0Temp());
        System.out.println("ap1 temp: " + bs.getAp1Temp());
        System.out.println("ap2 temp: " + bs.getAp2Temp());
        System.out.println("ap3 temp: " + bs.getAp3Temp());      
        System.out.println("bp clock: " + bs.getBpClock());
        System.out.println("nof frames: " + bs.getNofFrames());
        System.out.println("nof errors: " + bs.getNofErrors());
        System.out.println("last error: " + bs.getLastError());
        System.out.println("seq nr: " + bs.getSeqnr());
        System.out.println("error: " + bs.getError());
        System.out.println("interface: " + bs.getInterface());
        System.out.println("mode: " + bs.getMode());
        System.out.println("ri errors: " + bs.getRiErrors());
        System.out.println("rcux errors: " + bs.getRcuxErrors());
        System.out.println("lcu errors: " + bs.getLcuErrors());
        System.out.println("cep errors: " + bs.getCepErrors());
        System.out.println("serdes errors: " + bs.getSerdesErrors());
        System.out.println("ap0ri errors: " + bs.getAp0RiErrors());
        System.out.println("ap1ri errors: " + bs.getAp1RiErrors());
        System.out.println("ap2ri errors: " + bs.getAp2RiErrors());
        System.out.println("ap3ri errors: " + bs.getAp3RiErrors());
        System.out.println("blp0 sync: " + bs.getBlp0Sync());
        System.out.println("blp1 sync: " + bs.getBlp1Sync());
        System.out.println("blp2 sync: " + bs.getBlp2Sync());
        System.out.println("blp3 sync: " + bs.getBlp3Sync());
        System.out.println("blp0 rcu: " + bs.getBlp0Rcu());
        System.out.println("blp1 rcu: " + bs.getBlp1Rcu());
        System.out.println("blp2 rcu: " + bs.getBlp2Rcu());
        System.out.println("blp3 rcu: " + bs.getBlp3Rcu());
        System.out.println("cp status: " + bs.getCpStatus());
        System.out.println("blp0 adc offset: " + bs.getBlp0AdcOffset());
        System.out.println("blp1 adc offset: " + bs.getBlp1AdcOffset());
        System.out.println("blp2 adc offset: " + bs.getBlp2AdcOffset());
        System.out.println("blp3 adc offset: " + bs.getBlp3AdcOffset());      
    }
}
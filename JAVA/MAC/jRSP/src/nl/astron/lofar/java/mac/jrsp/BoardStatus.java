/*
 * BoardStatus.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.mac.jrsp;

/**
 * This class keeps hold of the Board Status information. All member variables are public so there is no
 * need for getters and setters.
 * A part of the statusdata is stored in special classes. Theses classes have a
 * strong resemblance to this class; their member variables are also public.
 * These classes are instantiated in de constructor of this class, so the user
 * has easy access and will not encounter any nullpointer exceptions.
 * @see nl.astron.lofar.mac.apl.gui.jrsp.ADOStatus
 * @see nl.astron.lofar.mac.apl.gui.jrsp.RCUStatus
 * @see nl.astron.lofar.mac.apl.gui.jrsp.SyncStatus
 *
 * @author  balken
 */
public class BoardStatus
{
    /** Measured 1.2 V supply voltage. */
    public double voltage1V2;
    
    /** Measured 2.5 V supply voltage. */
    public double voltage2V5;
    
    /** Measured 3.3 V supply voltage. */
    public double voltage3V3;
    
    /** Current RSP board temperature. */
    public short pcbTemp;
    
    /** Current Board Processor temperature. */
    public short bpTemp;
    
    /** Current Antenna Processor 0 temperature. */
    public short ap0Temp;
    
    /** Current Antenna Processor 1 temperature. */
    public short ap1Temp;
    
    /** Current Antenna Processor 2 temperature. */
    public short ap2Temp;
    
    /** Current Antenna Processor 3 temperature. */
    public short ap3Temp;
    
    /** Current Board Processor system clock speed. */
    public short bpClock;
    
    /** Number of eth frames received. */
    public int nofFrames;
    
    /** Number of incorrect ethernet frames. */
    public int nofErrors;
    
    /** Error status of last received ethernet frame. */
    public short lastError;
    
    /** Sequence number of previous received message. */
    public int seqNr;
    
    /** Error status of previous received message. */
    public short error;
    
    /** Interface under test. */
    public short ifUnderTest;
    
    /** Test mode. */
    public short mode;
    
    /** Number of detected errors. */
    public int riErrors;
    
    /** Number of detected errors. */
    public int rcuxErrors;
    
    /** Number of detected errors. */
    public int rcuyErrors;
    
    /** Number of detected errors. */
    public int lcuErrors;
    
    /** Number of detected errors. */
    public int cepErrors;
    
    /** Number of detected errors. */
    public int serdesErrors;
    
    /** Number of detected errors. */
    public int ap0RiErrors;
    
    /** Number of detected errors. */
    public int ap1RiErrors;
    
    /** Number of detected errors. */
    public int ap2RiErrors;
    
    /** Number of detected errors. */
    public int ap3RiErrors;
    
    /** Number of detected errors. */
    public SyncStatus blp0Sync;
    
    /** Number of detected errors. */
    public SyncStatus blp1Sync;
    
    /** Number of detected errors. */
    public SyncStatus blp2Sync;
    
    /** Number of detected errors. */
    public SyncStatus blp3Sync;
    
    /** BLP0 RCU status. */
    public RCUStatus blp0Rcu;
    
    /** BLP1 RCU status. */
    public RCUStatus blp1Rcu;
    
    /** BLP2 RCU status. */
    public RCUStatus blp2Rcu;
    
    /** BLP3 RCU status. */
    public RCUStatus blp3Rcu;
    
    /** Status information from CP. */
    /** RDY - Type of image loaded in FPGA. */
    public boolean cpRdy;
    
    /** ERR - Configuration result. */
    public boolean cpErr;
    
    /** APnBP - Type of FPGA that was configured previously. */
    public boolean cpFpga;
    
    /** IM - Type of image loaded in FPGA. */
    public boolean cpIm;
    
    /** TRIG - Cause of previous reconfiguration. */
    public short cpTrig;
    
    /** BLP0 ADC offset values. */
    public ADOStatus blp0AdcOffset;
    
    /** BLP1 ADC offset values. */
    public ADOStatus blp1AdcOffset;
    
    /** BLP2 ADC offset values. */
    public ADOStatus blp2AdcOffset;
    
    /** BLP3 ADC offset values. */
    public ADOStatus blp3AdcOffset;    
    
    /**
     * Default Constructor.
     */
    public BoardStatus()
    {
        blp0Sync = new SyncStatus();
        blp1Sync = new SyncStatus();
        blp2Sync = new SyncStatus();
        blp3Sync = new SyncStatus();
        blp0Rcu = new RCUStatus();
        blp1Rcu = new RCUStatus();
        blp2Rcu = new RCUStatus();
        blp3Rcu = new RCUStatus();
        blp0AdcOffset = new ADOStatus();
        blp1AdcOffset = new ADOStatus();
        blp2AdcOffset = new ADOStatus();
        blp3AdcOffset = new ADOStatus();
    }
}
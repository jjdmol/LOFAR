/*
 * Board.java
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

import org.apache.log4j.Logger;

/**
 * The Board class is used to connect to the C++ side of the whole jRSP project.
 *
 * This class should be used as follows:
 * First you can initialize the Board, at this point you *don't* have a connection
 * on the C++ side with the boards.
 * Then you can call the connect method with the hostname that should be used.
 * At the end you call the disconnect method to end/delete our connection in the
 * native code.
 * For example, if you want to get the status:
 * 
 * String sHostName = %hostname%;
 * int iRcuMask = %rcumask%;
 *
 * Board b = new Board();                         // initialize board
 * b.connect(sHostName);                          // connect to board @ hostname
 * BoardStatus[] arrBs = b.getStatus(iRcuMask);   // get the boardStatus
 * b.disconnect();                                // disconnect the board
 * 
 *
 * @author  balken
 */
public class Board
{
    /** Hostname */
    private String hostname;
    
    /** Array of BoardStatus */
    private BoardStatus[] boardStatus;
    
    /** Pointer to the RSPport class in C++ */
    private int ptrRSPport;
    
    /** Logger. */
    private Logger itsLogger;
    
    /**
     * Makes a new instance of the Board class.
     * @param   hostname    The hostname of the RSPBoard to connect to.
     */   
    public Board()
    {
        itsLogger = Logger.getLogger(this.getClass());
        //itsLogger.debug("Constructor");
        
        hostname = null;
        ptrRSPport = 0;
        boardStatus = null;
    }
    
    
    /** CONNECT FUNCTIONS **/
    
    /**
     * Makes a connection to the RSPdriver via RSPport. Before using this
     * function. There should be a check to see if the board is already
     * connected. And if so it should be disconnected with the disconnect-
     * method.
     *
     * @param   hostname    hostname of the board.
     * @throws  Exception   thrown when, probably, the hostname was incorrect.
     */
    public void connect(String hostname) throws Exception
    {        
        //itsLogger.debug("connect");
                
        this.hostname = hostname;
        
        /*
         * Try to initialize the RSPport. When the hostname is invalid a
         * exception will be thrown from the JNI code and that exception is
         * getting thrown.
         */        
        ptrRSPport = init(hostname);        
    }
    
    /**
     * Deletes the reference to the RSPport we have in C++.
     */
    public void disconnect()
    {
        //itsLogger.debug("disconnect");
        
        delete(ptrRSPport);
        
        hostname = null;
        ptrRSPport = 0;
    }
    
    /**
     * Returns if there is a connection with the RSPport.
     * @return              True if connected, False if not.
     */
    public boolean isConnected()
    {
        //itsLogger.debug("isConnected");
        
        return (ptrRSPport > 0);
    }
    
    /** END OF CONNECT FUNCTIONS **/
    
    /** NATIVE FUNCTIONS **/
    private native int init(String hostname) throws Exception; // initializes RSPport on the C++ side, for further use.
    private native void delete(int ptrRSPport); // deletes the RSPport instance on the c++ side.
    private native BoardStatus[] retrieveStatus(int rspMask, int ptrRSPport); // retrieves the boards.
    private native boolean setWaveformSettings(int rcuMask, int mode, double frequency, short phase, int amplitude, int ptrRSPport); // Sets the waveform settings.
    private native double[] getSubbandStats(int rcuMask, int ptrRSPport); // retrieves subbandstats
    private native WGRegisterType[] getWaveformSettings(int rcuMask, int ptrRSPport); // returnsd wfsettings
    private native int getNrRCUs(int ptrRSPport);
    private native int getNrRSPBoards(int ptrRSPport);
    private native int getMaxRSPBoards(int ptrRSPport);
    private native boolean setFilter(int rcuMask, int filterNr, int ptrRSPport);
    private native boolean sendClear(int rspMask, int ptrRSPport);
    private native boolean sendReset(int rspMask, int ptrRSPport);
    private native boolean sendSync(int rspMask, int ptrRSPport);
    private native double[] getBeamletStats(int rspMask, int ptrRSPport);
          
    static
    {
        System.loadLibrary("jrsp");
    }
    
    /** END OF NATIVE FUNCTIONS **/
    
    
    /**
     * @return  hostname
     */
    public String getHostname()
    {
        //itsLogger.debug("getHostname");
        
        return hostname;
    }
        
    /**
     * Returns the status of the Boards.
     * @return  BoardStatus[]   A array of BoardStatus.
     */
    public BoardStatus[] getStatus(RSPMask rspMask)
    {
        //itsLogger.debug("getStatus");
        
        boardStatus = retrieveStatus(rspMask.intValue(), ptrRSPport);
        return boardStatus;
    }
        
    /**
     * Sets the waveform settings.
     * @param   rcuMask
     * @param   mode
     * @param   frequency
     * @param   amplitude
     */
    public boolean setWaveformSettings(RCUMask rcuMask, int mode, double frequency, short phase,  int amplitude)
    {   
        // adjust phase from [0-360] to [0-255]
        int adjPhase = (phase * 255) / 360;
        
        // adjust frequency by multiplying with 10e6
        frequency *= 1e6;

        boolean ret = setWaveformSettings(rcuMask.intValue(), mode, frequency, (short)adjPhase, amplitude, ptrRSPport);
 
        return ret;
    }
    
    /**
     * Returns the subband stats.
     * @param   rcuMask
     * @return              array of doubles.
     */
    public double[] getSubbandStats(RCUMask rcuMask)
    {
        //itsLogger.debug("getSubbandStats");
        
        return getSubbandStats(rcuMask.intValue(), ptrRSPport);
    }
    
    /**
     * Returns the waveform settings.     
     */
    public WGRegisterType[] getWaveformSettings(RCUMask rcuMask)
    {
        //itsLogger.debug("getWaveformSettings");
        
        return getWaveformSettings(rcuMask.intValue(), ptrRSPport);
    }
 
    /**
     * Sets the filter.
     * @param   rcuMask     The mask that is being used.
     */
    public boolean setFilter(RCUMask rcuMask, int filterNr)
    {
        //itsLogger.debug("setFilter - mask: " + rcuMask.intValue() + ", filterNr: " + filterNr);
        
        return setFilter(rcuMask.intValue(), filterNr, ptrRSPport);
    }
    
    /**
     * Sends the clear command to the board specified in the mask.
     * @param   rcuMask     The mask that is being used.
     */
    public boolean sendClear(RSPMask rspMask)
    {
        //itsLogger.debug("sendClear - rcuMask: " + rcuMask.intValue());
        
        return sendClear(rspMask.intValue(), ptrRSPport);
    }
    
    /**
     * Sends the reset command to the board specified in the mask.
     * @param   rcuMask     The mask that is being used.
     */
    public boolean sendReset(RSPMask rspMask)
    {
        //itsLogger.debug("sendReset");
        
        return sendReset(rspMask.intValue(), ptrRSPport);
    }
    
    /**
     * Sends the sync command to the board specified in the mask.
     * @param   rcuMask     The mask that is being used.
     */
    public boolean sendSync(RSPMask rspMask)
    {
        //itsLogger.debug("sendSync");
        
        return sendSync(rspMask.intValue(), ptrRSPport);
    }    
       
    /**
     * Returns number of RCU's connected.
     */
    public int getNrRCUs()
    {
        //itsLogger.debug("getNrRCUs");
        
        return getNrRCUs(ptrRSPport);
    }
    
    /**
     * Returns the number of boards connected.
     */
    public int getNrRSPBoards()
    {
        //itsLogger.debug("getNrRSPBoards");
        
        return getNrRSPBoards(ptrRSPport);
    }
    
    /**
     * Returns the maximum number of boards that could be connected.
     */
    public int getMaxRSPBoards()
    {
        //itsLogger.debug("getMaxRSPBoards");
        
        return getMaxRSPBoards(ptrRSPport);
    }
    
    /**
     * Returns the beamlet stats based on the mask that is passed.
     * @param   rspMask
     * @return  A double array filled with beamlet stats.
     */
    public double[] getBeamletStats(RSPMask mask)
    {
        return getBeamletStats(mask.intValue(), ptrRSPport);
    }
}

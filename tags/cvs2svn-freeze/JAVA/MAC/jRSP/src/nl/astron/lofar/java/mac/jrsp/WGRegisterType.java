/*
 * WGRegisterType.java
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
 * Waveform Generator Register Type
 * Designed to hold WGSettings::WGRegisterType structs.
 *
 * @author balken
 */
public class WGRegisterType 
{
    public short mode;
    
    /** Used to access when the 0-255 value is required. JNI. */
    public short phase;
    
    /**
     * Returns phase as a number in the range 0-360.
     */
    public short getPhase()
    {
        int temp = (phase * 360) / 255;
        return (short)temp;
    }
    
    /**
     * Sets phase as a number in the range 0-255 from a 0-360 number.
     */
    public void setPhase(short phase)
    {
        int temp = (phase * 255) / 360;
        
        this.phase = (short)temp;
    }    
    
    public int nofSamples;
    
    /**
     * Frequency is stored as a large number.
     */
    public double frequency;
 
    /**
     * Returns frequency 1e6 times smaller.
     */
    public double getFrequency()
    {
        return frequency / 1e6;
    }
    
    
    public long amplitude;
 
    /** These two variables are added to register which antenna board it is. */
    public int board;
    public int antenna;
    
    public WGRegisterType()
    {
        board = -1;
        antenna = -1;
    }
    
    public WGRegisterType(short mode, short phase, int nofSamplse, double frequency, long amplitude)
    {
        this.mode = mode;
        this.phase = phase;
        this.nofSamples = nofSamples;
        this.frequency = frequency;
        this.amplitude = amplitude;
        
        board = -1;
        antenna = -1;
    }
}

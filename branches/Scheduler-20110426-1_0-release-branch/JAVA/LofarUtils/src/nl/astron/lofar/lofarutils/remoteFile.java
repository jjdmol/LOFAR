/*
 * remoteFile.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package nl.astron.lofar.lofarutils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.rmi.RemoteException;

/**
 * @created 13 juni 2006, 14:07
 * @author coolen
 * @version $Id: remoteFile.java 8891 2006-07-11 13:33:21Z coolen $
 */
public class remoteFile implements remoteFileInterface{
    
    private String itsName = "";
    /**
     * Creates a new instance of remoteFile
     */

    public remoteFile(String ext) {
        super();
        itsName=ext;
    }
    
    public byte[] downloadFile(String aFileName) throws RemoteException  {
        if (aFileName != null) {
            try {
                File aFile = new File(aFileName);
                byte buffer[] = new byte[(int)aFile.length()];
                System.out.println("File opened: "+aFile.getName()+" bytesize: "+ buffer.length);
                BufferedInputStream input = new BufferedInputStream(new FileInputStream(aFile));
                input.read(buffer,0,buffer.length);
                input.close();
                return(buffer);
            } catch (Exception ex) {
                System.out.println("RemoteFileImpl: " + ex.getMessage());
                ex.printStackTrace();
                return(null);
            }
        } else {
            System.out.println("RemoteFileAdapter: File not found : "+aFileName);
            return(null);
        }
    }
    
    public boolean uploadFile(byte[] buffer,String aFileName) throws RemoteException  {
        boolean succes=false;
        if (buffer != null && aFileName.length() > 0) {
            try {
                System.out.println("opening File: "+aFileName);
                BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFileName));
                System.out.println("Received buffer length: "+buffer.length);
                output.write(buffer,0,buffer.length);
                output.flush();
                output.close();
                return(true);
            } catch (Exception ex) {
                System.out.println("RemoteFileAdapter: " + ex.getMessage());
                ex.printStackTrace();
                return(succes);
            }
        } else {
            System.out.println("RemoteFileImpl: Buffer empty or filename not given");
            return(succes);
        }
    }
}
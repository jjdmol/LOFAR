/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package nl.astron.lofar.sas.otb.jotdb3;

import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 *
 * @author coolen
 */
public interface jOTDBaccessInterface extends Remote {

    public static final String SERVICENAME = "jOTDBaccess";

    public String login(String name, String pwd,String dbName) throws RemoteException;

}

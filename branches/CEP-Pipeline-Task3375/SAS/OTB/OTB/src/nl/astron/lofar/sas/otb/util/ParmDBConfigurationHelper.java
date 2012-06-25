/*
 * ParmDBConfigurationHelper.java
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

package nl.astron.lofar.sas.otb.util;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import nl.astron.lofar.sas.otb.exceptions.ParmDBConfigurationException;
import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

/**
 * @version $Id$
 * @created June 8, 2006, 9:53 AM
 * @author pompert
 */
public class ParmDBConfigurationHelper{
    
    private static ParmDBConfigurationHelper instance;
    private static Logger logger = Logger.getLogger(ParmDBConfigurationHelper.class);
    private static Document configFile;
    private static final String CONFIGFILE="conf/ParmDBConfiguration.xml";
    private static HashMap<String,String> servers;
    private HashMap<String,String> tables;
    
    /** Creates a new instance of ParmDBConfigurationHelper */
    private ParmDBConfigurationHelper() throws IOException,ParserConfigurationException,SAXException{
        //LOAD XML
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        
        DocumentBuilder builder = factory.newDocumentBuilder();
        configFile = builder.parse(new File(CONFIGFILE));
        
        
    }
    public static ParmDBConfigurationHelper getInstance() throws ParmDBConfigurationException{
        if(instance == null){
            try {
                instance = new ParmDBConfigurationHelper();
            } catch (SAXException ex) {
                logger.error("The ParmDB Configuration XML could not be parsed by SAX", ex);
                ParmDBConfigurationException e = new ParmDBConfigurationException("The ParmDB Configuration XML could not be parsed by SAX");
                e.initCause(ex);
                throw e;
            } catch (ParserConfigurationException ex) {
                logger.error("ParmDB Configuration XML caused an error in the XML parser", ex);
                ParmDBConfigurationException e = new ParmDBConfigurationException("ParmDB Configuration XML caused an error in the XML parser");
                e.initCause(ex);
                throw e;
            } catch (IOException ex) {
                logger.error("ParmDB Configuration XML could not be found/retrieved", ex);
                ParmDBConfigurationException e = new ParmDBConfigurationException("ParmDB Configuration XML could not be found/retrieved");
                e.initCause(ex);
                throw e;
            }
        }
        return instance;
    }
    
    public HashMap<String,String> getParmDBServerInformation() throws ParmDBConfigurationException{
        if(servers == null){
            servers = new HashMap<String,String>();
            String hostname = "lofar17.astron.nl";
            String port = "10668";
            String name = "ParmDB1";
            String friendlyName = "ParmDB on LOFAR17";
            try{
                
                for(int i = 0; i < configFile.getChildNodes().getLength(); i++){
                    Node parmDBNode = configFile.getChildNodes().item(i);
                    logger.trace("Tracing XML node "+parmDBNode.getNodeName());
                    if(parmDBNode.getNodeName()!= null){
                        if(parmDBNode.getNodeName().equalsIgnoreCase("parmdb")){
                            for(int j = 0; j < parmDBNode.getChildNodes().getLength(); j++){
                                Node serversNode = parmDBNode.getChildNodes().item(j);
                                logger.trace("Tracing XML node "+serversNode.getNodeName());
                                if(serversNode.getNodeName()!= null){
                                    if(serversNode.getNodeName().equalsIgnoreCase("servers")){
                                        logger.trace("Tracing XML node "+serversNode.getNodeName());
                                        for(int k = 0; k < serversNode.getChildNodes().getLength(); k++){
                                            Node serverNode = serversNode.getChildNodes().item(k);
                                            logger.trace("Tracing XML node "+serverNode.getNodeName());
                                            if(serverNode.getNodeName()!= null){
                                                if(serverNode.getNodeName().equalsIgnoreCase("server")){
                                                    NamedNodeMap attributes = serverNode.getAttributes();
                                                    name = attributes.getNamedItem("id").getNodeValue();
                                                    friendlyName = attributes.getNamedItem("name").getNodeValue();
                                                    for(int l = 0; l < serversNode.getChildNodes().getLength(); l++){
                                                        Node serverInformationNode = serverNode.getChildNodes().item(l);
                                                        logger.trace("Tracing XML node "+serverInformationNode.getNodeName());
                                                        if(serverInformationNode.getNodeName()!= null){
                                                            
                                                            if(serverInformationNode.getNodeName().equalsIgnoreCase("connection")){
                                                                for(int m = 0; m < serverInformationNode.getChildNodes().getLength(); m++){
                                                                    Node serverConnectionNode = serverInformationNode.getChildNodes().item(m);
                                                                    logger.trace("Tracing XML node "+serverConnectionNode.getNodeName());
                                                                    if(serverConnectionNode.getNodeName()!= null){
                                                                        if(serverConnectionNode.getNodeName().equalsIgnoreCase("rmihostname")){
                                                                            logger.trace("Found XML node name for Server RMI Hostname "+serverConnectionNode.getNodeName());
                                                                            hostname = serverConnectionNode.getTextContent();
                                                                            logger.trace("Found XML node value for Server RMI Hostname "+hostname);
                                                                        }
                                                                        if(serverConnectionNode.getNodeName().equalsIgnoreCase("rmiport")){
                                                                            logger.trace("Found XML node name for Server RMI Port "+serverConnectionNode.getNodeName());
                                                                            port = serverConnectionNode.getTextContent();
                                                                            logger.trace("Found XML node value for Server RMI Port "+port);
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                servers.put("name",name);
                servers.put("friendlyname",friendlyName);
                servers.put("rmihostname",hostname);
                servers.put("rmiport",port);
                logger.trace("Adding To Server information : name: "+name+" ,friendlyname: "+friendlyName+" ,rmihostname: "+hostname+" ,rmiport: "+port);
                
            }catch(Exception e){
                logger.error("ParmDB Configuration XML contains invalid server information", e);
                ParmDBConfigurationException ex = new ParmDBConfigurationException("ParmDB Configuration XML contains invalid server information");
                ex.initCause(e);
                throw ex;
            }
        }
        return servers;
    }
}

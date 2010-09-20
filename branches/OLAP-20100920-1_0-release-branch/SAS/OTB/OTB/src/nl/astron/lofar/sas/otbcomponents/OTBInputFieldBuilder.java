/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package nl.astron.lofar.sas.otbcomponents;

import java.rmi.RemoteException;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.lofarutils.inputfieldbuilder.inputFieldBuilder;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;

/**
 *
 * @author coolen
 */
public class OTBInputFieldBuilder extends inputFieldBuilder {

    static Logger logger = Logger.getLogger(OTBInputFieldBuilder.class);
    static String name="OTBInputFieldBuilder";

    public OTBInputFieldBuilder() {
        super();
    }

    public void setContent(jOTDBnode aNode,jOTDBparam aParam) {

        if (aParam == null ) return;

        String aType="";
        String aUnit="";
        // Format can be obtained from unit in a later stage
        String aFormat = "";
        try {
            aType = OtdbRmi.getRemoteTypes().getParamType(aParam.type);
        } catch (RemoteException e) {
            String aS="Error: GetParamType("+aParam.type+") failed " + e;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
       }
        try {
            aUnit = OtdbRmi.getRemoteTypes().getUnit(aParam.unit);
        } catch (RemoteException e) {
            String aS="Error: GetParamUnit("+aParam.unit+") failed " + e;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
       }

       super.setContent(aType,aUnit,aNode.limits,aFormat,aParam.limits);

    }

}

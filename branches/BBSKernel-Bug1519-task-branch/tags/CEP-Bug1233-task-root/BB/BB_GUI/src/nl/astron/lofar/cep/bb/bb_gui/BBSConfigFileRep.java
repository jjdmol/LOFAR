/*
 * BBSConfigFileRep.java
 *
 * Created on 20 december 2005, 14:29
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.cep.bb.bb_gui;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Vector;


/**
 *
 * @author coolen
 */
public class BBSConfigFileRep {
    // Configuration file representation
    private Integer i1=new Integer(1);
    private BBSConfigParamBaseClass nrPrediffers         =new BBSConfigParamBaseClass("", "nrPrediffers", i1);    
    private Integer i2=new Integer(1);
    private BBSConfigParamBaseClass nrStrategies         =new BBSConfigParamBaseClass("CTRLparams.", "nrStrategies", i2);
    private BBSConfigParamBaseClass BBDBname             =new BBSConfigParamBaseClass("", "BBDBname","");    
    private Boolean b1=new Boolean(false);
    private BBSConfigParamBaseClass writeIndividualParms =new BBSConfigParamBaseClass("", "writeIndividualParms", b1);    
    private BBSConfigParamBaseClass parmSolutionTable    =new BBSConfigParamBaseClass("", "parmSolutionTable", "bbs3ParmSolutions");    
    private BBSConfigParamBaseClass strategy             =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "strategy", "Simple");
    private BBSConfigParamBaseClass MSName               =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "MSName", "");    
    private BBSConfigParamBaseClass generalMSPath        =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "generalMSPath", "./");    
    private BBSConfigParamBaseClass subsetMSPath         =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "subsetMSPath", "./");    
    private BBSConfigParamBaseClass meqTableName         =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "meqTableName", "");    
    private BBSConfigParamBaseClass skyTableName         =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "skyTableName", "");    
    private BBSConfigParamBaseClass DBType               =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "DBType", "aips");    
    private BBSConfigParamBaseClass DBHost               =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "DBHost", "localhost");
    private Integer i3=new Integer(13157);
    private BBSConfigParamBaseClass DBMasterPort         =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "DBMasterPort", i3); 
    private Vector v1=new Vector();
    private BBSConfigParamBaseClass solvableParams       =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "solvableParams",v1);    
    private Vector v2=new Vector();
    private BBSConfigParamBaseClass excludeParams        =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "excludeParams",v2);    
    private BBSConfigParamBaseClass startTime            =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "startTime","");    
    private BBSConfigParamBaseClass endTime              =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "endTime","");    
    private Double d1=new Double(1.0e12);
    private BBSConfigParamBaseClass timeInterval         =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "timeInterval",d1);    
    private Vector v3=new Vector();
    private BBSConfigParamBaseClass sources              =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "sources",v3); 
    private Vector v4=new Vector();
    private BBSConfigParamBaseClass sourcePatches        =new BBSConfigParamBaseClass("CTRLparams.SC1params.MSDBparams.", "sourcePatches",v4); 
    private Integer i4=new Integer(1);
    private BBSConfigParamBaseClass maxNrIterations      =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "maxNrIterations",i4); 
    private Integer i5=new Integer(-1);
    private BBSConfigParamBaseClass fitCriterion         =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "fitCriterion",i5); 
    private Vector v5=new Vector();
    private BBSConfigParamBaseClass stationNames         =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "stationNames",v5);
    private BBSConfigParamBaseClass modelType            =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "modelType","");
    private Boolean b2=new Boolean(false);
    private BBSConfigParamBaseClass calcUVW              =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "calcUVW",b2);
    private Boolean b3=new Boolean(true);
    private BBSConfigParamBaseClass useSVD               =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "useSVD",b3);
    private Integer[] aI1 = {0,3};
    private BBSConfigParamBaseClass correlations         =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "correlations",aI1);
    private Boolean b4=new Boolean(false);
    private BBSConfigParamBaseClass useAutoCorr          =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "useAutoCorr",b4);
    private Boolean b5=new Boolean(false);
    private BBSConfigParamBaseClass controlParmUpdate    =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "controlParmUpdate",b5);
    private Boolean b6=new Boolean(false);
    private BBSConfigParamBaseClass writeParms           =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "writeParms",b6);
    private Boolean b7=new Boolean(true);
    private BBSConfigParamBaseClass writeInDataCol       =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "writeInDataCol",b7);
    private Integer i6=new Integer(0);
    private BBSConfigParamBaseClass startChan            =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "startChan",i6); 
    private Integer i7=new Integer(0);
    private BBSConfigParamBaseClass endChan              =new BBSConfigParamBaseClass("CTRLparams.SC1params.", "endChan",i7); 
    
    private File itsFile=null;
    
    
    

    /** Creates a new instance of BBSConfigFileRep */
    public BBSConfigFileRep() {
    }
    
     public boolean saveFile(File aFile) {
        try {
            BufferedWriter out = new BufferedWriter(new FileWriter(aFile));
            
            out.write(BBSIntegerToString(nrPrediffers));
            out.write(BBSIntegerToString(nrStrategies));
            out.write(BBSStringToString(BBDBname));
            String aB=BBSBooleanToString(writeIndividualParms);
            out.write(aB);
            if (((Boolean) writeIndividualParms.getValue())){
                out.write(BBSStringToString(parmSolutionTable));
            }
            String aStratType=BBSStringToString(strategy);
            out.write(aStratType);
            if (((String) strategy.getValue()).equals("WritePredData")) {
                out.write(BBSBooleanToString(writeInDataCol));
            } else {
                out.write(BBSDoubleToString(timeInterval));
                out.write(BBSIntegerToString(maxNrIterations));
                out.write(BBSIntegerToString(fitCriterion));
                out.write(BBSBooleanToString(useSVD));
                out.write(BBSBooleanToString(controlParmUpdate));
                out.write(BBSBooleanToString(writeParms));                
            }
            out.write(BBSStringToString(MSName));
            out.write(BBSStringToString(generalMSPath));
            out.write(BBSStringToString(subsetMSPath));
            out.write(BBSStringToString(meqTableName));
            out.write(BBSStringToString(skyTableName));
            String aDBType = BBSStringToString(DBType);
            out.write(aDBType);
            if (((String)DBType.getValue()).equals("bdbrepl")) {
                out.write(BBSStringToString(DBHost));
                out.write(BBSIntegerToString(DBMasterPort));
            }
            out.write(BBSVectorToString(solvableParams));
            out.write(BBSVectorToString(excludeParams));
            out.write(BBSStringToString(startTime));
            out.write(BBSStringToString(endTime));
            out.write(BBSIntegerToString(startChan));
            out.write(BBSIntegerToString(endChan));
            out.write(BBSVectorToString(sources));
            out.write(BBSVectorToString(sourcePatches));
            out.write(BBSVectorToString(stationNames));
            out.write(BBSStringToString(modelType));
            out.write(BBSBooleanToString(calcUVW));
            out.write(BBSIntArrayToString(correlations));
            out.write(BBSBooleanToString(useAutoCorr));
        
            
            out.close();
        } catch (IOException e) {
            System.out.println("Error writing configurationfile: "+e);
            return false;
        }
        
        return true;
    }
    
    public void restoreParams(String aStage) {
        // aStage default == Gui needs to be reset to default values
        // aStage old     == Gui needs to be reset to old values        
        
        boolean aDefaultValue=false;
        if (aStage.equals("default")) {
            aDefaultValue=true;
        }
        
        restoreParam(nrPrediffers, aDefaultValue);
        restoreParam(nrStrategies, aDefaultValue);
        restoreParam(DBMasterPort, aDefaultValue);
        restoreParam(startTime, aDefaultValue);
        restoreParam(endTime, aDefaultValue);
        restoreParam(maxNrIterations, aDefaultValue);
        restoreParam(fitCriterion, aDefaultValue);
        restoreParam(timeInterval, aDefaultValue);
        restoreParam(startChan, aDefaultValue);
        restoreParam(endChan, aDefaultValue);
        restoreParam(writeIndividualParms, aDefaultValue);
        restoreParam(calcUVW, aDefaultValue);
        restoreParam(useSVD, aDefaultValue);
        restoreParam(useAutoCorr, aDefaultValue);
        restoreParam(controlParmUpdate, aDefaultValue);
        restoreParam(writeParms, aDefaultValue);
        restoreParam(writeInDataCol, aDefaultValue);
        restoreParam(BBDBname, aDefaultValue);
        restoreParam(parmSolutionTable, aDefaultValue);
        restoreParam(useSVD, aDefaultValue);
        restoreParam(strategy, aDefaultValue);
        restoreParam(MSName, aDefaultValue);
        restoreParam(generalMSPath, aDefaultValue);
        restoreParam(subsetMSPath, aDefaultValue);
        restoreParam(meqTableName, aDefaultValue);
        restoreParam(skyTableName, aDefaultValue);
        restoreParam(DBType, aDefaultValue);
        restoreParam(DBHost, aDefaultValue);
        restoreParam(modelType, aDefaultValue);
        restoreParam(solvableParams, aDefaultValue);
        restoreParam(excludeParams, aDefaultValue);
        restoreParam(sources, aDefaultValue);
        restoreParam(sourcePatches, aDefaultValue);
        restoreParam(stationNames, aDefaultValue);
        restoreParam(writeParms, aDefaultValue);
        restoreParam(writeInDataCol, aDefaultValue);
        restoreParam(correlations, aDefaultValue);
    }
    
    public boolean restoreParam(String aBBSName,boolean defaultValue){
        boolean flag=true;
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("nrPrediffers")) {
            aClass=nrPrediffers;
        } else if (aBBSName.equals("nrStrategies")) {
            aClass=nrStrategies;
        } else if (aBBSName.equals("DBMasterPort")) {
            aClass=DBMasterPort;
        } else if (aBBSName.equals("startTime")) {
            aClass=startTime;
        } else if (aBBSName.equals("endTime")) {
            aClass=endTime;
        } else if (aBBSName.equals("maxNrIterations")) {
            aClass=maxNrIterations;
        } else if (aBBSName.equals("fitCriterion")) {
            aClass=fitCriterion;
        } else if (aBBSName.equals("timeInterval")) {
            aClass=timeInterval;
        } else if (aBBSName.equals("startChan")) {
            aClass=startChan;
        } else if (aBBSName.equals("endChan")) {
            aClass=endChan;
        } else if (aBBSName.equals("writeIndividualParms")) {
            aClass=writeIndividualParms;
        } else if (aBBSName.equals("calcUVW")) {
            aClass=calcUVW;
        } else if (aBBSName.equals("useSVD")) {
            aClass=useSVD;
        } else if (aBBSName.equals("useAutoCorr")) {
            aClass=useAutoCorr;
        } else if (aBBSName.equals("controlParmUpdate")) {
            aClass=controlParmUpdate;
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
        } else if (aBBSName.equals("BBDBname")) {
            aClass=BBDBname;
        } else if (aBBSName.equals("parmSolutionTable")) {
            aClass=parmSolutionTable;
        } else if (aBBSName.equals("useSVD")) {
            aClass=useSVD;
        } else if (aBBSName.equals("strategy")) {
            aClass=strategy;
        } else if (aBBSName.equals("MSName")) {
            aClass=MSName;
        } else if (aBBSName.equals("generalMSPath")) {
            aClass=generalMSPath;
        } else if (aBBSName.equals("subsetMSPath")) {
            aClass=subsetMSPath;
        } else if (aBBSName.equals("meqTableName")) {
            aClass=meqTableName;
        } else if (aBBSName.equals("skyTableName")) {
            aClass=skyTableName;
        } else if (aBBSName.equals("DBType")) {
            aClass=DBType;
        } else if (aBBSName.equals("DBHost")) {
            aClass=DBHost;
        } else if (aBBSName.equals("modelType")) {
            aClass=modelType;
        } else if (aBBSName.equals("solvableParams")) {
            aClass=solvableParams;
        } else if (aBBSName.equals("excludeParams")) {
            aClass=excludeParams;
        } else if (aBBSName.equals("sources")) {
            aClass=sources;
        } else if (aBBSName.equals("sourcePatches")) {
            aClass=sourcePatches;
        } else if (aBBSName.equals("stationNames")) {
            aClass=stationNames;
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
        } else if (aBBSName.equals("correlations")) {
            aClass=correlations;
        } else {
            System.out.println("Error: Unknown Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            restoreParam(aClass,defaultValue);
        }
        return flag;
        
    }
    
    public void clearButTopLevel() {
        // Set all values to default except Toplevel stuff
        boolean aDefaultValue=true;
        
        restoreParam(startTime, aDefaultValue);
        restoreParam(endTime, aDefaultValue);
        restoreParam(maxNrIterations, aDefaultValue);
        restoreParam(fitCriterion, aDefaultValue);
        restoreParam(timeInterval, aDefaultValue);
        restoreParam(startChan, aDefaultValue);
        restoreParam(endChan, aDefaultValue);
        restoreParam(writeIndividualParms, aDefaultValue);
        restoreParam(calcUVW, aDefaultValue);
        restoreParam(useSVD, aDefaultValue);
        restoreParam(useAutoCorr, aDefaultValue);
        restoreParam(controlParmUpdate, aDefaultValue);
        restoreParam(writeParms, aDefaultValue);
        restoreParam(writeInDataCol, aDefaultValue);
        restoreParam(parmSolutionTable, aDefaultValue);
        restoreParam(useSVD, aDefaultValue);
        restoreParam(strategy, aDefaultValue);
        restoreParam(modelType, aDefaultValue);
        restoreParam(solvableParams, aDefaultValue);
        restoreParam(excludeParams, aDefaultValue);
        restoreParam(sources, aDefaultValue);
        restoreParam(sourcePatches, aDefaultValue);
        restoreParam(stationNames, aDefaultValue);
        restoreParam(writeParms, aDefaultValue);
        restoreParam(writeInDataCol, aDefaultValue);
        restoreParam(correlations, aDefaultValue);        
    }
    
    public void clearTopLevel() {
        
        boolean aDefaultValue=true;
        
        restoreParam(nrPrediffers, aDefaultValue);
        restoreParam(nrStrategies, aDefaultValue);
        restoreParam(DBMasterPort, aDefaultValue);
        restoreParam(BBDBname, aDefaultValue);
        restoreParam(MSName, aDefaultValue);
        restoreParam(generalMSPath, aDefaultValue);
        restoreParam(subsetMSPath, aDefaultValue);
        restoreParam(meqTableName, aDefaultValue);
        restoreParam(skyTableName, aDefaultValue);
        restoreParam(DBType, aDefaultValue);
        restoreParam(DBHost, aDefaultValue);
    }
    
    public boolean loadFile(File aFile) {
        try {
            BufferedReader in = new BufferedReader(new FileReader(aFile));
            String aStr;
            
            while ((aStr = in.readLine()) != null) {
                String keyword="";
                String value="";
                // Split String in main parts keyword = value (# comment)
                //skip empty lines
                if (aStr.matches("^.*=.*$")) {
                    String [] aS=aStr.split("=");
                    //get last part from keyword
                    keyword=aS[0];
                    int i= keyword.lastIndexOf('.');
                    if (i >= 0 && i < keyword.length() -1) {
                        keyword = keyword.substring(i+1);
                    }
                    
                    // check if value != empty if so chop comments of
                    if(aS.length > 1) {
                        value=aS[1].split("#")[0];
                    }
                   saveParams(keyword,value);
//                   System.out.println("Found: " +keyword + " Value: "+value);
                }
            }
            itsFile=aFile;
            in.close();            
        } catch (IOException e) {
            System.out.println("Error reading from file: "+aFile.getName());
            return false;
        }

        return true;
    }
    
       public String getParams(String aBBSName) {
        boolean flag = true;
        String valType="";
        BBSConfigParamBaseClass aClass=null;

        if (aBBSName.equals("nrPrediffers")) {
            aClass=nrPrediffers;
            valType="Integer";
        } else if (aBBSName.equals("nrStrategies")) {
            aClass=nrStrategies;
            valType="Integer";
        } else if (aBBSName.equals("DBMasterPort")) {
            aClass=DBMasterPort;
            valType="Integer";
        } else if (aBBSName.equals("startTime")) {
            aClass=startTime;
            valType="String";
        } else if (aBBSName.equals("endTime")) {
            aClass=endTime;
            valType="String";
        } else if (aBBSName.equals("maxNrIterations")) {
            aClass=maxNrIterations;
            valType="Integer";
        } else if (aBBSName.equals("fitCriterion")) {
            aClass=fitCriterion;
            valType="Integer";
        } else if (aBBSName.equals("timeInterval")) {
            aClass=timeInterval;
            valType="Double";
        } else if (aBBSName.equals("startChan")) {
            aClass=startChan;
            valType="Integer";
        } else if (aBBSName.equals("endChan")) {
            aClass=endChan;
            valType="Integer";
        } else if (aBBSName.equals("writeIndividualParms")) {
            aClass=writeIndividualParms;
            valType="Boolean";
        } else if (aBBSName.equals("calcUVW")) {
            aClass=calcUVW;
            valType="Boolean";
        } else if (aBBSName.equals("useSVD")) {
            aClass=useSVD;
            valType="Boolean";
        } else if (aBBSName.equals("useAutoCorr")) {
            aClass=useAutoCorr;
            valType="Boolean";
        } else if (aBBSName.equals("controlParmUpdate")) {
            aClass=controlParmUpdate;
            valType="Boolean";
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
            valType="Boolean";
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
            valType="Boolean";
        } else if (aBBSName.equals("BBDBname")) {
            aClass=BBDBname;
            valType="String";
        } else if (aBBSName.equals("parmSolutionTable")) {
            aClass=parmSolutionTable;
            valType="String";
        } else if (aBBSName.equals("strategy")) {
            aClass=strategy;
            valType="String";
        } else if (aBBSName.equals("MSName")) {
            aClass=MSName;
            valType="String";
        } else if (aBBSName.equals("generalMSPath")) {
            aClass=generalMSPath;
            valType="String";
        } else if (aBBSName.equals("subsetMSPath")) {
            aClass=subsetMSPath;
            valType="String";
        } else if (aBBSName.equals("meqTableName")) {
            aClass=meqTableName;
            valType="String";
        } else if (aBBSName.equals("skyTableName")) {
            aClass=skyTableName;
            valType="String";
        } else if (aBBSName.equals("DBType")) {
            aClass=DBType;
            valType="String";
        } else if (aBBSName.equals("DBHost")) {
            aClass=DBHost;
            valType="String";
        } else if (aBBSName.equals("modelType")) {
            aClass=modelType;
            valType="String";
        } else if (aBBSName.equals("solvableParams")) {
            aClass=solvableParams;
            valType="Vector";
        } else if (aBBSName.equals("excludeParams")) {
            aClass=excludeParams;
            valType="Vector";
        } else if (aBBSName.equals("sources")) {
            aClass=sources;
            valType="Vector";
        } else if (aBBSName.equals("sourcePatches")) {
            aClass=sourcePatches;
            valType="Vector";
        } else if (aBBSName.equals("stationNames")) {
            aClass=stationNames;
            valType="Vector";
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
            valType="Vector";
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
            valType="Vector";
        } else if (aBBSName.equals("correlations")) {
            aClass=correlations;
            valType="IntArray";
        } else {
            System.out.println("Error: Unknown Paramname: "+aBBSName);
            flag=false;
        }
        
        String returnString="";
        
        if (flag) {
            if (valType.equals("Integer")) {
                returnString=Integer.toString(getIntegerParam(aClass,"new"));
            } else if (valType.equals("Double")) {
                returnString=Double.toString(getDoubleParam(aClass,"new"));
            } else if (valType.equals("Boolean")) {
                returnString=getBooleanParam(aClass,"new").toString();
            } else if (valType.equals("String")) {
                returnString=getStringParam(aClass, "new");
            } else if (valType.equals("Vector")) {
               Vector aVec = getVectorParam(aClass, "new");
                for (int i=0; i< aVec.size();i++) {
                    if (i > 0) {
                        returnString+=",";
                    }
                    String aS=(String)aVec.get(i);
                    returnString+=aS.replaceAll("\"","");
                }
 
            } else if (valType.equals("IntArray")) {
                Integer [] anI= getIntArrayParam(aClass,"new");
                for (int i=0;i<anI.length;i++) {
                    if (i > 0) {
                        returnString+=",";
                    }
                    returnString+=anI[i].toString();
                }
  
            } else {
                System.out.println("Unknown valType: "+valType);
            }
        }
   return returnString;   
   }
    
    public boolean saveParams(String aBBSName, String aV) {
        boolean flag=true;
        String valType="";
        BBSConfigParamBaseClass aClass=null;

        if (aBBSName.equals("nrPrediffers")) {
            aClass=nrPrediffers;
            valType="Integer";
        } else if (aBBSName.equals("nrStrategies")) {
            aClass=nrStrategies;
            valType="Integer";
        } else if (aBBSName.equals("DBMasterPort")) {
            aClass=DBMasterPort;
            valType="Integer";
        } else if (aBBSName.equals("startTime")) {
            aClass=startTime;
            valType="String";
        } else if (aBBSName.equals("endTime")) {
            aClass=endTime;
            valType="String";
        } else if (aBBSName.equals("maxNrIterations")) {
            aClass=maxNrIterations;
            valType="Integer";
        } else if (aBBSName.equals("fitCriterion")) {
            aClass=fitCriterion;
            valType="Integer";
        } else if (aBBSName.equals("timeInterval")) {
            aClass=timeInterval;
            valType="Double";
        } else if (aBBSName.equals("startChan")) {
            aClass=startChan;
            valType="Integer";
        } else if (aBBSName.equals("endChan")) {
            aClass=endChan;
            valType="Integer";
        } else if (aBBSName.equals("writeIndividualParms")) {
            aClass=writeIndividualParms;
            valType="Boolean";
        } else if (aBBSName.equals("calcUVW")) {
            aClass=calcUVW;
            valType="Boolean";
        } else if (aBBSName.equals("useSVD")) {
            aClass=useSVD;
            valType="Boolean";
        } else if (aBBSName.equals("useAutoCorr")) {
            aClass=useAutoCorr;
            valType="Boolean";
        } else if (aBBSName.equals("controlParmUpdate")) {
            aClass=controlParmUpdate;
            valType="Boolean";
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
            valType="Boolean";
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
            valType="Boolean";
        } else if (aBBSName.equals("BBDBname")) {
            aClass=BBDBname;
            valType="String";
        } else if (aBBSName.equals("parmSolutionTable")) {
            aClass=parmSolutionTable;
            valType="String";
        } else if (aBBSName.equals("strategy")) {
            aClass=strategy;
            valType="String";
        } else if (aBBSName.equals("MSName")) {
            aClass=MSName;
            valType="String";
        } else if (aBBSName.equals("generalMSPath")) {
            aClass=generalMSPath;
            valType="String";
        } else if (aBBSName.equals("subsetMSPath")) {
            aClass=subsetMSPath;
            valType="String";
        } else if (aBBSName.equals("meqTableName")) {
            aClass=meqTableName;
            valType="String";
        } else if (aBBSName.equals("skyTableName")) {
            aClass=skyTableName;
            valType="String";
        } else if (aBBSName.equals("DBType")) {
            aClass=DBType;
            valType="String";
        } else if (aBBSName.equals("DBHost")) {
            aClass=DBHost;
            valType="String";
        } else if (aBBSName.equals("modelType")) {
            aClass=modelType;
            valType="String";
        } else if (aBBSName.equals("solvableParams")) {
            aClass=solvableParams;
            valType="Vector";
        } else if (aBBSName.equals("excludeParams")) {
            aClass=excludeParams;
            valType="Vector";
        } else if (aBBSName.equals("sources")) {
            aClass=sources;
            valType="Vector";
        } else if (aBBSName.equals("sourcePatches")) {
            aClass=sourcePatches;
            valType="Vector";
        } else if (aBBSName.equals("stationNames")) {
            aClass=stationNames;
            valType="Vector";
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
            valType="Vector";
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
            valType="Vector";
        } else if (aBBSName.equals("correlations")) {
            aClass=correlations;
            valType="IntArray";
       } else {
            System.out.println("Error: Unknown Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            if (valType.equals("Integer")) {
                setParam(aClass,Integer.valueOf(aV).intValue());
            } else if (valType.equals("Double")) {
                setParam(aClass,Double.valueOf(aV).doubleValue());
            } else if (valType.equals("Boolean")) {
                boolean aB=false;
                if (aV.equals("T")) {
                    aB=true;
                }
                setParam(aClass,aB);
            } else if (valType.equals("String")) {
                setParam(aClass,aV);
            } else if (valType.equals("Vector")) {
                Vector<String> aVec = new Vector<String>();
                // remove [] from String and split on ,
                String [] aS=(aV.replace("[", "").replace("]", "")).split(",");
                for (int i=0; i<aS.length;i++) {
                    if (!aS[i].equals("")){
                        aVec.add(aS[i]);
                    }
                }
                setParam(aClass,aVec);
            } else if (valType.equals("IntArray")) {
                // remove [] from String and split on ,
                String [] aS=(aV.replace("[", "").replace("]","")).split(",");
                Integer aIA [] = new Integer[aS.length];
                for (int i=0; i<aS.length;i++) {
                    aIA[i]=Integer.valueOf(aS[i]).intValue();
                }
                setParam(aClass,aIA);
            } else {
                System.out.println("Unknown valType: "+valType);
            }
        }
        return flag;
    }
    
    public void setFile(File aFile) {
        itsFile=aFile;
    }
    
    //
    //   ------------------------   Private Methods -----------------------------
    //
    
    private String BBSStringToString(BBSConfigParamBaseClass aClass) {
        String aS=aClass.getPrefix()+aClass.getBBSName()+"="+aClass.getValue()+"\n";
        return aS;
    }
    
    private String BBSIntegerToString(BBSConfigParamBaseClass aClass) {
        String aS=aClass.getPrefix()+aClass.getBBSName()+"="+((Integer)aClass.getValue()).toString()+"\n";
        return aS;
    }

    private String BBSBooleanToString(BBSConfigParamBaseClass aClass) {
        String aB="F";
        if (((Boolean)aClass.getValue()).booleanValue()) {
            aB="T";
        }
        String aS=aClass.getPrefix()+aClass.getBBSName()+"="+aB+"\n";
        return aS;
    }
    
    private String BBSVectorToString(BBSConfigParamBaseClass aClass) {
        String aS=aClass.getPrefix()+aClass.getBBSName()+"=[";
        Vector aSP=(Vector) aClass.getValue();
        
        for (int i=0;i<aSP.size();i++) {
            if (i>0){
                aS+=",";
            }            
            aS+="\""+aSP.get(i).toString()+"\"";
        }
        aS+="]\n";
        return aS;
    }

    private String BBSIntArrayToString(BBSConfigParamBaseClass aClass) {
        String aS=aClass.getPrefix()+aClass.getBBSName()+"=[";
        Integer [] aSP=(Integer[]) aClass.getValue();
        
        for (int i=0;i<aSP.length;i++) {
            if (i>0){
                aS+=",";
            }
            aS+=aSP[i].toString();
        }
        aS+="]\n";
        return aS;
    }

    private String BBSDoubleToString(BBSConfigParamBaseClass aClass) {
        String aS=aClass.getPrefix()+aClass.getBBSName()+"="+((Double)aClass.getValue()).toString()+"\n";
        return aS;
    }
    
    private void setParam(BBSConfigParamBaseClass aClass, Object aNewValue) {
        aClass.setValue(aNewValue);
    }
    
    private void restoreParam(BBSConfigParamBaseClass aClass, boolean defaultValue) {
        if (defaultValue) {
            aClass.setValue(aClass.getDefaultValue());
        } else {
            aClass.setValue(aClass.getOldValue());
        }
    }
    
    private Object getParam(BBSConfigParamBaseClass aClass,String valIdent) {
        if (valIdent.equals("old")) {
            return aClass.getOldValue();
        } else if (valIdent.equals("default")) {
            return aClass.getDefaultValue();
        } else if (valIdent.equals("new")) {
            return aClass.getValue();
        } else {
            return null;
        }
    }
    

    
    // Set Params with IntegerValues
    private boolean setParam(String aBBSName,int aV) {
        boolean flag=true;
        Integer i = new Integer(aV);
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("nrPrediffers")) {
            aClass=nrPrediffers;
        } else if (aBBSName.equals("nrStrategies")) {
            aClass=nrStrategies;
        } else if (aBBSName.equals("DBMasterPort")) {
            aClass=DBMasterPort;
        } else if (aBBSName.equals("maxNrIterations")) {
            aClass=maxNrIterations;
        } else if (aBBSName.equals("fitCriterion")) {
            aClass=fitCriterion;
        } else {
            System.out.println("Error: Unknown Integer Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            setParam(aClass,i);
        }
        return flag;
    }
    
    // Set Params with DoubleValues
    private boolean setParam(String aBBSName,double aV) {
        boolean flag=true;
        Double d = new Double(aV);
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("timeInterval")) {
            aClass=timeInterval;
        } else if (aBBSName.equals("startChan")) {
            aClass=startChan;
        } else if (aBBSName.equals("endChan")) {
            aClass=endChan;
        } else {
            System.out.println("Error: Unknown Double Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            setParam(aClass,d);
        }
        return flag;
    }
    
    // Set Params with BooleanValues
    private boolean setParam(String aBBSName,boolean aV) {
        boolean flag=true;
        Boolean b = new Boolean(aV);
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("writeIndividualParms")) {
            aClass=writeIndividualParms;
        } else if (aBBSName.equals("calcUVW")) {
            aClass=calcUVW;
        } else if (aBBSName.equals("useSVD")) {
            aClass=useSVD;
        } else if (aBBSName.equals("useAutoCorr")) {
            aClass=useAutoCorr;
        } else if (aBBSName.equals("controlParmUpdate")) {
            aClass=controlParmUpdate;
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
        } else {
            System.out.println("Error: Unknown Boolean Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            setParam(aClass,b);
        }
        return flag;
    }

    // Set Params with StringValues
    private boolean setParam(String aBBSName,String aV) {
        boolean flag=true;
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("BBDBname")) {
            aClass=BBDBname;
        } else if (aBBSName.equals("parmSolutionTable")) {
            aClass=parmSolutionTable;
        } else if (aBBSName.equals("strategy")) {
            aClass=strategy;
        } else if (aBBSName.equals("MSName")) {
            aClass=MSName;
        } else if (aBBSName.equals("generalMSPath")) {
            aClass=generalMSPath;
        } else if (aBBSName.equals("subsetMSPath")) {
            aClass=subsetMSPath;
        } else if (aBBSName.equals("meqTableName")) {
            aClass=meqTableName;
        } else if (aBBSName.equals("skyTableName")) {
            aClass=skyTableName;
        } else if (aBBSName.equals("DBType")) {
            aClass=DBType;
        } else if (aBBSName.equals("DBHost")) {
            aClass=DBHost;
        } else if (aBBSName.equals("modelType")) {
            aClass=modelType;
        } else if (aBBSName.equals("startTime")) {
            aClass=startTime;
        } else if (aBBSName.equals("endTime")) {
            aClass=endTime;
            
        } else {
            System.out.println("Error: Unknown String Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            setParam(aClass,aV);
        }
        return flag;
    }

    // Set Params with VectorValues
    private boolean setParam(String aBBSName,Vector aV) {
        boolean flag=true;
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("solvableParams")) {
            aClass=solvableParams;
        } else if (aBBSName.equals("excludeParams")) {
            aClass=excludeParams;
        } else if (aBBSName.equals("sources")) {
            aClass=sources;
        } else if (aBBSName.equals("sourcePatches")) {
            aClass=sourcePatches;
        } else if (aBBSName.equals("stationNames")) {
            aClass=stationNames;
        } else if (aBBSName.equals("writeParms")) {
            aClass=writeParms;
        } else if (aBBSName.equals("writeInDataCol")) {
            aClass=writeInDataCol;
        } else {
            System.out.println("Error: Unknown Vector Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            setParam(aClass,aV);
        }
        return flag;
    }

    // Set Params with IntArrayValues
    private boolean setParam(String aBBSName,Integer[] aV) {
        boolean flag=true;
        BBSConfigParamBaseClass aClass=null;
        if (aBBSName.equals("correlations")) {
            aClass=correlations;
        } else if (aBBSName.equals("sources")) {
            aClass=correlations;
        } else {
            System.out.println("Error: Unknown intArray Paramname: "+aBBSName);
            flag=false;
        }
        if (flag) {
            setParam(aClass,aV);
        }
        return flag;
    }
    // Get Params with IntegerValues
    private Integer getIntegerParam(BBSConfigParamBaseClass aClass,String valIdent) {
         return (Integer) getParam(aClass, valIdent);
    }
    
    // Get Params with DoubleValues
    private Double getDoubleParam(BBSConfigParamBaseClass aClass,String valIdent) {
         return (Double) getParam(aClass,valIdent);
    }
    
    // Get Params with BooleanValues
    private Boolean getBooleanParam(BBSConfigParamBaseClass aClass,String valIdent) {
         return (Boolean) getParam(aClass,valIdent);
    }

    // Get Params with StringValues
    private String getStringParam(BBSConfigParamBaseClass aClass,String valIdent) {
        return (String) getParam(aClass,valIdent);
    }

    // Get Params with VectorValues
    private Vector getVectorParam(BBSConfigParamBaseClass aClass,String valIdent) {
        return (Vector) getParam(aClass,valIdent);
    }

    // Get Params with IntArrayValues
    private Integer []  getIntArrayParam(BBSConfigParamBaseClass aClass, String valIdent) {
        return (Integer [] ) getParam(aClass,valIdent);
    }    
}

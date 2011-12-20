/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.astron.lofar.sas.otb.objects;

import java.util.ArrayList;
import java.util.BitSet;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.util.IBeam;

/**
 *
 * @author coolen
 */
public class Beam implements IBeam{
    
    
    @Override
    public String getType() {
        return itsType;
    }

    public String getAngle1() {
        return itsAngle1;
    }

    public void setAngle1(String anAngle1) {
        this.itsAngle1 = anAngle1;
    }

    public String getAngle2() {
        return itsAngle2;
    }

    public void setAngle2(String anAngle2) {
        this.itsAngle2 = anAngle2;
    }

    public String getBeamletList() {
        return itsBeamletList;
    }

    public void setBeamletList(String aBeamletList) {
        this.itsBeamletList = aBeamletList;
    }

    public String getDirectionType() {
        return itsDirectionType;
    }

    public void setDirectionType(String aDirectionType) {
        this.itsDirectionType = aDirectionType;
    }

    public String getDuration() {
        return itsDuration;
    }

    public void setDuration(String aDuration) {
        this.itsDuration = aDuration;
    }

    public String getMaximizeDuration() {
        return itsMaximizeDuration;
    }

    public void setMaximizeDuration(String aMaximizeDuration) {
        this.itsMaximizeDuration = aMaximizeDuration;
    }

    public String getMomID() {
        return itsMomID;
    }

    public void setMomID(String aMomID) {
        this.itsMomID = aMomID;
    }

    public String getNrTabRings() {
        return itsNrTabRings;
    }

    public void setNrTabRings(String aNrTabRings) {
        this.itsNrTabRings = aNrTabRings;
    }

    public String getNrTiedArrayBeams() {
        return itsNrTiedArrayBeams;
    }

    public void setNrTiedArrayBeams(String aNrTiedArrayBeams) {
        this.itsNrTiedArrayBeams = aNrTiedArrayBeams;
    }

    public String getStartTime() {
        return itsStartTime;
    }

    public void setStartTime(String aStartTime) {
        this.itsStartTime = aStartTime;
    }

    public String getSubbandList() {
        return itsSubbandList;
    }

    public void setSubbandList(String aSubbandList) {
        this.itsSubbandList = aSubbandList;
    }

    public String getTabRingSize() {
        return itsTabRingSize;
    }

    public void setTabRingSize(String aTabRingSize) {
        this.itsTabRingSize = aTabRingSize;
    }

    public String getTarget() {
        return itsTarget;
    }

    public void setTarget(String aTarget) {
        this.itsTarget = aTarget;
    }

    public ArrayList<TiedArrayBeam> getTiedArrayBeams() {
        return itsTiedArrayBeams;
    }
    
    public void setTiedArrayBeams(ArrayList<TiedArrayBeam> aTiedArrayBeams) {
        this.itsTiedArrayBeams = aTiedArrayBeams;
        this.itsNrTiedArrayBeams = Integer.toString(aTiedArrayBeams.size());
    }

    public String getCoordType() {
        return itsCoordType;
    }

    public void setCoordType(String aCoordType) {
        this.itsCoordType = aCoordType;
    }
    
    public String getDirectionTypeChoices() {
        return itsDirectionTypeChoices;
    }

    public void setDirectionTypeChoices(String directionTypeChoices) {
        this.itsDirectionTypeChoices = directionTypeChoices;
    }
    
    public BitSet getBeamletBitSet() {
        return LofarUtils.beamletToBitSet(LofarUtils.expandedArrayString(itsBeamletList));
    }
    
    
    private static String            itsType="Beam";

    private String                   itsTarget;
    private String                   itsAngle1; 
    private String                   itsAngle2;
    private String                   itsCoordType;
    private String                   itsDirectionType;
    private String                   itsStartTime;
    private String                   itsDuration;
    private String                   itsMaximizeDuration;
    private String                   itsSubbandList;
    private String                   itsBeamletList;
    private String                   itsMomID;
    private String                   itsNrTiedArrayBeams;
    private String                   itsNrTabRings;
    private String                   itsTabRingSize;
    private ArrayList<TiedArrayBeam> itsTiedArrayBeams = new ArrayList<>(); 
    private String                   itsDirectionTypeChoices;
}

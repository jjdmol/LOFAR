/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.astron.lofar.sas.otb.objects;

import nl.astron.lofar.sas.otb.util.IBeam;

/**
 *
 * @author coolen
 */
public class AnaBeam implements IBeam {
    
    private static String itsType="AnaBeam";

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

    public String getCoordType() {
        return itsCoordType;
    }

    public void setCoordType(String aCoordType) {
        this.itsCoordType = aCoordType;
    }

    public String getDirectionType() {
        return itsDirectionType;
    }

    public void setDirectionType(String aDirectionType) {
        this.itsDirectionType = aDirectionType;
    }

    public String getDirectionTypeChoices() {
        return itsDirectionTypeChoices;
    }

    public void setDirectionTypeChoices(String directionTypeChoices) {
        this.itsDirectionTypeChoices = directionTypeChoices;
    }

    public String getDuration() {
        return itsDuration;
    }

    public void setDuration(String aDuration) {
        this.itsDuration = aDuration;
    }



    public String getRank() {
        return itsRank;
    }

    public void setRank(String aRank) {
        this.itsRank = aRank;
    }

    public String getRankChoices() {
        return itsRankChoices;
    }

    public void setRankChoices(String rankChoices) {
        this.itsRankChoices = rankChoices;
    }

    public String getStartTime() {
        return itsStartTime;
    }

    public void setStartTime(String aStartTime) {
        this.itsStartTime = aStartTime;
    }

    public String getTarget() {
        return itsTarget;
    }

    public void setTarget(String aTarget) {
        this.itsTarget = aTarget;
    }
    
    @Override
    public AnaBeam clone() {
        AnaBeam clone = new AnaBeam();
        clone.itsTarget = itsTarget;
        clone.itsAngle1 = itsAngle1;
        clone.itsAngle2 = itsAngle2;
        clone.itsCoordType = itsCoordType;
        clone.itsDirectionType = itsDirectionType;
        clone.itsDirectionTypeChoices = itsDirectionTypeChoices;
        clone.itsStartTime = itsStartTime;
        clone.itsDuration = itsDuration;
        clone.itsRank = itsRank;
        clone.itsRankChoices = itsRankChoices;
        return clone;
    }
    
    
    private String itsTarget;
    private String itsAngle1;
    private String itsAngle2;
    private String itsCoordType;
    private String itsDirectionType;
    private String itsDirectionTypeChoices;
    private String itsStartTime;
    private String itsDuration;
    private String itsRank;
    private String itsRankChoices;
}
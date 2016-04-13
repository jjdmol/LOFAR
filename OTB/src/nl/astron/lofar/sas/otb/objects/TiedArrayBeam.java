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
public class TiedArrayBeam implements IBeam {
    private static String itsType="TiedArrayBeam";


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

    public String getCoherent() {
        return itsCoherent;
    }

    public void setCoherent(String aCoherent) {
        this.itsCoherent = aCoherent;
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

    public String getDispersionMeasure() {
        return itsDispersionMeasure;
    }

    public void setDispersionMeasure(String aDispersionMeasure) {
        this.itsDispersionMeasure = aDispersionMeasure;
    }
    
    public String getDirectionTypeChoices() {
        return itsDirectionTypeChoices;
    }

    public void setDirectionTypeChoices(String directionTypeChoices) {
        this.itsDirectionTypeChoices =directionTypeChoices;
    }
    
    @Override
    public TiedArrayBeam clone() {
        TiedArrayBeam clone = new TiedArrayBeam();
        clone.itsAngle1 = itsAngle1;
        clone.itsAngle2 = itsAngle2;
        clone.itsCoordType = itsCoordType;
        clone.itsDirectionType = itsDirectionType;
        clone.itsDispersionMeasure = itsDispersionMeasure;
        clone.itsCoherent = itsCoherent;
        clone.itsDirectionTypeChoices = itsDirectionTypeChoices;
        
        return clone;
    }
    
    private String itsAngle1;
    private String itsAngle2;
    private String itsCoordType;
    private String itsDirectionType;
    private String itsDispersionMeasure;
    private String itsCoherent;
    private String itsDirectionTypeChoices;
}

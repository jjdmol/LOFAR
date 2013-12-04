      SUBROUTINE FRMATD(WWIDTH, EWIDTH)
C
C  THIS SUBROUTINE COMPUTES, FOR THE FORMAT SPECIFICATION, DW.E, THE
C  NUMBER OF DIGITS TO THE RIGHT OF THE DECIMAL POINT, E=EWIDTH, AND
C  THE FIELD WIDTH, W=WWIDTH.
C
C  WWIDTH INCLUDES THE FIVE POSITIONS NEEDED FOR THE SIGN OF THE
C  MANTISSA, THE SIGN OF THE EXPONENT, THE 0, THE DECIMAL POINT AND THE
C  CHARACTER IN THE OUTPUT - +0.XXXXXXXXXD+YYYY
C
C  THE FOLLOWING MACHINE-DEPENDENT VALUES ARE USED -
C
C  I1MACH(10) - THE BASE, B
C  I1MACH(14) - THE NUMBER OF BASE-B DIGITS IN THE MANTISSA
C  I1MACH(15) - THE SMALLEST EXPONENT, EMIN
C  I1MACH(16) - THE LARGEST EXPONENT, EMAX
C
      INTEGER I1MACH, ICEIL, IFLR, EWIDTH, WWIDTH
      INTEGER DEMIN, DEMAX, EXPWID
      REAL BASE
C
      BASE = I1MACH(10)
C
      EWIDTH = ICEIL( ALOG10(BASE)*FLOAT(I1MACH(14)) )
C
      DEMIN =  IFLR( ALOG10(BASE)*FLOAT(I1MACH(15)-1) ) + 1
      DEMAX = ICEIL( ALOG10(BASE)*FLOAT(I1MACH(16)) )
      EXPWID = IFLR( ALOG10(FLOAT(MAX0(IABS(DEMIN),IABS(DEMAX)))) ) + 1
      WWIDTH = EWIDTH + EXPWID + 5
C
      RETURN
      END

      SUBROUTINE E9RINT(MESSG,NW,NERR,SAVE)
C
C  THIS ROUTINE STORES THE CURRENT ERROR MESSAGE OR PRINTS THE OLD ONE,
C  IF ANY, DEPENDING ON WHETHER OR NOT SAVE = .TRUE. .
C
C  CHANGED, BY P.FOX, MAY 18, 1983, FROM THE ORIGINAL VERSION IN ORDER
C  TO GET RID OF THE FORTRAN CARRIAGE CONTROL LINE OVERWRITE
C  CHARACTER +, WHICH HAS ALWAYS CAUSED TROUBLE.
C  FOR THE RECORD, THE PREVIOUS VERSION HAD THE FOLLOWING ARRAY
C  AND CALLS -   (WHERE CCPLUS WAS DECLARED OF TYPE INTEGER)
C
C      DATA CCPLUS  / 1H+ /
C
C      DATA FMT( 1) / 1H( /
C      DATA FMT( 2) / 1HA /
C      DATA FMT( 3) / 1H1 /
C      DATA FMT( 4) / 1H, /
C      DATA FMT( 5) / 1H1 /
C      DATA FMT( 6) / 1H4 /
C      DATA FMT( 7) / 1HX /
C      DATA FMT( 8) / 1H, /
C      DATA FMT( 9) / 1H7 /
C      DATA FMT(10) / 1H2 /
C      DATA FMT(11) / 1HA /
C      DATA FMT(12) / 1HX /
C      DATA FMT(13) / 1HX /
C      DATA FMT(14) / 1H) /
C
C        CALL S88FMT(2,I1MACH(6),FMT(12))
C        WRITE(IWUNIT,FMT) CCPLUS,(MESSGP(I),I=1,NWP)
C
C/6S
C     INTEGER MESSG(NW)
C/7S
      CHARACTER*1 MESSG(NW)
C/
      LOGICAL SAVE
C
C  MESSGP STORES AT LEAST THE FIRST 72 CHARACTERS OF THE PREVIOUS
C  MESSAGE. ITS LENGTH IS MACHINE DEPENDENT AND MUST BE AT LEAST
C
C       1 + 71/(THE NUMBER OF CHARACTERS STORED PER INTEGER WORD).
C
C/6S
C     INTEGER MESSGP(36),FMT(10), FMT10(10)
C     EQUIVALENCE (FMT(1),FMT10(1))
C/7S
      CHARACTER*1 MESSGP(72),FMT(10)
      CHARACTER*10 FMT10
      EQUIVALENCE (FMT(1),FMT10)
C/
C
C  START WITH NO PREVIOUS MESSAGE.
C
C/6S
C     DATA MESSGP(1)/1H1/, NWP/0/, NERRP/0/
C/7S
      DATA MESSGP(1)/'1'/, NWP/0/, NERRP/0/
C/
C
C  SET UP THE FORMAT FOR PRINTING THE ERROR MESSAGE.
C  THE FORMAT IS SIMPLY (A1,14X,72AXX) WHERE XX=I1MACH(6) IS THE
C  NUMBER OF CHARACTERS STORED PER INTEGER WORD.
C
C/6S
C     DATA FMT( 1) / 1H( /
C     DATA FMT( 2) / 1H3 /
C     DATA FMT( 3) / 1HX /
C     DATA FMT( 4) / 1H, /
C     DATA FMT( 5) / 1H7 /
C     DATA FMT( 6) / 1H2 /
C     DATA FMT( 7) / 1HA /
C     DATA FMT( 8) / 1HX /
C     DATA FMT( 9) / 1HX /
C     DATA FMT(10) / 1H) /
C/7S
      DATA FMT( 1) / '(' /
      DATA FMT( 2) / '3' /
      DATA FMT( 3) / 'X' /
      DATA FMT( 4) / ',' /
      DATA FMT( 5) / '7' /
      DATA FMT( 6) / '2' /
      DATA FMT( 7) / 'A' /
      DATA FMT( 8) / 'X' /
      DATA FMT( 9) / 'X' /
      DATA FMT(10) / ')' /
C/
C
      IF (.NOT.SAVE) GO TO 20
C
C  SAVE THE MESSAGE.
C
        NWP=NW
        NERRP=NERR
        DO 10 I=1,NW
 10     MESSGP(I)=MESSG(I)
C
        GO TO 30
C
 20   IF (I8SAVE(1,0,.FALSE.).EQ.0) GO TO 30
C
C  PRINT THE MESSAGE.
C
        IWUNIT=I1MACH(4)
        WRITE(IWUNIT,9000) NERRP
 9000   FORMAT(7H ERROR ,I4,4H IN )
C
C/6S
C       CALL S88FMT(2,I1MACH(6),FMT( 8))
C/7S
        CALL S88FMT(2, 1, FMT(8))
C/
        WRITE(IWUNIT,FMT10) (MESSGP(I),I=1,NWP)
C
 30   RETURN
C
      END

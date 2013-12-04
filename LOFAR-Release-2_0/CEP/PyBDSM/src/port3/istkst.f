      INTEGER FUNCTION ISTKST(NFACT)
C
C  RETURNS CONTROL INFORMATION AS FOLLOWS
C
C  NFACT    ITEM RETURNED
C
C    1         LOUT,  THE NUMBER OF CURRENT ALLOCATIONS
C    2         LNOW,  THE CURRENT ACTIVE LENGTH
C    3         LUSED, THE MAXIMUM USED
C    4         LMAX,  THE MAXIMUM ALLOWED
C
      COMMON /CSTAK/DSTAK
C
      DOUBLE PRECISION DSTAK(500)
      INTEGER ISTAK(1000)
      INTEGER ISTATS(4)
      LOGICAL INIT
C
      EQUIVALENCE (DSTAK(1),ISTAK(1))
      EQUIVALENCE (ISTAK(1),ISTATS(1))
C
      DATA INIT/.TRUE./
C
      IF (INIT) CALL I0TK00(INIT,500,4)
C
C/6S
C     IF (NFACT.LE.0.OR.NFACT.GE.5) CALL SETERR
C    1   (33HISTKST - NFACT.LE.0.OR.NFACT.GE.5,33,1,2)
C/7S
      IF (NFACT.LE.0.OR.NFACT.GE.5) CALL SETERR
     1   ('ISTKST - NFACT.LE.0.OR.NFACT.GE.5',33,1,2)
C/
C
      ISTKST = ISTATS(NFACT)
C
      RETURN
C
      END

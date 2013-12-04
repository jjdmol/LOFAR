      SUBROUTINE XTRAP(TM,M,NVAR,NG,KMAX,XPOLY,T,ERROR,EBEST)
C
C  ASSUME AN EXPANSION FOR THE VECTOR VALUED FUNCTION T(H) OF THE FORM
C
C            T(H) = T(0) + SUM(J=1,2,3,...)(A(J)*H**(J*GAMMA))
C
C  WHERE THE A(J) ARE CONSTANT VECTORS AND GAMMA IS A POSITIVE CONSTANT.
C
C  GIVEN T(H(M)), WHERE H(M)=H0/N(M), M=1,2,3,..., THIS ROUTINE USES
C  POLYNOMIAL (XPOLY), OR RATIONAL (.NOT.XPOLY), EXTRAPOLATION TO
C  SEQUENTIALLY APPROXIMATE T(0).
C
C  INPUT
C
C    TM     - TM = T(H(M)) FOR THIS CALL.
C    M      - H(M) WAS USED TO OBTAIN TM.
C    NVAR   - THE LENGTH OF THE VECTOR TM.
C    NG     - THE REAL VALUES
C
C                 NG(I) = N(I)**GAMMA
C
C             FOR I=1,...,M. NG MUST BE A MONOTONE INCREASING ARRAY.
C    KMAX   - THE MAXIMUM NUMBER OF COLUMNS TO BE USED IN THE
C             EXTRAPOLATION PROCESS.
C    XPOLY  - IF XPOLY=.TRUE., THEN _USE_ POLYNOMIAL EXTRAPOLATION.
C             IF XPOLY=.FALSE., THEN _USE_ RATIONAL EXTRAPOLATION.
C    T      - THE BOTTOM EDGE OF THE EXTRAPOLATION LOZENGE.
C             T(I,J) SHOULD CONTAIN THE J-TH EXTRAPOLATE OF THE I-TH
C             COMPONENT OF T(H) BASED ON THE SEQUENCE H(1),...,H(M-1),
C             FOR I=1,...,NVAR AND J=1,...,MIN(M-1,KMAX).
C
C             WHEN M=1, T MAY CONTAIN ANYTHING.
C
C             FOR M.GT.1, NOTE THAT THE OUTPUT VALUE OF T AT THE
C             (M-1)-ST CALL IS THE INPUT FOR THE M-TH CALL.
C             THUS, THE USER NEED NEVER PUT ANYTHING INTO T,
C             BUT HE CAN NOT ALTER ANY ELEMENT OF T BETWEEN
C             CALLS TO XTRAP.
C
C  OUTPUT
C
C    TM     - TM(I)=THE MOST ACCURATE APPROXIMATION IN THE LOZENGE
C             FOR THE I-TH VARIABLE, I=1,...,NVAR.
C    T      - T(I,J) CONTAINS THE J-TH EXTRAPOLATE OF THE I-TH
C             COMPONENT OF T(H) BASED ON THE SEQUENCE H(1),...,H(M),
C             FOR I=1,...,NVAR AND J=1,...,MIN(M,KMAX).
C    ERROR  - ERROR(I,J) GIVES THE SIGNED BULIRSCH-STOER ESTIMATE OF THE
C             ERROR IN THE J-TH EXTRAPOLATE OF THE I-TH COMPONENT OF
C             T(H) BASED ON THE SEQUENCE H(1),...,H(M-1),
C             FOR I=1,...,NVAR AND J=1,...,MIN(M-1,KMAX).
C             IF ERROR=EBEST AS ARRAYS, THEN THE ABOVE ELEMENTS
C             ARE NOT STORED. RATHER, EBEST=ERROR IS LOADED AS DESCRIBED
C             BELOW.
C    EBEST  - EBEST(I)=THE ABSOLUTE VALUE OF THE ERROR IN TM(I),
C             I=1,...,NVAR. THIS ARRAY IS FULL OF GARBAGE WHEN M=1.
C
C  SCRATCH SPACE ALLOCATED - MIN(M-1,KMAX) REAL WORDS +
C
C                            MIN(M-1,KMAX) INTEGER WORDS.
C
C  ERROR STATES -
C
C    1 - M.LT.1.
C    2 - NVAR.LT.1.
C    3 - NG(1).LT.1.
C    4 - KMAX.LT.1.
C    5 - NG IS NOT MONOTONE INCREASING.
C
      REAL TM(NVAR),NG(M),T(NVAR,1)
C     REAL T(NVAR,MIN(M,KMAX))
      REAL ERROR(NVAR,1),EBEST(NVAR)
C     REAL ERROR(NVAR,MIN(M-1,KMAX))
      LOGICAL XPOLY
C
      LOGICAL ESAVE
C
      COMMON /CSTAK/DS
      DOUBLE PRECISION DS(500)
      REAL WS(1)
      REAL RS(1000)
      EQUIVALENCE (DS(1),WS(1)),(DS(1),RS(1))
C
C ... CHECK THE INPUT.
C
C/6S
C     IF (M.LT.1) CALL SETERR(15H XTRAP - M.LT.1,15,1,2)
C     IF (NVAR.LT.1) CALL SETERR(18H XTRAP - NVAR.LT.1,18,2,2)
C     IF (NG(1).LT.1.0E0) CALL SETERR(19H XTRAP - NG(1).LT.1,19,3,2)
C     IF (KMAX.LT.1) CALL SETERR(18H XTRAP - KMAX.LT.1,18,4,2)
C/7S
      IF (M.LT.1) CALL SETERR(' XTRAP - M.LT.1',15,1,2)
      IF (NVAR.LT.1) CALL SETERR(' XTRAP - NVAR.LT.1',18,2,2)
      IF (NG(1).LT.1.0E0) CALL SETERR(' XTRAP - NG(1).LT.1',19,3,2)
      IF (KMAX.LT.1) CALL SETERR(' XTRAP - KMAX.LT.1',18,4,2)
C/
C
      IF (M.EQ.1) GO TO 20
C
      DO 10 I=2,M
C/6S
C        IF (NG(I-1).GE.NG(I)) CALL SETERR
C    1      (38H XTRAP - NG IS NOT MONOTONE INCREASING,38,5,2)
C/7S
         IF (NG(I-1).GE.NG(I)) CALL SETERR
     1      (' XTRAP - NG IS NOT MONOTONE INCREASING',38,5,2)
C/
 10      CONTINUE
C
C ... SEE IF ERROR=EBEST AS ARRAYS. IF (ESAVE), THEN LOAD ERROR.
C
 20   ERROR(1,1)=1.0E0
      EBEST(1)=2.0E0
      ESAVE=ERROR(1,1).NE.EBEST(1)
C
C ... ALLOCATE SCRATCH SPACE.
C
      IRHG=1
      IEMAG=1
      IF (M.GT.1) IRHG=ISTKGT(MIN0(M-1,KMAX),3)
      IF (M.GT.1) IEMAG=ISTKGT(MIN0(M-1,KMAX),3)
C
      CALL A0XTRP(TM,M,NVAR,NG,KMAX,XPOLY,T,ERROR,EBEST,WS(IRHG),
     1            RS(IEMAG),ESAVE)
C
      IF (M.GT.1) CALL ISTKRL(2)
C
      RETURN
C
      END

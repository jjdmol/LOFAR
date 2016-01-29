C$TEST DPT6
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE DPT6
C***********************************************************************
C
C  EXAMPLE OF USE OF PORT PROGRAM DPOST
C
C***********************************************************************
      COMMON /CSTAK/ DS
      DOUBLE PRECISION DS(4000)
      COMMON /TIME/ T
      DOUBLE PRECISION T
      COMMON /PARAM/ VC, X
      DOUBLE PRECISION VC(4), X(3)
      EXTERNAL DEE, HANDLE, UOFX, BC, AF
      INTEGER NDX, IDLUMB, ISTKGT, K, IU, IS(1000)
      INTEGER NU, NV, IMMMD, IMESH, NMESH
      REAL ERRPAR(2), RS(1000)
      LOGICAL LS(1000)
      COMPLEX CS(500)
      DOUBLE PRECISION TSTART, V(4), DT, XB(3), WS(500), TSTOP
      EQUIVALENCE (DS(1), CS(1), WS(1), RS(1), IS(1), LS(1))
C TO TEST DPOST ON THE HYPERBOLIC PROBLEM
C      U SUB T = - U SUB X + G      ON (-PI,+PI) * (0,PI)
C WITH A MOVING SHOCK X(T) CHARACTERIZED BY
C    U(X(T)+,T) = 0 AND
C    U(X(T)+,T) - U(X(T)-,T) = X'(T)
C WHERE G IS CHOSEN SO THAT THE SOLUTION IS
C               SIN(X+T)  FOR X < X(T)
C      U(X,T) = 
C               COS(X+T)  FOR X > X(T)
C WITH X(T) = PI/2 -T .
C V(1,2,3) GIVES THE MOVING MESH AND V(4) IS THE HEIGHT OF THE JUMP.
C THE PORT LIBRARY STACK AND ITS ALIASES.
C INITIALIZE THE PORT LIBRARY STACK LENGTH.
      CALL ISTKIN(4000, 4)
      CALL ENTER(1)
      NU = 1
      NV = 4
      ERRPAR(1) = 0
C ABSOLUTE ERROR.
      ERRPAR(2) = 1E-2
      TSTART = 0
      TSTOP = 3.14
      DT = 0.4
      K = 4
C NDX UNIFORM MESH POINTS ON EACH INTERVAL OF XB.
      NDX = 6
      XB(1) = 0
      XB(2) = 1
      XB(3) = 2
C GET MESH ON PORT STACK.
      IMESH = IDLUMB(XB, 3, NDX, K, NMESH)
C MAKE 1 OF MULTIPLICITY K-1.
      IMESH = IMMMD(IMESH, NMESH, 1D0, K-1)
      X(1) = -3.14
      X(2) = 3.14/2.
      X(3) = 3.14
C INITIAL VALUES FOR V.
      CALL DLPLMG(3, X, VC)
C GET U ON THE PORT STACK.
      IU = ISTKGT(NMESH-K, 4)
C UOFX NEEDS TIME.
      T = TSTART
C THE INITIAL HEIGHT OF THE JUMP.
      VC(4) = 1
C UOFX NEEDS V FOR MAPPING.
      CALL MOVEFD(NV, VC, V)
C INITIAL CONDITIONS FOR U.
      CALL DL2SFF(UOFX, K, WS(IMESH), NMESH, WS(IU))
C OUTPUT ICS.
      CALL HANDLE(T-1D0, WS(IU), V, T, WS(IU), V, NU, NMESH-K, NV, K, 
     1   WS(IMESH), NMESH, DT, TSTOP)
      CALL DPOST(WS(IU), NU, K, WS(IMESH), NMESH, V, NV, TSTART, TSTOP
     1   , DT, AF, BC, DEE, ERRPAR, HANDLE)
      CALL LEAVE
      CALL WRAPUP
      STOP 
      END
      SUBROUTINE AF(T, XI, NX, U, UX, UT, UTX, NU, V, VT, NV, A, 
     1   AU, AUX, AUT, AUTX, AV, AVT, F, FU, FUX, FUT, FUTX, FV, FVT)
      INTEGER NU, NV, NX
      DOUBLE PRECISION T, XI(NX), U(NX, NU), UX(NX, NU), UT(NX, NU), 
     1   UTX(NX, NU)
      DOUBLE PRECISION V(NV), VT(NV), A(NX, NU), AU(NX, NU, NU), AUX(NX,
     1   NU, NU), AUT(NX, NU, NU)
      DOUBLE PRECISION AUTX(NX, NU, NU), AV(NX, NU, NV), AVT(NX, NU, NV)
     1   , F(NX, NU), FU(NX, NU, NU), FUX(NX, NU, NU)
      DOUBLE PRECISION FUT(NX, NU, NU), FUTX(NX, NU, NU), FV(NX, NU, NV)
     1   , FVT(NX, NU, NV)
      COMMON /DPOSTF/ FAILED
      LOGICAL FAILED
      INTEGER I
      DOUBLE PRECISION XXI(99), XTV(99), XVV(99), X(99), DCOS, DSIN
      DOUBLE PRECISION XXIV(99), AX(99), FX(99), XT(99), XV(99)
      LOGICAL TEMP
      TEMP = V(2) .LE. V(1)
      IF (.NOT. TEMP) TEMP = V(2) .GE. V(3)
      IF (.NOT. TEMP) GOTO 1
         FAILED = .TRUE.
         RETURN
C MAP XI INTO X.
   1  CALL DLPLM(XI, NX, V, 3, X, XXI, XXIV, XV, XVV, XT, XTV)
C MAP U INTO X SYSTEM.
      CALL DPOSTU(XI, X, XT, XXI, XV, VT, NX, 3, UX, UT, NU, AX, FX)
      DO  4 I = 1, NX
         A(I, 1) = -U(I, 1)
         AU(I, 1, 1) = -1
         F(I, 1) = UT(I, 1)
         FUT(I, 1, 1) = 1
         IF (XI(I) .GT. 1D0) GOTO 2
            F(I, 1) = F(I, 1)-2D0*DCOS(X(I)+T)
            FX(I) = 2D0*DSIN(X(I)+T)
            GOTO  3
   2        F(I, 1) = F(I, 1)-VT(4)
            FVT(I, 1, 4) = -1
            F(I, 1) = F(I, 1)+2D0*DSIN(X(I)+T)
            FX(I) = 2D0*DCOS(X(I)+T)
   3     CONTINUE
   4     CONTINUE
C MAP A AND F INTO XI SYSTEM.
      CALL DPOSTI(XI, X, XT, XXI, XV, XTV, XXIV, XVV, NX, UX, UT, NU, V,
     1   VT, NV, 1, 3, A, AX, AU, AUX, AUT, AUTX, AV, AVT, F, FX, FU, 
     2   FUX, FUT, FUTX, FV, FVT)
      RETURN
      END
      SUBROUTINE BC(T, L, R, U, UX, UT, UTX, NU, V, VT, NV, B, BU,
     1   BUX, BUT, BUTX, BV, BVT)
      INTEGER NU, NV
      DOUBLE PRECISION T, L, R, U(NU, 2), UX(NU, 2), UT(NU, 2)
      DOUBLE PRECISION UTX(NU, 2), V(NV), VT(NV), B(NU, 2), BU(NU, NU, 2
     1   ), BUX(NU, NU, 2)
      DOUBLE PRECISION BUT(NU, NU, 2), BUTX(NU, NU, 2), BV(NU, NV, 2), 
     1   BVT(NU, NV, 2)
      DOUBLE PRECISION DSIN
      B(1, 1) = U(1, 1)-DSIN(T-3.14)
C U(-PI,T) = SIN(-PI+T).
      BU(1, 1, 1) = 1
      RETURN
      END
      SUBROUTINE DEE(T, K, X, NX, U, UT, NU, NXMK, V, VT, NV, D, 
     1   DU, DUT, DV, DVT)
      INTEGER NXMK, NU, NV, NX
      INTEGER K
      DOUBLE PRECISION T, X(NX), U(NXMK, NU), UT(NXMK, NU), V(NV), VT(
     1   NV)
      DOUBLE PRECISION D(NV), DU(NV, NXMK, NU), DUT(NV, NXMK, NU), DV(
     1   NV, NV), DVT(NV, NV)
      INTEGER INTRVD, I, ILEFT
      DOUBLE PRECISION BX(10), XX(1), D1MACH
      INTEGER TEMP
      D(1) = V(1)+3.14
C X(0,V) = -PI.
      DV(1, 1) = 1
C XX(1) = 1 + A ROUNDING ERROR.
      XX(1) = D1MACH(4)+1D0
      ILEFT = INTRVD(NX, X, XX(1))
C GET THE B-SPLINE BASIS AT XX.
      CALL DBSPLN(K, X, NX, XX, 1, ILEFT, BX)
      D(2) = -V(4)
C U(X(T)+,T) - JUMP = 0.
      DV(2, 4) = -1
      DO  1 I = 1, K
         TEMP = ILEFT+I-K
         D(2) = D(2)+U(TEMP, 1)*BX(I)
         TEMP = ILEFT+I-K
         DU(2, TEMP, 1) = BX(I)
   1     CONTINUE
      D(3) = V(3)-3.14
C X(2,V) = +PI.
      DV(3, 3) = 1
C JUMP + D( X(1,V(T)) )/DT = 0.
      D(4) = VT(2)+V(4)
      DVT(4, 2) = 1
      DV(4, 4) = 1
      RETURN
      END
      SUBROUTINE HANDLE(T0, U0, V0, T, U, V, NU, NXMK, NV, K, X, 
     1   NX, DT, TSTOP)
      INTEGER NXMK, NU, NV, NX
      INTEGER K
      DOUBLE PRECISION T0, U0(NXMK, NU), V0(NV), T, U(NXMK, NU), V(NV)
      DOUBLE PRECISION X(NX), DT, TSTOP
      COMMON /PARAM/ VC, XX
      DOUBLE PRECISION VC(4), XX(3)
      COMMON /TIME/ TT
      DOUBLE PRECISION TT
      EXTERNAL UOFX
      INTEGER I1MACH
      DOUBLE PRECISION DEESFF, EU, EV(2)
      INTEGER TEMP
C OUTPUT AND CHECKING ROUTINE.
      IF (T0 .NE. T) GOTO 2
         TEMP = I1MACH(2)
         WRITE (TEMP,  1) T, DT
   1     FORMAT (16H RESTART FOR T =, 1PE10.2, 7H   DT =, 1PE10.2)
         RETURN
   2  TT = T
C UOFX NEEDS V FOR MAPPING.
      CALL MOVEFD(NV, V, VC)
      EU = DEESFF(K, X, NX, U, UOFX)
C ERROR IN POSITION OF SHOCK.
      EV(1) = V(2)-(3.14/2.-T)
C ERROR IN HEIGHT OF SHOCK.
      EV(2) = V(4)-1D0
      TEMP = I1MACH(2)
      WRITE (TEMP,  3) T, EU, EV
   3  FORMAT (14H ERROR IN U(X,, 1PE10.2, 4H ) =, 1PE10.2, 6H   V =, 2(
     1   1PE10.2))
      RETURN
      END
      SUBROUTINE UOFX(XI, NX, U, W)
      INTEGER NX
      DOUBLE PRECISION XI(NX), U(NX), W(NX)
      COMMON /CSTAK/ DS
      DOUBLE PRECISION DS(500)
      COMMON /PARAM/ VC, X
      DOUBLE PRECISION VC(4), X(3)
      COMMON /TIME/ T
      DOUBLE PRECISION T
      INTEGER IXV, IXX, ISTKGT, I, IS(1000)
      REAL RS(1000)
      LOGICAL LS(1000)
      COMPLEX CS(500)
      DOUBLE PRECISION EWE, WS(500)
      INTEGER TEMP
      EQUIVALENCE (DS(1), CS(1), WS(1), RS(1), IS(1), LS(1))
C THE PORT LIBRARY STACK AND ITS ALIASES.
      CALL ENTER(1)
      IXX = ISTKGT(NX, 4)
C SPACE FOR X AND XV.
      IXV = ISTKGT(3*NX, 4)
C MAP INTO USER SYSTEM.
      CALL DLPLMX(XI, NX, VC, 3, WS(IXX), WS(IXV))
      DO  1 I = 1, NX
         TEMP = IXX+I
         U(I) = EWE(T, WS(TEMP-1), VC(2))
         IF (XI(I) .GT. 1D0) U(I) = U(I)+1D0
   1     CONTINUE
      CALL LEAVE
      RETURN
      END
      DOUBLE PRECISION FUNCTION EWE(T, X, XBREAK)
      DOUBLE PRECISION T, X, XBREAK
      DOUBLE PRECISION DCOS, DSIN
      IF (X .GE. XBREAK) GOTO 1
         EWE = DSIN(X+T)
         RETURN
   1     IF (X .LE. XBREAK) GOTO 2
            EWE = DCOS(X+T)
            RETURN
   2        CALL SETERR(17HEWE - X == XBREAK, 17, 1, 2)
   3  CONTINUE
   4  STOP 
      END

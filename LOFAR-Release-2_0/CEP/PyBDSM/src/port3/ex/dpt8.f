C$TEST DPT8
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE DPT8
C***********************************************************************
C
C  EXAMPLE OF USE OF PORT PROGRAM DPOST
C
C***********************************************************************
      COMMON /CSTAK/ DS
      DOUBLE PRECISION DS(5000)
      COMMON /TIME/ T
      DOUBLE PRECISION T
      COMMON /KMESH/ K, NMESH
      INTEGER K, NMESH
      COMMON /CMESH/ MESH
      DOUBLE PRECISION MESH(100)
      EXTERNAL DEE, HANDLE, UOFX, BC, AF
      INTEGER NDX, I, IS(1000), NU, NV
      REAL ERRPAR(2), RS(1000)
      LOGICAL LS(1000)
      COMPLEX CS(500)
      DOUBLE PRECISION U(100), V(100), DT, WS(500), TSTOP
      INTEGER TEMP
      EQUIVALENCE (DS(1), CS(1), WS(1), RS(1), IS(1), LS(1))
C TO TEST DPOST ON THE INTEGRO-PDE
C      U SUB T = 2 * U SUB XX - INT(0,1) EXP(X-Y)*U(Y) DY      ON (0,1)
C SUBJECT TO GIVEN DIRICHLET BCS, CHOSEN SO THAT THE SOLUTION IS
C      U(X,T) = EXP(T+X).
C THE PORT LIBRARY STACK AND ITS ALIASES.
C INITIALIZE THE PORT LIBRARY STACK LENGTH.
      CALL ISTKIN(5000, 4)
      NU = 1
      ERRPAR(1) = 0
C ABSOLUTE ERROR.
      ERRPAR(2) = 1E-2
      TSTOP = 1
      DT = 1D-2
      K = 4
C NDX UNIFORM MESH POINTS ON (0,1).
      NDX = 7
      CALL DUMB(0D0, 1D0, NDX, K, MESH, NMESH)
      NV = NMESH-K
C UOFX NEEDS T.
      T = 0
C ICS FOR U.
      CALL DL2SFF(UOFX, K, MESH, NMESH, U)
      TEMP = NMESH-K
      DO  1 I = 1, TEMP
         V(I) = U(I)
   1     CONTINUE
C ICS FOR V.
      CALL DPOST(U, NU, K, MESH, NMESH, V, NV, 0D0, TSTOP, DT, AF, BC, 
     1   DEE, ERRPAR, HANDLE)
      CALL WRAPUP
      STOP 
      END
      SUBROUTINE AF(T, X, NX, U, UX, UT, UTX, NU, V, VT, NV, A, 
     1   AU, AUX, AUT, AUTX, AV, AVT, F, FU, FUX, FUT, FUTX, FV, FVT)
      INTEGER NU, NV, NX
      DOUBLE PRECISION T, X(NX), U(NX, NU), UX(NX, NU), UT(NX, NU), UTX(
     1   NX, NU)
      DOUBLE PRECISION V(NV), VT(NV), A(NX, NU), AU(NX, NU, NU), AUX(NX,
     1   NU, NU), AUT(NX, NU, NU)
      DOUBLE PRECISION AUTX(NX, NU, NU), AV(NX, NU, NV), AVT(NX, NU, NV)
     1   , F(NX, NU), FU(NX, NU, NU), FUX(NX, NU, NU)
      DOUBLE PRECISION FUT(NX, NU, NU), FUTX(NX, NU, NU), FV(NX, NU, NV)
     1   , FVT(NX, NU, NV)
      COMMON /KMESH/ K, NMESH
      INTEGER K, NMESH
      COMMON /CMESH/ MESH
      DOUBLE PRECISION MESH(100)
      INTEGER I
      DO  1 I = 1, NX
         A(I, 1) = 2D0*UX(I, 1)
         AUX(I, 1, 1) = 2
         F(I, 1) = UT(I, 1)
         FUT(I, 1, 1) = 1
   1     CONTINUE
C GET THE INTEGRAL.
      CALL INTGRL(K, MESH, NMESH, V, X, NX, F, FV)
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
      DOUBLE PRECISION DEXP
      B(1, 1) = U(1, 1)-DEXP(T)
      B(1, 2) = U(1, 2)-DEXP(T+1D0)
      BU(1, 1, 1) = 1
      BU(1, 1, 2) = 1
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
      INTEGER I
      DO  1 I = 1, NXMK
         D(I) = U(I, 1)-V(I)
         DU(I, I, 1) = 1
         DV(I, I) = -1
   1     CONTINUE
      RETURN
      END
      SUBROUTINE HANDLE(T0, U0, V0, T, U, V, NU, NXMK, NV, K, X, 
     1   NX, DT, TSTOP)
      INTEGER NXMK, NU, NV, NX
      INTEGER K
      DOUBLE PRECISION T0, U0(NXMK, NU), V0(NV), T, U(NXMK, NU), V(NV)
      DOUBLE PRECISION X(NX), DT, TSTOP
      COMMON /TIME/ TT
      DOUBLE PRECISION TT
      EXTERNAL UOFX
      INTEGER I1MACH
      DOUBLE PRECISION DEESFF, EU
      INTEGER TEMP
C OUTPUT AND CHECKING ROUTINE.
      IF (T0 .NE. T) GOTO 2
         TEMP = I1MACH(2)
         WRITE (TEMP,  1) T0, DT
   1     FORMAT (16H RESTART FOR T =, 1PE10.2, 7H   DT =, 1PE10.2)
         RETURN
   2  TT = T
      EU = DEESFF(K, X, NX, U, UOFX)
      TEMP = I1MACH(2)
      WRITE (TEMP,  3) T, EU
   3  FORMAT (14H ERROR IN U(X,, 1PE10.2, 4H ) =, 1PE10.2)
      RETURN
      END
      SUBROUTINE UOFX(X, NX, U, W)
      INTEGER NX
      DOUBLE PRECISION X(NX), U(NX), W(NX)
      COMMON /TIME/ T
      DOUBLE PRECISION T
      INTEGER I
      DOUBLE PRECISION DEXP
      DO  1 I = 1, NX
         U(I) = DEXP(T+X(I))
   1     CONTINUE
      RETURN
      END
      SUBROUTINE INTGRL(K, MESH, NMESH, V, X, NX, F, FV)
      INTEGER NX, NMESH
      INTEGER K
      DOUBLE PRECISION MESH(NMESH), V(1), X(NX), F(NX), FV(NX, 1)
      INTEGER MGQ, I, J, L, IX
      LOGICAL FIRST
      DOUBLE PRECISION EWE, KER, WGQ(3), XGQ(3), B(3, 4, 200), KERU
      DOUBLE PRECISION XX(3)
      INTEGER TEMP, TEMP1
      DATA FIRST/.TRUE./
C TO COMPUTE
C    F = INTEGRAL FROM MESH(1) TO MESH(NMESH)
C       KERNEL(X,Y,SUM(I=1,...,NMESH-K) V(I)*B(I,Y)) DY
C  AND
C    FV = D(F)/D(V).
C ASSUME THAT CALL KERNEL(X,Y,U,KER,KERU) RETURNS
C     KER = KERNEL(X,Y,U) AND
C     KERU = PARTIAL KERNEL / PARTIAL U.
C V(NMESH-K),FV(NX,NMESH-K)
C THE FOLLOWING DECLARATION IS SPECIFIC TO K = 4 SPLINES.
      IF (NMESH-K .GT. 200) CALL SETERR(27HINTGRL - NMESH-K .GT. NXMAX
     1   , 27, 1, 2)
C NEED MORE LOCAL SPACE.
      IF (K .NE. 4) CALL SETERR(17HINTGRL - K .NE. 4, 17, 2, 2)
C USE K-1 POINT GAUSSIAN-QUADRATURE RULE ON EACH INTERVAL.
      MGQ = K-1
      IF (FIRST) CALL DGQM11(MGQ, XGQ, WGQ)
C ONLY GET GQ RULE ONCE, ITS EXPENSIVE.
C THE GAUSSIAN QUADRATURE RULE.
C DO INTEGRAL INTERVAL BY INTERVAL.
      TEMP = NMESH-K
      DO  6 I = K, TEMP
C G.Q. POINTS ON (MESH(I), MESH(I+1)).
         DO  1 J = 1, MGQ
            XX(J) = 0.5*(MESH(I+1)+MESH(I))+0.5*(MESH(I+1)-MESH(I))*XGQ(
     1         J)
   1        CONTINUE
         IF (FIRST) CALL DBSPLN(K, MESH, NMESH, XX, MGQ, I, B(1, 1, I))
C ONLY GET B-SPLINE BASIS ONCE, ITS EXPENSIVE.
         DO  5 J = 1, MGQ
C GET SUM() V()*B()(XX).
            EWE = 0
            DO  2 L = 1, K
               TEMP1 = I+L-K
               EWE = EWE+V(TEMP1)*B(J, L, I)
   2           CONTINUE
            DO  4 IX = 1, NX
C GET KERNEL AND PARTIAL.
               CALL KERNEL(X(IX), XX(J), EWE, KER, KERU)
               F(IX) = F(IX)+0.5*KER*(MESH(I+1)-MESH(I))*WGQ(J)
               DO  3 L = 1, K
                  TEMP1 = I+L-K
                  FV(IX, TEMP1) = FV(IX, TEMP1)+0.5*B(J, L, I)*KERU*(
     1               MESH(I+1)-MESH(I))*WGQ(J)
   3              CONTINUE
   4           CONTINUE
   5        CONTINUE
   6     CONTINUE
      FIRST = .FALSE.
      RETURN
      END
      SUBROUTINE KERNEL(X, Y, U, KER, KERU)
      DOUBLE PRECISION X, Y, U, KER, KERU
      DOUBLE PRECISION DEXP
C TO EVALUATE THE KERNEL EXP(X-Y)*U(Y) AND ITS PARTIAL WRT. U.
      KERU = DEXP(X-Y)
      KER = KERU*U
      RETURN
      END

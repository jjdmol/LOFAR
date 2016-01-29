C$TEST NLSR
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE NLSR
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAMS NSG AND NSF
C
C***********************************************************************
C *** NSG AND NSF EXAMPLE PROGRAM ***
C
C *** FIT N = 33 DATA POINTS (T,Y) TO THE CURVE
C *** X(1) + X(2)*DEXP(T*X(4)) + X(3)*DEXP(T*X(5))
C
C *** THE FOLLOWING CODE IS FOR CALLING NSG.  DIFFERENCES FOR
C *** CALLING NSF ARE EXPLAINED IN COMMENTS.
C
      INTEGER I, J, INC(4,2), IV(124), LIV, LTY, LV, UI(1)
      DOUBLE PRECISION C(3), T(33), Y(33), V(612), X(5)
      EXTERNAL DUMMY, OSB1A, OSB1B
      DATA LIV/124/, LTY/50/, LV/612/
C
C *** FOR NSF, OMIT OSB1B FROM THE EXTERNAL STATEMENT.
C
C
C *** TO MAKE THIS EXAMPLE SELF-CONTAINED, WE USE A DATA STATEMENT
C *** AND DO LOOP TO SUPPLY (T(I),Y(I)) PAIRS.
C
C *** Y VALUES...
C
      DATA Y(1) /8.44D-1/, Y(2) /9.08D-1/, Y(3)/9.32D-1/,
     1     Y(4) /9.36D-1/, Y(5) /9.25D-1/, Y(6)/9.08D-1/,
     2     Y(7) /8.81D-1/, Y(8) /8.50D-1/, Y(9)/8.18D-1/,
     3     Y(10)/7.84D-1/, Y(11)/7.51D-1/, Y(12)/7.18D-1/,
     4     Y(13)/6.85D-1/, Y(14)/6.58D-1/, Y(15)/6.28D-1/,
     5     Y(16)/6.03D-1/, Y(17)/5.80D-1/, Y(18)/5.58D-1/,
     6     Y(19)/5.38D-1/, Y(20)/5.22D-1/, Y(21)/5.06D-1/,
     7     Y(22)/4.90D-1/, Y(23)/4.78D-1/, Y(24)/4.67D-1/,
     8     Y(25)/4.57D-1/, Y(26)/4.48D-1/, Y(27)/4.38D-1/,
     9     Y(28)/4.31D-1/, Y(29)/4.24D-1/, Y(30)/4.20D-1/,
     A     Y(31)/4.14D-1/, Y(32)/4.11D-1/, Y(33)/4.06D-1/
C
C ***  T VALUES...
C
      DO 10 I = 1, 33
         T(I) = -10.D+0 *FLOAT(I-1)
 10      CONTINUE
C
C     ***  SET UP INC  ***
C
      DO 30 J = 1, 2
         DO 20 I = 1, 4
 20           INC(I,J) = 0
 30      CONTINUE
      INC(2,1) = 1
      INC(3,2) = 1
C
C *** SPECIFY ALL DEFAULT IV AND V INPUT COMPONENTS ***
C
      IV(1) = 0
C
C ... TO TURN OFF THE DEFAULT COMPUTATION AND PRINTING OF THE
C ... REGRESSION DIAGNOSTIC VECTOR, WE WOULD REPLACE THE ABOVE
C ... ASSIGNMENT OF 0 TO IV(1) WITH THE FOLLOWING THREE LINES...
C
C     CALL IVSET(1, IV, LIV, LV, V)
C     IV(57) = 1
C     IV(14) = 1
C
C ... THAT IS, WE SET IV(RDREQ) AND IV(COVPRT) TO 1, THUS REQUESTING
C ... COMPUTATION AND PRINTING OF JUST A COVARIANCE MATRIX.
C
C
C *** SUPPLY INITIAL GUESS...
C
      X(1) = 1.D-2
      X(2) = 2.D-2
C
C *** SOLVE THE PROBLEM -- NSG WILL PRINT THE SOLUTION FOR US...
C
      CALL DNSG(33, 2, 3, X, C, Y, OSB1A, OSB1B, INC, 4,
     1           IV, LIV, LV, V, UI, T, DUMMY)
C
C *** FOR NSF, THE CORRESPONDING CALL WOULD BE...
C
C     CALL NSF(33, 2, 3, X, C, Y, OSB1A, INC, 4,
C    1           IV, LIV, LV, V, UI, T, DUMMY)
C
C
C *** NOTE -- ON MOST SYSTEMS, WE COULD SIMPLY PASS OSB1A (OR OSB1B)
C *** AS THE UF PARAMETER, SINCE OSB1A AND OSB1B IGNORE THIS
C *** PARAMETER.  BUT THERE EXIST SYSTEMS (E.G. UNIVAC) THAT WOULD
C *** GIVE A RUN-TIME ERROR IF WE DID THIS.  HENCE WE PASS THE
C *** IMMEDIATELY FOLLOWING DUMMY SUBROUTINE AS UF.
C
      STOP
      END
      SUBROUTINE DUMMY
      RETURN
      END
      SUBROUTINE OSB1A(N, P, L, X, NF, A, UI, T, UF)
C
C *** THIS ROUTINE COMPUTES THE A MATRIX, A = A(X),
C *** FOR TEST PROBLEM OSBORNE1.
C
      INTEGER L, N, NF, P, UI(1)
      DOUBLE PRECISION A(N,1), T(N), X(P)
      EXTERNAL UF
C
      INTEGER I
      DOUBLE PRECISION ONE, TI
      DATA ONE/1.D+0/
C
      DO 10 I = 1, N
         TI = T(I)
         A(I,1) = ONE
         A(I,2) = DEXP(TI*X(1))
         A(I,3) = DEXP(TI*X(2))
 10      CONTINUE
      RETURN
      END
      SUBROUTINE OSB1B(N, P, L, X, NF, B, UI, T, UF)
C
C *** THIS ROUTINE COMPUTES THE JACOBIAN TENSOR, B = B(X),
C *** FOR TEST PROBLEM OSBORNE1.
C
      INTEGER L, N, NF, P, UI(1)
      DOUBLE PRECISION B(N,2), T(N), X(P)
      EXTERNAL UF
C
      INTEGER I
      DOUBLE PRECISION TI
C
      DO 10 I = 1, N
         TI = T(I)
         B(I,1) = TI * DEXP(TI*X(1))
         B(I,2) = TI * DEXP(TI*X(2))
 10      CONTINUE
      RETURN
      END

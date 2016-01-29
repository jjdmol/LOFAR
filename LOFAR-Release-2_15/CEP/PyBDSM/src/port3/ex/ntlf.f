C$TEST NTLF
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE NTLF
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAM SMNG
C
C***********************************************************************
        INTEGER N
        EXTERNAL ROSN,ROSG
        REAL X(2)
        N=2
C INITIALIZE X
        X(1)=-1.2
        X(2)=1.0
C
C SOLVE THE PROBLEM
C
        CALL SMNG(N, X, ROSN, ROSG, 100, 1.E-4)
C       PRINT RESULTS ON STANDARD OUTPUT UNIT
        IWRITE=I1MACH(2)
        WRITE(IWRITE,10)(X(I),I=1,N)
 10     FORMAT(10H SOLUTION-,5E15.5)
        STOP
        END
        SUBROUTINE ROSN(N,X,NF,F)
C THIS SUBROUTINE COMPUTES THE  FUNCTION
        INTEGER N, NF
        REAL X(N), F
        F=100.0*(X(2)-X(1)*X(1))**2 + (1.0 - X(1))**2
        RETURN
        END
        SUBROUTINE ROSG(N,X,NF,G)
C THIS SUBROUTINE COMPUTES THE GRADIENT
        INTEGER N,NF
        REAL X(N), G(N)
        G(1)=200.0*(X(2)-X(1)*X(1))*(-2.0)*X(1) - 2.0*(1-X(1))
        G(2)=200.0*(X(2)-X(1)*X(1))
        RETURN
        END

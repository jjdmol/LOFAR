      SUBROUTINE MOVEFI(N,A,B)
C
C     MOVEFI MOVES N INTEGER ITEMS FROM A TO B
C     USING A FORWARDS DO LOOP
C
      INTEGER A(1),B(1)
C
      IF(N .LE. 0) RETURN
C
      DO 10 I = 1, N
 10     B(I) = A(I)
C
      RETURN
C
      END

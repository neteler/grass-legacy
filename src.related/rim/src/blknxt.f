      SUBROUTINE BLKNXT(NXT)
      INCLUDE 'syspar.d'
C
C  CLEAR OUT THE CURRENT BLOCK SET AND RETURN A POINTER TO THE
C  FIRST AVAILABLE WORD IN //BUFFER.
C
      INCLUDE 'incore.d'
C
      CALL BLKCLN
      NXT = NEXT
      RETURN
      END

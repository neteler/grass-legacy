      SUBROUTINE WHETOL
      INCLUDE 'syspar.d'
C
C     THIS ROUTINE CHANGES THE WHERE COMMON BLOCK TO REFLECT
C     TOLERANCES WHERE POSSIBLE.  LE,LT,GE,GT TOLERANCES ARE
C     CRANKED INTO WHCOM TO AVOID CALCULATING THEM FOR EVERY
C     ROW.  EQ AND NE WILL BE DONE IN KOMPAR.
C
      INCLUDE 'rmatts.d'
      INCLUDE 'whcom.d'
      INCLUDE 'flags.d'
      INCLUDE 'rimptr.d'
      IF(TOL.EQ.0.) RETURN
      IF(NBOO.EQ.0) RETURN
      IF(KATTY(NBOO).EQ.KZREAL) NS = 0
      IF(KATTY(NBOO).EQ.KZDOUB) NS = 0
      DO 1000 I=1,NBOO
      IF(KATTY(I).EQ.KZTEXT) GO TO 1000
      IF(KATTY(I).EQ.KZINT) GO TO 1000
      IF(KOMTYP(I).LT.4) GO TO 1000
      IF(KOMTYP(I).GT.7) GO TO 1000
C
C     CHANGE THEM VALUES
C
      NUM = KOMLEN(I)
      NPOS = KOMPOS(I)
      NPOT = KOMPOT(I)
      DO 100 J=1,NUM
      CALL ITOH(NR,NW,WHRLEN(NPOT))
      NPOT = NPOT + 1
      IF(KATTY(I).EQ.KZREAL) CALL TOLER(KOMTYP(I),WHRVAL(NPOS),NW)
      IF(KATTY(I).EQ.KZDOUB) CALL TOLED(KOMTYP(I),WHRVAL(NPOS),NW/2)
      NPOS = NPOS + NW
  100 CONTINUE
 1000 CONTINUE
      RETURN
      END

      SUBROUTINE DELETE(MAT)
      INCLUDE 'syspar.d'
C
C  THIS ROUTINE PROCESSES A DELETE IN RIM.
C
C  PARAMETERS
C         MAT-----ARRAY TO HOLD ONE TUPLE
      INCLUDE 'ascpar.d'
      INCLUDE 'start.d'
      INCLUDE 'tupler.d'
      INCLUDE 'tuplea.d'
      INCLUDE 'rimcom.d'
      INCLUDE 'rimptr.d'
      INCLUDE 'files.d'
      INTEGER COLUMN
C
C  DIMENSION STATEMENTS.
C
      DIMENSION MAT(1)
C
      ND = 0
C
C  SEQUENCE THROUGH THE DATA DELETING TUPLES.
C
      IF(NTUPLE.LE.0) GO TO 9999
      IID = CID
  200 CONTINUE
      CALL RMLOOK(MAT,1,0,LENGTH)
      IF(RMSTAT.NE.0) GO TO 700
C
C  DELINK THIS TUPLE.
C
      CALL DELDAT(1,CID)
C
C  PROCESS ANY KEY ATTRIBUTES.
C
      J = LOCATT(BLANK,NAME)
  400 CONTINUE
      CALL ATTGET(ISTAT)
      IF(ISTAT.NE.0) GO TO 600
      IF(ATTKEY.EQ.0) GO TO 400
      COLUMN = ATTCOL
      IF(ATTWDS.NE.0) GO TO 500
      COLUMN = MAT(ATTCOL)
      COLUMN = COLUMN + 2
  500 CONTINUE
      START = ATTKEY
      CALL BTREP(MAT(COLUMN),0,CID,ATTYPE)
      GO TO 400
  600 CONTINUE
      IF(CID.EQ.IID) IID = NID
      ND = ND + 1
      GO TO 200
C
C  CHANGE THE STARTING ID IF NEEDED.
C
  700 CONTINUE
      CALL RELGET(ISTAT)
      RSTART = IID
      NTUPLE = NTUPLE - ND
      IF(NTUPLE.EQ.0) REND = 0
C.
      IF(NTUPLE.LE.0)RSTART = 0
C.
      CALL RELPUT
      RMSTAT = 0
 9999 CONTINUE
      CALL MSG(' ',' ','+')
      CALL IMSG(ND,8,'+')
      CALL MSG(' ',' ROWS WERE DELETED.',' ')
C
C  DONE.
C
      RETURN
      END

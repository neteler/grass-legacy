C This is used to get the direct-for executable code

      IMPLICIT INTEGER*2 (A-Z)
      CHARACTER*160 DIFILE
      CHARACTER*160 ELFILE, infile
      CHARACTER*6 SYSTEM
      LOGICAL ACTIVE(20000),ACTIVITY,GOAGAIN
      INTEGER*2 LEVEL(20000,3)
      DIMENSION DIR(20000),SDIR(20000,3),SELECT(256)
      EQUIVALENCE (SDIR(1,1),LEVEL(1,1))
      COMMON ACTIVE,LEVEL,DIR
      DATA SELECT/  0,  1,  2,  2,  4,  1,  2,  2,  8,  1,
     *  8,  2,  8,  4,  4,  2, 16, 16, 16,  2, 16,  4,  4,
     *  2,  8,  8,  8,  8,  8,  8,  8,  4, 32,  1,  2,  2,
     *  4,  4,  2,  2, 32,  8,  8,  2,  8,  8,  4,  4, 32,
     * 32, 32, 32, 16, 32,  4,  2, 16, 16, 16, 16,  8, 16,
     *  8,  8, 64, 64, 64,  1, 64,  1,  2,  2, 64, 64,  8,
     *  2,  8,  8,  4,  2, 16, 64, 64,  2, 16, 64,  2,  2,
     * 16,  8,  8,  8,  8,  8,  8,  4, 32, 64, 32,  1, 32,
     * 32, 32,  2, 32, 32, 32,  2, 32,  8,  4,  4, 32, 32,
     * 32, 32, 32, 32, 32, 32, 32, 32, 16, 16, 16, 16,  8,
     *  8,128,128,128,  1,  4,  1,  2,  2,128,128,  2,  1,
     *  8,  4,  4,  2, 16,128,  2,  1,  4,128,  2,  1,  8,
     *128,  8,  1,  8,  8,  4,  2, 32,128,  1,  1,128,128,
     *  2,  1, 32,128, 32,  1,  8,128,  4,  2, 32, 32, 32,
     *  1, 32,128, 32,  1, 16, 16, 16,  1, 16, 16,  8,  4,
     *128,128,128,128,128,128,  2,  1,128,128,128,  1,128,
     *128,  4,  2, 64,128,128,  1,128,128,128,  1,  8,128,
     *  8,  1,  8,  8,  8,  2, 64,128, 64,128, 64,128, 64,
     *128, 32, 64, 64,128, 64, 64, 64,  1, 32, 64, 64,128,
     * 64, 64, 64,128, 32, 32, 32, 64, 32, 32, 16,128/
      I1=1
      I2=2
      I3=3
      PRINT *,'ENTER NL,NS,CODE 1 TO RESTART,ELEV FILE,DIR FILE,SYSTEM(U
     *NIX,VMS,PRIME)'
C-- LFILE IS THE I*2 ELEVATION LEVELS
      LFILE=11
C-- SFILE IS THE SELECTED DIR FILE THAT IS MADE NEXT, I*2
      SFILE=15
      read(*,'(a)') infile
      OPEN(UNIT=10,STATUS='OLD',FILE=infile)
c     OPEN(UNIT=10,STATUS='OLD',file='zap123.txt')
c     READ(10,*)NL,NS,CODE,ELFILE,DIFILE,SYSTEM
      READ(10,*)NL,NS,CODE
      READ(10, '(a)')ELFILE
      READ(10, '(a)')DIFILE
      READ(10, '(a)')SYSTEM
      IF (SYSTEM.EQ.'VMS') GOTO 701
      IF (SYSTEM.EQ.'PRIME') GOTO 710
C
C     This section is for opening files under UNIX
C
      OPEN(UNIT=LFILE,STATUS='OLD',RECL=NS*2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=ELFILE)
      IF(CODE.EQ.1)GOTO 12
      OPEN(UNIT=SFILE,STATUS='NEW',RECL=NS*2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',file=DIFILE)
      GOTO 14
 12   OPEN(UNIT=SFILE,STATUS='OLD',FORM='UNFORMATTED',
     *   ACCESS='DIRECT',file=DIFILE,RECL=NS*2)
      GOTO 41
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C     This section is for opening files under VMS
C
 701  CONTINUE
      OPEN(UNIT=LFILE,STATUS='OLD',RECL=(NS+1)/2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=ELFILE)
      IF(CODE.EQ.1)GOTO 712
      OPEN(UNIT=SFILE,STATUS='NEW',RECL=(NS+1)/2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',file=DIFILE)
      GOTO 14
 712  OPEN(UNIT=SFILE,STATUS='OLD',FORM='UNFORMATTED',
     *   ACCESS='DIRECT',file=DIFILE,RECL=(NS+1)/2)
      GOTO 41
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C     This section is for opening files under PRIMOS
C
 710  CONTINUE
      OPEN(UNIT=LFILE,STATUS='OLD',RECL=NS,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=ELFILE)
      IF(CODE.EQ.1)GOTO 713
      OPEN(UNIT=SFILE,STATUS='NEW',RECL=NS,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',file=DIFILE)
      GOTO 14
 713  OPEN(UNIT=SFILE,STATUS='OLD',FORM='UNFORMATTED',
     *   ACCESS='DIRECT',file=DIFILE,RECL=NS)
      GOTO 41
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C--  DIR CODES AS...  7  8  1
C                     6     2
C                     5  4  3
 14   DO 5 I=1,NS+2
      DIR(I)=0
 5    LEVEL(I,1)=0
      LEVEL(1,2)=0
      LEVEL(NS+2,2)=0
      LEVEL(1,3)=0
      LEVEL(NS+2,3)=0
      READ (LFILE,REC=1)(LEVEL(II,I2),II=2,NS+1)
      DO 10 I=2,NL+1
      IF (I.LE.NL)GOTO 15
      DO 13 II=1,NS
 13   LEVEL(II,I3)=0
      GOTO 17
 15   READ(LFILE,REC=I)(LEVEL(II,I3),II=2,NS+1)
C-- PROCESS FIRST AND LAST SAMPLES IN THE LINE
 17   DO 20 J=2,NS+1
      DIR(J)=0
      IF(LEVEL(J,I2).EQ.0)GOTO 20
      DIR(J)=THEDIR(LEVEL(J,I2),LEVEL(J+1,I1),LEVEL(J+1,I2),
     *              LEVEL(J+1,I3),LEVEL(J,I3),LEVEL(J-1,I3),
     *              LEVEL(J-1,I2),LEVEL(J-1,I1),LEVEL(J,I1))
 20   CONTINUE
      WRITE(SFILE,REC=I-1)(DIR(II),II=2,NS+1)
      ITEMP=I1
      I1=I2
      I2=I3
      I3=ITEMP
 10   CONTINUE
C-- NOW MAKE A PASS RESOLVING NON-FLATS WITH MORE THAN ONE DOWN LINK
      DO 30 I=1,NL
      READ(SFILE,REC=I)(DIR(J),J=1,NS)
      DO 31 J=1,NS
      IF(DIR(J).LT.0)GOTO 31
      DIR(J)=SELECT(DIR(J)+1)
 31   CONTINUE
 30   WRITE(SFILE,REC=I)(DIR(J),J=1,NS)
C-- NOW ITERATE ON THE SFILE, LINKING IN THE FLATS.
 41   DO 40 I=2,NL-1
 40   ACTIVE(I)=.TRUE.
      ACTIVE(1)=.FALSE.
      ACTIVE(NL)=.FALSE.
      I1=1
      I2=2
      I3=3
      FIRSTL=2
      LASTL=NL-1
      PASS=0
C
C     PROCESS THE DOWNWARD PASS
C
 35   ACTIVITY=.FALSE.
      READ(SFILE,REC=FIRSTL-1)(SDIR(J,I1),J=1,NS)
      READ(SFILE,REC=FIRSTL)(SDIR(J,I2),J=1,NS)
      READ(SFILE,REC=FIRSTL+1)(SDIR(J,I3),J=1,NS)
      PASS=PASS+1
      PRINT *,'DOWNWARD PASS',PASS,'FIRSTL',FIRSTL,'LASTL',LASTL
      I=FIRSTL
 42   ACTIVE(I)=.FALSE.
 51   GOAGAIN = .FALSE.
      DO 50 J=2,NS-1
      DIR(J)=SDIR(J,I2)
 50   IF(SDIR(J,I2).LT.0)DIR(J)=FLINK(SDIR(J,I2),ACTIVE(I),SDIR(J+1,I1),
     *   SDIR(J+1,I2),SDIR(J+1,I3),SDIR(J,I3),
     *   SDIR(J-1,I3),SDIR(J-1,I2),SDIR(J-1,I1),
     *   SDIR(J,I1),SELECT,ACTIVITY,GOAGAIN)
      IF (GOAGAIN) GOTO 51
      WRITE(SFILE,REC=I)SDIR(1,I2),(DIR(K),K=2,NS-1),SDIR(NS,I2)
C-- ROTATE TO THE NEXT LINE
 45   I=I+1
      IF(I.GT.LASTL+1)GOTO 60
      ITEMP=I1
      I1=I2
      I2=I3
      I3=ITEMP
      IF(.NOT.ACTIVE(I).AND..NOT.ACTIVE(I+1)
     *    .AND..NOT.ACTIVE(I+2))GOTO 45
      READ(SFILE,REC=I+1)(SDIR(K,I3),K=1,NS)
      IF(.NOT.ACTIVE(I))GOTO 45
      GOTO 42
C-- DONE WITH THIS ITERATION,  UPDATE FIRSTL AND LASTL AND GO AGAIN
 60   DO 70 I=FIRSTL,LASTL
      IF(ACTIVE(I))GOTO 73
 70   CONTINUE
C-- ALL DONE
      GOTO 100
 73   IF (ACTIVITY)GOTO 75
      PRINT *,'COULD NOT SOLVE FOR ALL CELLS'
      GOTO 100
 75   FIRSTL=I
      DO 80 I=LASTL,FIRSTL,-1
      IF(ACTIVE(I))GOTO 85
 80   CONTINUE
 85   LASTL=I
C
C     PROCESS THE UPWARD PASS
C
 350  ACTIVITY=.FALSE.
      READ(SFILE,REC=LASTL+1)(SDIR(J,I3),J=1,NS)
      READ(SFILE,REC=LASTL)(SDIR(J,I2),J=1,NS)
      READ(SFILE,REC=LASTL-1)(SDIR(J,I1),J=1,NS)
      PASS=PASS+1
      PRINT *,'UPWARD PASS',PASS,'FIRSTL',FIRSTL,'LASTL',LASTL
      I=LASTL
 420  ACTIVE(I)=.FALSE.
 510  GOAGAIN = .FALSE.
      DO 500 J=2,NS-1
      DIR(J)=SDIR(J,I2)
 500  IF(SDIR(J,I2).LT.0)DIR(J)=FLINK(SDIR(J,I2),ACTIVE(I),SDIR(J+1,I1),
     *   SDIR(J+1,I2),SDIR(J+1,I3),SDIR(J,I3),
     *   SDIR(J-1,I3),SDIR(J-1,I2),SDIR(J-1,I1),
     *   SDIR(J,I1),SELECT,ACTIVITY,GOAGAIN)
      IF (GOAGAIN) GOTO 510
      WRITE(SFILE,REC=I)SDIR(1,I2),(DIR(K),K=2,NS-1),SDIR(NS,I2)
C-- ROTATE TO THE NEXT LINE
 450  I=I-1
      IF(I.LT.FIRSTL-1)GOTO 600
      ITEMP=I3
      I3=I2
      I2=I1
      I1=ITEMP
      IM1=MAX(I-1,1)
      IM2=MAX(I-2,1)
      print *,im1,im2
      IF(.NOT.ACTIVE(I).AND..NOT.ACTIVE(IM1)
     *    .AND..NOT.ACTIVE(IM2))GOTO 450
      READ(SFILE,REC=I-1)(SDIR(K,I1),K=1,NS)
      IF(.NOT.ACTIVE(I))GOTO 450
      GOTO 420
C-- DONE WITH THIS ITERATION,  UPDATE FIRSTL AND LASTL AND GO AGAIN
 600  DO 700 I=LASTL,FIRSTL,-1
      IF(ACTIVE(I))GOTO 730
 700  CONTINUE
C-- ALL DONE
      GOTO 100
 730  IF (ACTIVITY)GOTO 750
      PRINT *,'COULD NOT SOLVE FOR ALL CELLS'
      GOTO 100
 750  LASTL=I
      DO 800 I=FIRSTL,LASTL
      IF(ACTIVE(I))GOTO 850
 800  CONTINUE
 850  FIRSTL=I
C
C     END OF UPWARD PROCESS
C
      GOTO 35
 100  CONTINUE
      STOP
      END
C
C
      FUNCTION THEDIR(MID,N1,N2,N3,N4,N5,N6,N7,N8)
      IMPLICIT INTEGER*2 (A-Z)
      REAL N(8),MAXDROP
C-- RETURN A <0 MASK IF THE PATHS ARE FLAT.
      THEDIR=0
      N(1)=(MID-N1)/1.414
      N(2)= MID-N2
      N(3)=(MID-N3)/1.414
      N(4)= MID-N4
      N(5)=(MID-N5)/1.414
      N(6)= MID-N6
      N(7)=(MID-N7)/1.414
      N(8)= MID-N8
      MAXDROP=MAX(N(1),N(2),N(3),N(4),N(5),N(6),N(7),N(8))
      DO 10 I=1,8
 10   IF (N(I).EQ.MAXDROP)THEDIR=THEDIR+2**(I-1)
      IF(MAXDROP.EQ.0)THEDIR=-THEDIR
c-- A PIT WILL BE A -300
      IF(MAXDROP.LT.0)THEDIR=-300
      RETURN
      END
C
C
      FUNCTION FLINK(CENTER,ACTIVE,D1,D2,D3,D4,D5,D6,D7,D8,SELECT
     * ,ACTIVITY,GOAGAIN)
      IMPLICIT INTEGER*2(A-Z)
      LOGICAL ACTIVE,C(8),ACTIVITY,GOAGAIN
      DIMENSION SELECT(256),BITMASK(8)
      DATA BITMASK/1,2,4,8,16,32,64,128/
      FLINK=CENTER
C-- CHECK IF ITS A PIT
      IF(FLINK.EQ.-300)GOTO 100
      CWORK=-CENTER
      DO 5 I=8,1,-1
      C(I)=.FALSE.
      IF(CWORK-BITMASK(I).LT.0)GOTO 5
      CWORK=CWORK-BITMASK(I)
      C(I)=.TRUE.
 5    CONTINUE
C-- CHECK FOR DOWNSTREAM LINKS
      OUTFLOW=0
      IF(D1.NE.16.AND.D1.GT.0.AND.C(1))OUTFLOW=OUTFLOW+1
      IF(D2.NE.32.AND.D2.GT.0.AND.C(2))OUTFLOW=OUTFLOW+2
      IF(D3.NE.64.AND.D3.GT.0.AND.C(3))OUTFLOW=OUTFLOW+4
      IF(D4.NE.128.AND.D4.GT.0.AND.C(4))OUTFLOW=OUTFLOW+8
      IF(D5.NE.1.AND.D5.GT.0.AND.C(5))OUTFLOW=OUTFLOW+16
      IF(D6.NE.2.AND.D6.GT.0.AND.C(6))OUTFLOW=OUTFLOW+32
      IF(D7.NE.4.AND.D7.GT.0.AND.C(7))OUTFLOW=OUTFLOW+64
      IF(D8.NE.8.AND.D8.GT.0.AND.C(8))OUTFLOW=OUTFLOW+128
      IF(OUTFLOW.EQ.0)GOTO 10
      CENTER=SELECT(OUTFLOW+1)
      FLINK=CENTER
      ACTIVITY=.TRUE.
      GOAGAIN=.TRUE.
      GOTO 100
 10   ACTIVE=.TRUE.
 100  CONTINUE
      RETURN
      END

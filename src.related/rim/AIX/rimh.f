      PROGRAM MAIN
      INCLUDE 'syspar.d'
C
C     GENERATE THE HELP DATABASE
C
c     (UNIX version to open correct files)
c
c
C  THE HELPDB SCHEMA IS DEFINED AS FOLLOWS:
C
C  DEFINE RIM_HELP           *(UWRIM HELP DATABASE)
C  OWNER 'RIM'
C  ATTRIBUTES
C  COMKEY    TEXT          3   *(COMMAND KEYWORD)
C  SUBKEY    TEXT          3   *(SUB-COMMAND KEYWORD)
C  COMTXT    TEXT       VAR    *(TEXT LINE OF HELP TEXT)
C
C  *(THERE IS NO EXPLICIT SEQUENCING FOR THE TEXT RECORDS
C    BELONGING TO A SPECIFIC COMMAND AND SUB-COMMAND.
C    THEY ARE RETRIEVED AS THEY WERE LOADED.)
C
C  RELATIONS
C  HELP_TEXT     WITH COMKEY SUBKEY COMTXT
C  END
C
C
C  INPUT RECORDS HAVE A COMMAND CHARACTER IN COLUMN 1 AND
C  TEXT IN COLS 2 - 80
C
C     '*' = COMMENT
C     '.' = BEGIN NEW COMMAND OR SUBCOMMAND
C     '&' = BEGIN MACRO DEFINITION
C     '=' = COPY MACRO TEXT
C
      INCLUDE 'ascpar.d'
      INCLUDE 'flags.d'
      INCLUDE 'rimcom.d'
      INCLUDE 'dclar1.d'
      LOGICAL RIM
C
      PARAMETER (NW = 80/ZCW)
      COMMON /LINE/ UNIT, LINE(NW), LINEL, EOF
      CHARACTER*1 COM
      CHARACTER*4 COMK, SUBK
 
C     DATA TO HOLD THE TEXT TUPLES
      PARAMETER (TPL=3+NW+2)
      PARAMETER (TXTSRT=4)
      COMMON /TUP/ TUPLE(TPL)
C
C     MACRO TABLE
      PARAMETER (MACMXL=300,MACMRL=NW)
      INTEGER MACKEY(MACMXL), MACBUF(MACMRL,MACMXL), MACLEN(MACMXL)
C
C     SET TO IGNORE INPUT ERROR 212 (SHORT RECORDS)
C                     AND ERROR 219 (MISSING FILES)
C                     AND SOME OTHER FILE ERRORS 
C
      CALL ERRSET(212,256,-1,1,1,1)
      CALL ERRSET(219,256,-1,1,1,1)
      CALL ERRSET(218,256,-1,1,1,1)
      CALL ERRSET(171,256,-1,1,1,1)

      OPEN (2,FILE='rim_help.data',STATUS='OLD')
 
 
      IF (.NOT.RIM(1,'OPEN rim_help')) THEN
         WRITE(6,1000) RMSTAT
1000     FORMAT(' OPEN ERROR:',I5)
         GOTO 900
      ENDIF
      IF (.NOT.RIM(1,'LOAD HELP_TEXT')) THEN
         WRITE(6,1100) RMSTAT
1100     FORMAT(' FIND ERROR:',I5)
         GOTO 900
      ENDIF
 
      UNIT = 2
      TUPLE(3) = TXTSRT
      TUPLE(TXTSRT+1) = 0
      MACIDX = 0
C
10    IF (GETLIN(COM).NE.0) GOTO 800
20    IF (LINEL.EQ.0) GOTO 10
      IF (COM.EQ.'*') GOTO 10
      IF (COM.EQ.'.') GOTO 100
      IF (COM.EQ.'&') GOTO 200
      IF (COM.EQ.'=') GOTO 300
      GOTO 10
C
C     START NEW COMMAND
C
100   TUPLE(1) = KEY3(TUPLE(2))
      CALL STRASC(COMK,TUPLE(1),4)
      CALL STRASC(SUBK,TUPLE(2),4)
      WRITE(6,1200) COMK, SUBK
1200  FORMAT (' COMMAND: ',A4,1X,A4)
C
C     FIRST ROW IS THE COMMAND
C
110   CALL LODROW(LINE,LINEL)
      IF (GETLIN(COM).NE.0) GOTO 900
      IF (COM.NE.' ') GOTO 20
      GOTO 110
C
C     READ MACRO TEXT
C
200   KEY = KEY3(SUB)
      CALL STRASC(COMK,KEY,4)
      WRITE(6,1300) COMK
1300  FORMAT (' MACRO: ',A4)
210   IF (GETLIN(COM).NE.0) GOTO 900
      IF (COM.NE.' ') GOTO 20
      MACIDX = MACIDX + 1
      IF (MACIDX.GT.MACMXL) THEN
         WRITE(6,1400)
1400     FORMAT ('TOO MANY MACRO TABLE ENTRIES')
         GOTO 900
      ENDIF
      MACKEY(MACIDX) = KEY
      DO 220 I = 1, NW
220   MACBUF(I,MACIDX) = LINE(I)
      MACLEN(MACIDX) = LINEL
      GOTO 210
C
C     COPY MACRO TEXT
C
300   KEY = KEY3(SUB)
      DO 310 I = 1, MACIDX
      PTR = I
      IF (MACKEY(I).EQ.KEY) GOTO 320
310   CONTINUE
      CALL STRASC(COMK,KEY,4)
      WRITE(6,1500) COMK
1500  FORMAT(' MACRO NOT FOUND: ',A4)
      GOTO 10
C
320   CALL LODROW(MACBUF(1,PTR),MACLEN(PTR))
      PTR = PTR + 1
      IF (PTR.GT.MACIDX) GOTO 10
      IF (MACKEY(PTR).NE.KEY) GOTO 10
      GOTO 320
C
C     AT END - BUILD KEY FOR COMKEY
C
800   WRITE(6,1800)
1800  FORMAT(' BUILDING KEY FOR COMKEY')
      IF (RIM(1,'BUILD KEY FOR COMKEY IN HELP_TEXT')) GOTO 900
      WRITE(6,1810) RMSTAT
1810  FORMAT(' ERROR: ',I5)
C
900   IF (RIM(1,'CLOSE')) CALL EXIT
      STOP
      END
      FUNCTION GETLIN(COM)
      INCLUDE 'syspar.d'
C
C     GET A LINE OF TEXT
C
      CHARACTER*1 COM
      INCLUDE 'ascpar.d'
 
      PARAMETER (NW = 80/ZCW)
      COMMON /LINE/ UNIT, LINE(NW), LINEL, EOF
      CHARACTER*80 CLINE
C
      GETLIN = 0
      READ (2,10,END=900)CLINE
   10 FORMAT(A80)
      DO 100 I = 80,1,-1
      LINEL = I
      IF (CLINE(I:I).NE.' ') GOTO 110
100   CONTINUE
110   COM = CLINE(1:1)
      DO 200 I = 1,NW
200   LINE(I) = BLANK(1)
      DO 300 I = 2,LINEL
300   CALL PUTT(LINE,I,ASCCHR(CLINE(I:I)))
      RETURN
C
900   GETLIN = 1
      WRITE(6,1000)
1000  FORMAT(' EOF ON UNIT 2')
      RETURN
      END
      FUNCTION KEY3(SUB)
      INCLUDE 'syspar.d'
C
C     GET FIRST THREE CHARS OF FIRST WORD
C     GET FIRST THREE CHARS OF SECOND WORD TO SUB
C
      INCLUDE '../src/ascpar.d'
 
      PARAMETER (NW = 80/ZCW)
      COMMON /LINE/ UNIT, LINE(NW), LINEL, EOF
C
      KEY3 = BLANK(1)
      SUB  = BLANK(1)
 
      WN = 1
      CN = 1
      DO 100 I = 2, LINEL
      CALL GETT(LINE,I,CH)
      IF (CH.EQ.ABLANK) THEN
         IF (CN.NE.1) WN = WN + 1
         CN = 1
         GOTO 100
      ENDIF
 
      IF (CN.LE.3) THEN
         IF (WN.EQ.1) CALL PUTT(KEY3,CN,UPCASE(CH))
         IF (WN.EQ.2) CALL PUTT(SUB ,CN,UPCASE(CH))
         CN = CN + 1
      ENDIF
100   CONTINUE
900   RETURN
      END
      SUBROUTINE LODROW(LINE,NC)
      INCLUDE 'syspar.d'
C
C     WRITE A LINE TO DB
C
      INCLUDE '../src/rimcom.d'
      LOGICAL RIMDM
C
      PARAMETER (NW = 80/ZCW)
      INTEGER LINE(NW)
 
C     DATA TO HOLD THE TEXT TUPLES
      PARAMETER (TPL=3+NW+2)
      PARAMETER (TXTSRT=4)
      COMMON /TUP/ TUPLE(TPL)
 
      DO 120 I = 1, NW
120   TUPLE(TXTSRT+1+I) = LINE(I)
      CALL PUTT(TUPLE(TXTSRT+1+I),1,ABLANK)
      TUPLE(TXTSRT) = NC
130   IF (.NOT.RIMDM(1,'LOAD',TUPLE)) THEN
         WRITE(6,1000) NC, RMSTAT
1000     FORMAT(' LOAD ERROR: ',2I5)
      ENDIF
      RETURN
      END

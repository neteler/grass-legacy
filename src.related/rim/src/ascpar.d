C
C  *** / A S C P A R / ***
C
C  ASCII-TEXT PARAMETERS ( MUST BE VARIABLES )
C
      COMMON /ASCPAR/ ABLANK,BLANK(Z),NONE(Z),
     X  KDBHDR(Z),
     X  KARRC(Z),KARDT(Z),KANAM(Z),
     X  KANUM(Z),KAAOR(Z),KAAN1(Z),KARN1(Z),KAOPR(Z),KATYP(Z),
     X  KAAN2(Z),KARN2(Z),KAVAL(Z),KAXXX(Z),
     X  KZHPDB(Z),KZHPRL(Z),KZHPKY(Z),KZHPSK(Z),KZHPTX(Z),
     X  KDATFM,KTIMFM,
     X  ASMTXT(12)
C
C  PARAMETER DEFINITIONS:
C
C     ABLANK --- A ASCII-CHAR BLANK
C     BLANK ---- ASCII-TEXT (Z) BLANKS
C     NONE ----- PASSWORD FOR 'NO PASSWORD'
C
C     KDBHDR --- ASCII-TEXT DATABASE IDENT
C
C     KLKREL --- LINK RELATION NAME
C
C     KARDT-KAXXX - RULE RELATION AND ATTRIBUTE NAMES
C
C     HELP TEXT DATABASE NAMES
C
C     KZHPDB   - HELP DATABASE NAME
C     KZHPRL   - HELP TEXT RELATION NAME
C     KZHPKY   - HELP TEXT KEY ATTRIBUTE NAME
C     KZHPSK   - HELP TEXT SUBKEY ATTRIBUTE NAME
C     KZHPTX   - HELP TEXT TEXT ATTRIBUTE NAME
C
C     DEFAULT FORMATS
C
C     KDATFM  -- DATE PRINT FORMAT
C     KTIMFM  -- TIME PRINT FORMAT
C
C     ASMTXT  -- ASCII-TEXT OF MONTH NAMES (3-CHARS)
C

C
C  *** / R I M P T R / ***
C
C  RIM INTERNAL POINTERS
C
      COMMON /RIMPTR/ IVAL,LIMVAL,CID,NID,NS,MID,INDCUR,INDMAX
      PARAMETER (ZRMPTR=6)
C
C  VARIABLE DEFINITIONS:
C     IVAL----CURRENT TUPLE NUMBER
C     LIMVAL--COUNT OF TUPLES SATISFYING WHERE
C     CID-----CURRENT TUPLE ID
C     NID-----NEXT TUPLE ID
C     NS------SORTING AND INDEX FLAG
C               0 = SEQUENTIAL SCAN OR DONE
C               1 = SORTED RETRIEVAL
C               2 = INDEXED RETRIEVAL (FIRST VALUE)
C               3 = INDEXED RETRIEVAL (MULTIPLE VALUES)
C     MID-----MULTIPLE OCCURENCE ID VALUE (NEXT MOT INDEX)
C     INDCUR--CURRENT INDEX TO THE THE SET OF PARTITIONS IN USE
C     INDMAX--MAXIMUM NUMBER OF POINTERS IN USE
C

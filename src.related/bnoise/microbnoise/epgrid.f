	SUBROUTINE EPGRID
	CHARACTER*5 IB
	WRITE(7,10)
10	FORMAT(1H ,'PUDDLE GRID',69X)
20	WRITE(0,30)
30	FORMAT(/' INVERSION FACTOR 1: ')
	READ(5,*,ERR=20)RA
40	WRITE(0,50)
50	FORMAT(/' INVERSION FACTOR 2: ')
	READ(5,*,ERR=40)RB
60	WRITE(0,70)
70	FORMAT(/' INVERSION FACTOR 3: ')
	READ(5,*,ERR=60)RC
80	WRITE(0,90)
90	FORMAT(/' GRID SIZE [2000]: ')
	READ(5,*,ERR=80)GS
	WRITE(0,100)
100	FORMAT(/' "DAY", "NIGHT" OR "BOTH": ')
	READ(5,110)IB
110	FORMAT(A5)
	WRITE(7,120)RA,RB,RC,GS,IB
120	FORMAT(1H ,3F10.1,F10.0,A5,35X)
	RETURN
	END

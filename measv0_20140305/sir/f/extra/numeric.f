C extra utility routine
C
C ********************************************************************
C
	LOGICAL FUNCTION NUMERIC(A)
C
C	TESTS FOR A NUMERIC CHARACTER
C	I.E.  "-", ".", "0" THROUGH "9", "E", "e"
C
C	A	(CHAR*1)	ASCII VALUE TO TEST
C
	CHARACTER*1 A
	NUMERIC=.TRUE.
	IF (A.EQ.'.'.OR.A.EQ.'e'.OR.A.EQ.'E'.OR.A.EQ.'-') RETURN
	IF (A.GT.'/'.AND.A.LT.':') RETURN
	NUMERIC=.FALSE.
	RETURN
	END

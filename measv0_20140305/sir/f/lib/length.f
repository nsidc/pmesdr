C
C **************************************************************************
C
	INTEGER FUNCTION LENGTH(LINE)
C
C	COMPUTES THE LENGTH (NON-BLANK) OF A STRING
C
	CHARACTER*(*) LINE
	N=LEN(LINE)
	DO I=N,1,-1
		IF (LINE(I:I).NE.' '.AND.LINE(I:I).NE.CHAR(0)) THEN
			LENGTH=I
			GOTO 10
		ENDIF
	END DO
	LENGTH=1
10	RETURN
	END

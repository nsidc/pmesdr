C
        SUBROUTINE F2IPIX(X,Y,IX,IY,NSX,NSY)
C
C     QUANTIZES F FLOATING POINT PIXEL VALUE TO THE "ACTUAL" INTEGER
C     PIXEL VALUE  (SEE LATLON2PIX)
C
        IF (X.GE.1.0.AND.X.LT.NSX+1) THEN
           IX=IFIX(X+0.0001)  ! ALLOW A SMALL AMOUNT OF ROUNDING
        ELSE
           IX=0
        ENDIF
        IF (Y.GE.1.0.AND.Y.LT.NSY+1) THEN
           IY=IFIX(Y+0.0001)  ! ALLOW A SMALL AMOUNT OF ROUNDING
        ELSE
           IY=0
        ENDIF
        RETURN
        END

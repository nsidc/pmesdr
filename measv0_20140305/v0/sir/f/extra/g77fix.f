C
C Unfortunately, key intrinsic routines are not completely implemented
C in linux g77, resulting in apparent compiler errors and warnings.
C The function definitions below supply the missing routines and should
C be included in the library when using linux g77.
C

      function sind(x)
      sind=sin(x*3.141592654/180.0)
      return
      end

      function cosd(x)
      cosd=cos(x*3.141592654/180.0)
      return
      end

      function tand(x)
      tand=tan(x*3.141592654/180.0)
      return
      end

      function atand(x)
      atand=atan(x)*180.0/3.141592654
      return
      end

      function asind(x)
      asind=asin(x)*180.0/3.141592654
      return
      end

      function acosd(x)
      acosd=acos(x)*180.0/3.141592654
      return
      end

      function atan2d(x,y)
      atan2d=atan2(x,y)*180.0/3.141592654
      return
      end

      double precision function dsind(x)
      double precision x
      dsind=sin(x*3.141592654d0/180.0d0)
      return
      end

      double precision function dcosd(x)
      double precision x
      dcosd=cos(x*3.141592654d0/180.0d0)
      return
      end

      double precision function dtand(x)
      double precision x
      dtand=tan(x*3.141592654d0/180.0d0)
      return
      end

      double precision function datand(x)
      double precision x
      datand=atan(x)*180.0d0/3.141592654d0
      return
      end

      double precision function dasind(x)
      double precision x
      dasind=asin(x)*180.0d0/3.141592654d0
      return
      end

      double precision function dacosd(x)
      double precision x
      dacosd=acos(x)*180.0d0/3.141592654d0
      return
      end

      double precision function datan2d(x,y)
      double precision x,y
      datan2d=atan2(x,y)*180.0d0/3.141592654d0
      return
      end


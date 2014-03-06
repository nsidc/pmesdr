c
c	include file to define platform dependent parameters for
c	reading/writing BYU SIR file formats
c
        integer ISIRRECLENGTH   ! bytes/record in file open statement
				!  (typically 512 or 128)
				!  SIR header record length is 512 bytes
				!  and SIR file is a multiple of 512 bytes long
				!  Set this variable to value used to specify a
				!  512 byte record in file open statement RECL
				!  HPUX, Sun, linux use RECL=512 (bytes/record)
				!  SGI, VAX use RECL=128 (4 byte words/record)
c
	logical BSWAP           ! byte swap if BSWAP is true
				!  SIR files are stored big-endian (the
				!  standard byte order on typical unix
				!  platforms, e.g. HP, SGI, SUN
				!  set BSWAP to .TRUE. for little-endian 
				!  (least-significant byte first) machines
				!  such as VAX and intel machines
				!  set BSWAP to .FALSE. for big-endian 
				!  (most-significant byte first) machines
				!  such as HP, SUN, Motorolla, etc.
c
	logical VMS             ! if .TRUE., VMS-style file read/write is used
				!  otherwise, unix-style file read/write used
				!  (note: some of the VMS open options are
				!  commented out to make non-VMS compilers
				!  happy)
c
c	uncomment appropriate statements for platform
c
	parameter (ISIRRECLENGTH=512)
cc	parameter (ISIRRECLENGTH=128)
cc	parameter (BSWAP=.FALSE.)
	parameter (BSWAP=.TRUE.)
	parameter (VMS=.FALSE.)
cc	parameter (VMS=.TRUE.)






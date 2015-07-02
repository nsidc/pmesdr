Definition files for MEASURES processing

regiondef1.dat       General region definition file

SSMI/I .def files:

 EASE2N_test1ch.def  EASE2-N single channel (separated ltod) def file for E2N - used for rss_quick and csu_quick targets
 E2N_test.def          EASE2-N for E2N 
 E2S_test.def          EASE2-S for E2S 
 E2T_test.def          EASE2-S for E2T 


Format of .def file

first line has the number of regions defined in the file - 
      note that in this context a region is really the combination of a projection, projection scale and channel id
      for example, if you have 5 channels and you wish to process each channel for EASE2N and separate into ltod images, 
      you will create 10 regions

after the first line there will be 5 lines per region e.g. if the first line is 2 (for 2 regions), there will be 10 more lines in the file
      defining each of those 2 regions.

the lines defining each region are:-
    region id
    resolution factor
    asc/des/ltod flag
    beam number
    SIR iterations

region id numbers (308,309,310) - corresponding to EASE2N, EASE2S, EASE2T
EASE grid resolution factor for the region (0, 1, 2, 3) - power
of 2 divided into 25km - factor 0 = 25/(2**0) km,
factor 1 = 25/(2**1) = 12.5 km,
factor 2 = 25/(2**2) = 6.25 km,
factor 3 = 25/(2**3) = 3.125 km
etc.  This resolution is specified, by channel, in the white
paper BGI vs SIR located in bitbucket at
https://bitbucket.org/nsidc/measures-pmesdr/

asc/desc/LTOD flag for the region (0=all,1=asc,2=dsc,3=morn,4=eve)
beam number for region (1=19H, 2=19V, 3=22V, 4=37H, 5=37V, 6=85H, 7=85V) - this is for SSMI/I
number of SIR iterations for this channel


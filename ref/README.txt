Definition files for MEASURES processing

regiondef1.dat       General region definition file

SSMI/I .def files:
 arctic.def          Polarstereographic def file for Arc region
 EASE2N_test1ch.def  EASE2-N single channel def file for E2N
 EASE2N.def          EASE2-N for E2N (no sectioning)
 EASE2S.def          EASE2-S for E2S (no sectioning)
 EASE2T.def          EASE2-S for E2T (no sectioning)


Format of .def file

number of regions (1..10)
region id number for region 1 (308,309,310)
EASE grid resolution factor for region 1 (1,2)
asc/desc/LTOD flag for region 1 (0=all,1=asc,2=dsc,3=morn,4=eve)
beam number for region 1 (1=19V,2=19H,3=22V,4=37V,5=37H,6=85V,7=85H)
sectioning parameter for region 1 (0=no sectioning)
region id number for region 2 (308,309,310)
EASE grid resolution factor for region 2 (1,2)
asc/desc/LTOD flag for region 2 (0=all,1=asc,2=dsc,3=morn,4=eve)
beam number for region 2 (1=19V,2=19H,3=22V,4=37V,5=37H,6=85V,7=85H)
sectioning parameter for region 2 (0=no sectioning)
...
0 (end of file flag)

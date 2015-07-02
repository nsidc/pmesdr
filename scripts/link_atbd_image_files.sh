#!/bin/bash
# Refer to wiki page for filenameing conventions:
# https://nsidc.org/confluence/display/PMESDR/CETB+Data+Set+File+Definition#CETBDataSetFileDefinition-FilenamingConvention

# Row 1: CSU North, 25km, SIR, BGI
ln -s /projects/PMESDR/janus_fullday/sirCSU_3km/FD4m-E2N97-061-061.lis_dump.nc \
    EASE2_N25km.F13_SSMI.1997061.37H.M.SIR.CSU.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/sirCSU_3km/FD4m-E2N97-061-061.lis_dump.nc \
    EASE2_N3.125km.F13_SSMI.1997061.37H.M.SIR.CSU.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/bgiCSU_3km/FD4m-E2N97-061-061.lis_dump1.nc \
    EASE2_N3.125km.F13_SSMI.1997061.37H.M.BGI.CSU.v0.1.nc

# Row 2: RSS North, 25km, SIR, BGI
ln -s /projects/PMESDR/janus_fullday/sirRSS_3km/FD4m-E2N97-061-061.lis_dump.nc \
    EASE2_N25km.F13_SSMI.1997061.37H.M.SIR.RSS.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/sirRSS_3km/FD4m-E2N97-061-061.lis_dump.nc \
    EASE2_N3.125km.F13_SSMI.1997061.37H.M.SIR.RSS.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/bgiRSS_3km/FD4m-E2N97-061-061.lis_dump1.nc \
    EASE2_N3.125km.F13_SSMI.1997061.37H.M.BGI.RSS.v0.1.nc

# Row 3: CSU South, 25km, SIR, BGI
ln -s /projects/PMESDR/janus_fullday/sirCSU_3km/FD4m-E2S97-061-061.lis_dump.nc \
    EASE2_S25km.F13_SSMI.1997061.37H.M.SIR.CSU.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/sirCSU_3km/FD4m-E2S97-061-061.lis_dump.nc \
    EASE2_S3.125km.F13_SSMI.1997061.37H.M.SIR.CSU.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/bgiCSU_3km/FD4m-E2S97-061-061.lis_dump1.nc \
    EASE2_S3.125km.F13_SSMI.1997061.37H.M.BGI.CSU.v0.1.nc

# Row 4: RSS South, 25km, SIR, BGI
ln -s /projects/PMESDR/janus_fullday/sirRSS_3km/FD4m-E2S97-061-061.lis_dump.nc \
    EASE2_S25km.F13_SSMI.1997061.37H.M.SIR.RSS.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/sirRSS_3km/FD4m-E2S97-061-061.lis_dump.nc \
    EASE2_S3.125km.F13_SSMI.1997061.37H.M.SIR.RSS.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/bgiRSS_3km/FD4m-E2S97-061-061.lis_dump1.nc \
    EASE2_S3.125km.F13_SSMI.1997061.37H.M.BGI.RSS.v0.1.nc

# Row 5: CSU T 25km, SIR, BGI
ln -s /projects/PMESDR/janus_fullday/sirCSU_3km/FD4a-E2T97-061-061.lis_dump.nc \
    EASE2_T25km.F13_SSMI.1997061.37H.A.SIR.CSU.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/sirCSU_3km/FD4a-E2T97-061-061.lis_dump.nc \
    EASE2_T3.125km.F13_SSMI.1997061.37H.A.SIR.CSU.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/bgiCSU_3km/FD4a-E2T97-061-061.lis_dump1.nc \
    EASE2_T3.125km.F13_SSMI.1997061.37H.A.BGI.CSU.v0.1.nc

# Row 6: RSS T, 25km, SIR, BGI
ln -s /projects/PMESDR/janus_fullday/sirRSS_3km/FD4a-E2T97-061-061.lis_dump.nc \
    EASE2_T25km.F13_SSMI.1997061.37H.A.SIR.RSS.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/sirRSS_3km/FD4a-E2T97-061-061.lis_dump.nc \
    EASE2_T3.125km.F13_SSMI.1997061.37H.A.SIR.RSS.v0.1.nc
ln -s /projects/PMESDR/janus_fullday/bgiRSS_3km/FD4a-E2T97-061-061.lis_dump1.nc \
    EASE2_T3.125km.F13_SSMI.1997061.37H.A.BGI.RSS.v0.1.nc



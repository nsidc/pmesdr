# PMESDR
This software repository contains the source code to transform passive
microwave radiometer data to gridded format, using image reconstruction methods
developed by D. G. Long at Brigham Young University. The radiometer version of
Scatterometer Image Reconstruction (rSIR) leverages large footprint overlaps and
irregularly-spaced sampling locations to enhanced the spatial resolution of the
output grids.

Copyright (C) 2014-2019 The Regents of the University of Colorado and
Brigham Young University.

This project software is licensed under the GNU General Public
License v3.0.  Please refer to the LICENSE.txt file for details.

References for the rSIR method:

Long, D. G. and M. J. Brodzik. 2016. Optimum Image Formation for
Spaceborne Microwave Radiometer Products. IEEE Transactions on
Geoscience and Remote Sensing, 54(5):2763–2779. doi:
10.1109/TGRS.2015.2505677.

Long, D. G., M. J. Brodzik, and M. A. Hardman. 2019. Enhanced
Resolution SMAP Brightness Temperature Image Products. IEEE
Transactions on Geoscience and Remote Sensing,
1-13. doi:10.1109/TGRS.2018.2889427.

A description of software methods and data set requirements
definitions is included in:

Brodzik, M. J., D. G. Long, and M. A. Hardman. 2018. Best
Practices in Crafting the Calibrated, Enhanced–Resolution
Passive–Microwave EASE-Grid 2.0 Brightness Temperature Earth
System Data Record. Remote Sensing, 10(11), 2018. doi:
10.3390/rs10111793.

This rSIR system has been used to produce the following
Calibrated, Enhanced-Resolution Brightness Temperature (CETB)
data sets:

Brodzik, M. J., D. G. Long, M. A. Hardman, A. Paget, and
R. Armstrong. 2016, Updated 2018. MEaSUREs Calibrated
Enhanced-Resolution Passive Microwave Daily EASE-Grid 2.0
Brightness Temperature ESDR, Version 1. Boulder, Colorado
USA. NASA National Snow and Ice Data Center Distributed Active
Archive Center. doi:
https://doi.org/10.5067/MEASURES/CRYOSPHERE/NSIDC-0630.001.

Brodzik, M. J., D. G. Long, M. A. Hardman. 2019. SMAP Radiometer
Twice-Daily rSIR-Enhanced EASE-Grid 2.0 Brightness
Temperatures. Version 1. Boulder, Colorado USA: NASA DAAC at the
National Snow and Ice Data. doi:
https://doi.org/10.5067/QZ3WJNOUZLFK.

*********************************************************************

2 small changes here.

Base directory for the BYU MEASURES project:

CETB_process/ scripts for running this system on summit supercomputer
docs/     documentation
ease2/    sample EASE2 grid software and test data files
janus_batchfiles/ batchfile scripts for running on CU janus system
pmesdr_utils/ directory with pmesdr_utils python package
python/   directory with general python utilities
ref/      input and control files
sample_data/  sample data files for regression and development testing
scripts/  control scripts
src/      source code for production and display
testing/  directory of testing code and data files

Please also see the 2 further README's in this directory that
explain adding a new sensor and changing the base resolution,
viz. README.add_sensor.txt and README.change_resolution.txt
respectively. 



https://medium.com/@kc_clintone/the-ultimate-guide-to-writing-a-great-readme-md-for-your-project-3d49c2023357

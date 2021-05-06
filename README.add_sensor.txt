To add and process a new sensor with this software:-

1.  The input swath files for the new sensor will need to be
added to the Extended Generic Swath project.  i.e. the new sensor
swath files will need to be output in the required GSX format in
order to be read into the measures-byu software suite.

2.  The file cetb.h will need to be expanded to take note of the
new sensor name, platform, NSIDC dataset name, channels etc.
Most of these will be easy to recognize once the file is
examined, eg there are enums defined for the platform, producer
id etc.

3.  Meas_meta_make will need to be modified to add in a new
single letter identifier that is used in the setup files - see
the section in the code that compares F_num to the CETB_platform
enum

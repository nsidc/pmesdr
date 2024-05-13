#  Short Python script to calculate the start and end times for the TB_time
#  variable in a CETB file and wirte out the correct global attributes for
#  time_coverage_start and time_coverage_end
#
from datetime import timedelta, datetime
from netCDF4 import Dataset
import numpy as np
import sys

if len(sys.argv) < 2:
    print("Need a file argument\n")
    exit()

input_file_list = str(sys.argv[1])

with open(input_file_list) as fp:
    for line in fp:
        try:
            print(line.strip())
            ds=Dataset(line.strip(), 'a', 'NETCDF4')
            tb_time = ds.variables['TB_time'][:]

# Check to make sure there are some non-masked elements
# If the entire array is masked out then just close the file
            if not tb_time.mask.all():
    
                startmin = np.min(tb_time)
                endmin = np.max(tb_time)
                time_units = ds.variables['TB_time'].units
                dd = datetime.strptime(time_units, "minutes since %Y-%m-%d %H:%M:%S")
                startdd = dd + timedelta(minutes=np.int(startmin))
                enddd = dd + timedelta(minutes=np.int(endmin))
                start_string="%d-%02d-%02dT%02d:%02d:%02d.00Z" % (startdd.year, startdd.month, startdd.day, startdd.hour, startdd.minute, startdd.second)
                end_string="%d-%02d-%02dT%02d:%02d:%02d.00Z" % (enddd.year, enddd.month, enddd.day, enddd.hour, enddd.minute, enddd.second)
                ds.setncattr('time_coverage_start', start_string)
                ds.setncattr('time_coverage_end', end_string)

                ds.close()
        except OSError as exception:
            print(exception)

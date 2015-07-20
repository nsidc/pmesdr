import inspector
import re
import glob

atbd_data_dir = "/projects/PMESDR/vagrant/brodzik/ATBD_images"
#files = sorted( glob.glob( '/'.join([atbd_data_dir,'EASE2_?25*.nc'])) )
files = sorted( glob.glob( '/'.join([atbd_data_dir,'EASE2_T3*SIR.CSU*.nc'])) )
print len(files)

for file in files:
    m = re.match(r'.*EASE2_[NST](\d+)', file )
    res = m.groups()
    inspector.make_png( res[0], file )

print "Done."


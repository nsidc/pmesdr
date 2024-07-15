# Passive Microwave Earth Science Data Record (PMESDR) System

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Passive Microwave Earth Science Data Record (PMESDR) System](#passive-microwave-earth-science-data-record-pmesdr-system)
    - [Introduction](#introduction)
    - [License](#license)
    - [Requirements](#requirements)
    - [Installation ](#installation)
    - [Testing](#testing)
        - [Quick Regression](#quick-regression)
        - [Daily_Regression](#daily_regression)
    - [Development Cycle](#development-cycle)
        - [Releasing](#releasing)
    - [Software](#software)
        - [gsx](#gsx)
        - [meas_meta_make](#meas_meta_make)
        - [meas_meta_setup](#meas_meta_setup)
        - [meas_meta_sir](#meas_meta_sir)
        - [other software](#other-software)
    - [Operational Instructions](#operational-instructions)
    - [Development Notes](#development-notes)
        - [Adding a New Sensor](#adding-a-new-sensor)
        - [Changing Spatial Resolution](#changing-spatial-resolution)
        - [Known Issues](#known-issues)
    - [Data Products](#data-products)
        - [CETB Data Products](#cetb-data-products)
        - [SMAP CETB Data Products](#smap-cetb-data-products)
    - [References](#references)

<!-- markdown-toc end -->

## Introduction

This software repository contains source code to transform passive microwave
radiometer swath-format data to gridded format, using image reconstruction
methods developed by D. G. Long at Brigham Young University. The radiometer
version of Scatterometer Image Reconstruction (rSIR) ([Long and Brodzik,
2016][long2016]; Long et al., 2019; Long et al., 2023) leverages large footprint
overlaps and irregularly-spaced sampling locations to enhanced the spatial
resolution of the output grids. The PMESDR system is currently maintained and
operated at the National Snow and Ice Data Center (NSIDC) Distributed Active
Archive Center (DAAC) on hardware resources at the DAAC and at the University of
Colorado Research Computing (CURC) high performance computing facilities in
Boulder, CO.

A description of software methods and data set requirements definitions for the
project development is included in Brodzik et al., 2018. Data products are
produced in EASE-Grid 2.0 (Brodzik et al., 2012; 2014).  Algorithm theoretical
basis documents (ATBDs) are available for CETB Version 1 (Brodzik et al., 20xx)
data and CETB Version 2 (Brodzik et al., 20xx) data products.  Algorithm
theoretical basis documents (ATBDs) are available for SMAP CETB Version 2
(Brodzik et al., 20xx) data and SMAP CETB Version 3 (Brodzik et al., 20xx) data
products.

Copyright (C) 2014-2024 The Regents of the University of Colorado and Brigham
Young University.

## License

This project software is licensed under the GNU General Public License v3.0.
Please refer to the LICENSE.txt file for details.

## Requirements

The system requires the following:

`python`: Python is required to run the extended generic swath utility toconvert
various input swath data formats to gsx-formatted .nc files; python is also
required to execute some unit test comparisons and to perform regression
testing; Miniconda is required to build a conda environment; make targets are
included to build the conda environment with gsx and required packages for
running the system

`make`:	The `make` build utility is required to build the C components and run
regression tests.

C compiler: The system builds and has been tested with `gcc` and `icc` compilers
on systems at NSIDC (Ubuntu) and CURC (RedHat).

`bash`:	Operational scripts are written in the `bash` scripting language.

## Installation 

Install the the PMESDR system by cloning the repo:

``` bash
git clone git@github.com:nsidc/pmesdr
```

Always start working on the system by setting the system environment variables:

``` bash
cd pmesdr/src/prod
source ./set_pmesdr_environment.sh
```

This script will set required bash environment variables and is sensitive to
hostnames and available compilers on systems available at the University of
Colorado. On the CURC supercomputer, required software and packages will be
loaded. Users who need to build the system in a different environment will
need to edit the set_pmesdr_environment.sh script with settings for your local
environment and compiler.

Download and install
[Miniconda](https://docs.anaconda.com/miniconda/miniconda-install/).

Configure your conda installation channels to include `nsidc` and `conda-forge`:

``` bash
conda config --add channels conda-forge
conda config --add channels nsidc
```

The result from the --show channels command should return:

``` bash
$ conda config --show channels
channels:
  - nsidc
  - conda-forge
  - defaults
```

Create a conda environment with required python packages, including `gsx`:

``` bash
cd pmesdr/src/prod
source ./set_pmesdr_environment.sh
make conda-env
```

This will create a conda environment named ${PMESDR_CONDAENV} with the required packages to test and run the system.
		
Compile and install the system:
	
``` bash
cd pmesdr/src/prod
source ./set_pmesdr_environment.sh
make clean
make all
make install
```

This will build and install C executables `meas_meta_make`,	`meas_meta_setup`, and `meas_meta_sir` in the bin/ directory.
		
## Testing

Install the regression date repository with:

``` bash
cd pmesdr/src/prod
source ./set_pmesdr_environment.sh
make regression-setup
```

This will clone a git repository with regression data for comparison tests in a
directory named `pmesdr_regression_data` as a sibling to the directory where the
`pmesdr` clone is located. This step only needs to be done once and then again
only if the regression data changes. Data in the repository are organized in
directories by date. The regression date used in the tests is specified by the
value of `regression_yyyymmdd` that is defined in `set_pmesdr_environment.sh`.

The regression tests assume that a conda environment named ${PMESDR_CONDAENV}
has been built as described above. 

### Quick Regression

A "quick" regression, intended for quick development cycles, is fast, but only
executes tests for the Northern Hemisphere grid and a limited set of pre-defined
gsx inputs. It assumes that you have re-built the executables (make clean; make
all; make install) with whatever changes you may be testing. Run a quick
regression with:

``` bash
cd pmesdr/src/prod
source ./set_pmesdr_environment.sh
make quick-regression
```

This test runs in about a minute on a CURC compute node.

### Daily_Regression

A "daily" regression, intended for more rigorous testing, takes longer to run,
and executes tests for the N, S and T (Northern and Southern Hemisphere, and
Temperate Cylindrical Equal-area)  grids and a larger set of pre-defined
gsx inputs. It assumes that you have re-built the executables (make clean; make
all; make install) with whatever changes you may be testing. Run a daily
regression with:

``` bash
cd pmesdr/src/prod
source ./set_pmesdr_environment.sh
make daily-regression
```

This test runs in about five minutes on a CURC compute node. At NSIDC, it is
configured to run daily at midnight as part of continuous integration.

## Development Cycle

	1. Create a feature branch
	2. Create and test changes on the feature branch
		* 'make unit-test' to rebuild executables and run unit tests
		* 'make quick-regression' for fast regression
		* 'make daily-regression' for more comprehensive regression
	3. Push commits on the feature branch
	4. Create a Pull Request on GitHub
	5. When the feature PR is merged to the main branch, the patch version and
	tag on the main branch will be incremented

### Releasing

notes here about bumpversion for patches to main.
major-minor bumps need to be committed on main branch with "[skip actions]" in
commit message.

## Software

### gsx

### meas_meta_make

### meas_meta_setup

### meas_meta_sir

### other software

## Operational Instructions

## Development Notes

### Adding a New Sensor

### Changing Spatial Resolution

### Known Issues

## Data Products

The PMESDR system has been used to produce the following Calibrated,
Enhanced-Resolution Brightness Temperature (CETB) data sets:

### CETB Data Products

Brodzik, M. J., D. G. Long, M. A. Hardman, A. Paget, and R. Armstrong. 2016,
updated current year. MEaSUREs Calibrated Enhanced-Resolution Passive Microwave
Daily EASE-Grid 2.0 Brightness Temperature ESDR, Version 1. Boulder, Colorado
USA. NASA National Snow and Ice Data Center Distributed Active Archive
Center. [nsidc0630](https://nsidc.org/data/nsidc-0630/versions/1)
[10.5067/MEASURES/CRYOSPHERE/NSIDC-0630.001](https://doi.org/10.5067/MEASURES/CRYOSPHERE/NSIDC-0630.001).

### SMAP CETB Data Products

Brodzik, M. J., D. G. Long, M. A. Hardman. 2021, updated 2023. SMAP Radiometer
Twice-Daily rSIR-Enhanced EASE-Grid 2.0 Brightness
Temperatures. Version 2. Boulder, Colorado USA: NASA DAAC at the National Snow
and Ice Data
Center. [nsidc0738v2](https://nsidc.org/data/nsidc-0738/versions/2). [10.5067/YAMX52BXFL10](https://doi.org/10.5067/YAMX52BXFL10).

Brodzik, M. J., D. G. Long, M. A. Hardman. 2024, updated current year. SMAP
Radiometer Twice-Daily rSIR-Enhanced EASE-Grid 2.0 Brightness
Temperatures. Version 3. Boulder, Colorado USA: NASA DAAC at the National Snow
and Ice Data Center.
[nsidc0738v3](https://nsidc.org/data/nsidc-0738/versions/3). [10.5067/8OULQIU7ZPSX](https://doi.org/10.5067/8OULQIU7ZPSX).

## References

Brodzik, M. J., B. Billingsley, T. Haran, B. Raup, and M. H. Savoie. 2012.
EASE-Grid 2.0: Incremental but Significant Improvements for Earth-Gridded Data
Sets. ISPRS Int. J. Geo-Inf. 1,
32–45. [10.3390ijgi1010032](https://doi.org/10.3390/ijgi1010032).

Brodzik, M. J., B. Billingsley, T. Haran, B. Raup, and M. H. Savoie. 2014.
Correction: Brodzik, M.J., et al. EASE-Grid 2.0: Incremental but Significant
Improvements for Earth-Gridded Data Sets, ISPRS Int. J. Geo-Inf. 2012, 1,
32–45. ISPRS Int. J. Geo-Inf. 3,
1154–1156. [10.3390/ijgi3031154](https://doi.org/10.3390/ijgi3031154).

Brodzik, M. J. and D. G. Long. 2018. Calibrated Passive Microwave Daily
EASE-Grid 2.0 Brightness Temperature ESDR (CETB) Algorithm Theoretical Basis
Document, Version 1.0. Boulder, CO, USA. [10.5181/zenodo.7958456](https:
//doi.org/10.5281/zenodo.7958456).

Brodzik, M. J. and D. G. Long. 2024. Calibrated Passive Microwave Daily
EASE-Grid 2.0 Brightness Temperature ESDR (CETB) Algorithm Theoretical Basis
Document, Version 2.1. Boulder, CO, USA. [10.5281/zenodo.11626219](https:
//doi.org/10.5281/zenodo.11626219).

Brodzik, M. J., D. G. Long, and M. A. Hardman. 2018. Best Practices in Crafting
the Calibrated, Enhanced–Resolution Passive–Microwave EASE-Grid 2.0 Brightness
Temperature Earth System Data Record. Remote Sensing,
10(11), 2018. [10.3390/rs10111793](https://doi.org/10.3390/rs10111793).

Brodzik, M. J., D. G. Long, and M. A. Hardman. 2021. SMAP Twice–Daily
rSIR–Enhanced EASE-Grid 2.0 Brightness Temperatures Algorithm Theoretical Basis
Document, Version 2. Boulder, CO, USA. [10.5281/zenodo.11069045](https:
//doi.org/10.5281/zenodo.11069045).

Brodzik, M. J., D. G. Long, and M. A. Hardman. 2024. SMAP Twice–Daily
rSIR–Enhanced EASE-Grid 2.0 Brightness Temperatures Algorithm Theoretical Basis
Document, Version 3. Boulder, CO, USA. [10.5281/zenodo.11069054](https:
//doi.org/10.5281/zenodo.11069054).

[long2016]: Long, D. G. and M. J. Brodzik. 2016. Optimum Image Formation for Spaceborne Microwave Radiometer Products. IEEE Transactions on Geoscience and Remote Sensing, 54(5):2763–2779. [10.1109/TGRS.2015.2505677](https://doi.org/10.1109/TGRS.2015.2505677).

Long, D. G., M. J. Brodzik, and M. A. Hardman. 2019. Enhanced Resolution SMAP
Brightness Temperature Image Products. IEEE Transactions on Geoscience and
Remote Sensing,
1-13. [10.1109/TGRS.2018.2889427](https://doi.org/10.1109/TGRS.2018.2889427).

Long, D. G., M. J. Brodzik, and M. A. Hardman. 2023. Evaluating the Effective
Resolution of Enhanced Resolution SMAP Brightness Temperature Image
Products. Frontiers in Remote Sensing 4
(1073765), 16. [10.3389/frsen.2023.1073765](https://doi.
org/10.3389/frsen.2023.1073765).

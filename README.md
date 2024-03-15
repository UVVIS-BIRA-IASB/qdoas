# QDOAS

QDOAS is a cross-platform application based on the long experience of BIRA-IASB (Royal Belgian Institute for Space Aeronomy) in the development and improvement of algorithms for the retrieval of trace gases from UV-Visible spectral measurements (satellite, ground-based, mobile or aircraft-based instruments) using the DOAS (Differential Optical Absorption Spectroscopy).  This software evolves along with the measurement techniques in DOAS. It has been extensively validated in different intercomparison campaigns. The friendly user interface, flexibility and the robustness of the algorithms largely contribute to QDOAS success in the world.

QDOAS is now part of the [Atmospheric Toolbox](https://atmospherictoolbox.org).

Ready-for-use packages for windows, linux and MAC systems are available from [conda-forge](https://anaconda.org/conda-forge/qdoas), you can install them using the [Miniconda](https://docs.anaconda.com/free/miniconda/index.html) or [Anaconda](https://www.anaconda.com/download) package managers.

This readme.md file summarizes the main features and provides prerequisites and instructions to compile QDOAS.

## Main Features

### General features

* The main components of the graphical user interface (GUI) are organized in multi-page panels with a fixed arrangement and tab-switched access to the different pages;
* The application is based on a tree structure;
* Large amount of files can be processed in one shot;
* Support a large number of spectra file formats (for ground-based and satellites applications);
* On line help in HTML format

### Plot

* Visualization of spectra and the results in different tab pages;
* Possibility to set plot colour and style;
* Interactive plot mode (zooming, overlay of an existing ASCII file, possibility to fix the scaling of the plot, ...), activated by right clicking the title of the plot;
* Export of plot in different portable image formats (png, jpg)

### Analysis

* DOAS/intensity fitting modes;
* shift/stretch fully configurable for any spectral item (cross-section or spectrum);
* different filters available to apply on spectra and cross section (for example : Kaiser, gaussian, boxcar, Savitsky Golay, ...);
* possibility to define gaps within fitting intervals (e.g. to eliminate bad pixels);
* possibility to fit an instrumental offset;
* possibility to define several configurations of spectral windows under a project;
* automatic identification and removal of outliers in residuals (spike removal)

### Cross-sections Handling

* possibility to calculate differential absorption cross-sections (by orthogonalization or high-pass filtering);
* possibility to multiply cross-sections with wavelength dependent AMF;
* possibility to fix the column density of any selected species;
* possibility to convolve cross-sections in real time using a user defined slit function or the information on calibration and slit function provided by the wavelength calibration procedure;
* possibility to handle differences in resolutions between measured and control spectra;
* use of Pukite cross sections (preconvolved or calculated by the S/W);
* molecular ring correction to account for non linear effects in the spectral analysis windows

### Calibration And Slit Function Characterization

* wavelength calibration and instrumental slit function characterization using a non-linear least-squares (NLLS) fitting approach where measured intensities are fitted to a high resolution solar spectrum degraded to the resolution of the instrument. The fitting method (DOAS or intensity fitting) can be different from the method used in the analysis;
* possibility to correct for atmospheric absorption and Ring effect;
* supports different analytical line shapes;
* possibility to customize the calibration sub-windows;
* for satellites, use of a predefined radiance as reference

### Output

* The output is fully configurable by selecting individual items at the level of the project or at the level of analysis windows;
* ASCII and netCDF formats supported

### Tools

* convolution :
    - Convolution/Filtering tool,
    - standard and I0-corrected convolutions are supported;
    - possibility to create an effective slit function taking into account the (finite) resolution of the source spectrum (using a FT deconvolution method);
    - asymmetric line shapes, wavelength dependent slit functions;
* ring : calculates Ring effect cross-sections (Rotational Raman Scattering approach);
* usamp : generates undersampling cross-sections;
* doas_cl : powerful Command line tool for batch processing; a switch allows to change options that are in the xml config file (for example : the reference spectra)

## Compilation

### Prerequisites

The GUI is built on the Open-Source version of the Qt-5 toolkit. As a result, QDOAS is portable to Windows, Unix/Linux and Mac, and the user interface is effectively the same on all platforms.

Building QDOAS from sources requires :

* C++ compiler (g++ version 4.8.1 or higher is recommended),
* [Qt5](http://www.trolltech.com)
* [Qwt](http://sourceforge.net/projects/qwt)
* [CODA](https://atmospherictoolbox.org/coda/)
* [HDF4](https://support.hdfgroup.org/products/hdf4/)
* [HDF5](https://www.hdfgroup.org/downloads/hdf5/)
* [NetCDF](https://www.unidata.ucar.edu/software/netcdf/)
* [Boost](https://www.boost.org)
* [Eigen](https://eigen.tuxfamily.org)

### Compilation and installation

Use CMake

QDOAS comes in five independent executables or modules :

* **qdoas** : the user interface similar to the WinDOAS one;
* **convolution** : the convolution tool;
* **ring** : the Ring calculation tool;
* **usamp** : the undersampling calculation tool;
* **doas_cl** : a powerful command line tool that applies on qdoas, convolution, Ring and usamp configuration files.

Convolution, Ring and usamp tools manage their own configuration files and can be called either from the QDOAS user interface or from the system command line.


### Starting with QDOAS

If you are not familiar with QDOAS, just call qdoas to open the graphics user environment and follow the instructions in the "Quickstart" chapter of the [S/W User Manual](http://uv-vis.aeronomie.be/software/QDOAS/QDOAS_manual.pdf).

GOME2 applications requires that the CODA library is previously installed (the package can be downloaded from the [CODA GithHub](https://github.com/stcorp/coda/releases/latest) web site) and the `CODA_DEFINITION` environment variable is defined, pointing to the location where to find the codadef definitions of the supported data products.

## Authors

The following people contributed to the S/W :

* **Caroline FAYT, Jonas VLIETINCK, Thomas DANCKAERT and Michel VAN ROOZENDAEL** from BIRA-IASB
* **Sander NIEMEIJER, Ian PRICE** from S&T

In case of publication of results obtained using QDOAS, please mention the [QDOAS S/W user manual]((http://uv-vis.aeronomie.be/software/QDOAS/QDOAS_manual.pdf)) in the reference and the authors in the acknowledgements (see above).

## Acknowledgements

The authors would also like to acknowledge all people who have contributed directly or indirectly to the S/W user manual and/or to the improvement of the code.

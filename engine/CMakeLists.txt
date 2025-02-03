add_library(engine STATIC
  analyse.c
  analyse.h
  apex_read.cpp
  apex_read.h
  ascii-qdoas.c
  ascii.c
  bin_read.c
  bin_read.h
  bira-airborne-read.c
  ccd-read.c
  comdefs.h
  constants.h
  curfit.c
  curfit.h
  date_util.h
  date_util.cpp
  debug.c
  dir_iter.h
  doas.h
  engine.c
  engine.h
  engine_context.h
  engine_xsconv.c
  engine_xsconv.h
  erf.c
  erf.h
  error.c
  filter.c
  filter.h
  fit_properties.c
  fit_properties.h
  frm4doas_read.cpp
  frm4doas_read.h
  fvoigt.c
  gems_read.h
  gems_read.cpp
  gdp_bin_read.c
  gdp_bin_read.h
  gome1netcdf_read.cpp
  gome1netcdf_read.h
  gome2_read.c
  gome2_read.h
  kurucz.c
  kurucz.h
  linear_system.cpp
  linear_system.h
  matrix.c
  matrix.h
  matrix_netcdf_read.cpp
  matrix_netcdf_read.h
  memory.c
  mfc-read.c
  mfc-read.h
  mkzy-read.c
  moon.c
  moon.h
  netcdfwrapper.cpp
  netcdfwrapper.h
  noaa-read.c
  oceanoptics.c
  omi_read.c
  omi_read.h
  omiv4_read.cpp
  omiv4_read.h
  omps_read.cpp
  omps_read.h
  output.c
  output.h
  output_ascii.c
  output_common.h
  output_formats.h
  output_netcdf.cpp
  output_netcdf.h
  output_private.h
  radiance_ref.h
  radiance_ref.cpp
  ref_list.c
  ref_list.h
  resource.c
  resource.h
  ring.c
  saoz-read.c
  scia-read.c
  scia-read.h
  scia_common.c
  scia_common.h
  scia_defs.h
  scia_l1c.c
  scia_l1c.h
  scia_l1c_lib.c
  scia_l1c_lib.h
  spectral_range.c
  spectral_range.h
  spectrum_files.h
  spline.c
  spline.h
  stdfunc.c
  stdfunc.h
  svd.c
  svd.h
  tropomi.h
  tropomi_read.cpp
  tropomi_read.h
  uoft-read.c
  usamp.c
  usamp.h
  vector.c
  vector.h
  winfiles.c
  winfiles.h
  winsites.c
  winsites.h
  winsymb.c
  winsymb.h
  winthrd.c
  winthrd.h
  wvlen_det_pix.c
  xsconv.c
  xsconv.h
  zenithal.c
  zenithal.h
)

target_link_libraries(engine PRIVATE nelmin Boost::boost Eigen3::Eigen
  config
  ${HDF5_CXX_LIBRARIES}
  ${CODA_LIBRARY}
  ${NETCDF_LIBRARIES})
if (TARGET hdf-static)
  target_link_libraries(engine PRIVATE hdf-static mfhdf-static)
endif()
target_include_directories(engine PRIVATE ${netCDF_INCLUDE_DIR} ${CODA_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS})
target_include_directories(engine PUBLIC ../common ../mediator)

find_library(HDF4_MFHDF mfhdf)
if (HDF4_MFHDF)
target_link_libraries(engine PRIVATE ${HDF4_MFHDF})
endif (HDF4_MFHDF)

target_compile_features(engine PUBLIC cxx_std_17)
set_property(TARGET engine PROPERTY C_STANDARD 99)

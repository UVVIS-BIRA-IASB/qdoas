add_library(mediate STATIC
  mediate_xsconv_output_netcdf.cpp
  mediate_xsconv_output_netcdf.h
  mediate.c
  mediate.h
  mediate_analysis_window.c
  mediate_analysis_window.h
  mediate_common.c
  mediate_common.h
  mediate_convolution.c
  mediate_convolution.h
  mediate_general.h
  mediate_limits.h
  mediate_project.c
  mediate_project.h
  mediate_request.h
  mediate_response.cpp
  mediate_response.h
  mediate_ring.c
  mediate_ring.h
  mediate_types.h
  mediate_usamp.c
  mediate_usamp.h
  mediate_xsconv.c
  mediate_xsconv.h
)

target_compile_features(mediate PUBLIC cxx_std_11)
set_property(TARGET mediate PROPERTY C_STANDARD 99)
target_link_libraries(mediate PRIVATE
  engine
  ${netCDF_LIBRARIES}
)
target_include_directories(mediate PRIVATE ${netCDF_INCLUDE_DIR})
target_include_directories(mediate PUBLIC ../common ../engine)


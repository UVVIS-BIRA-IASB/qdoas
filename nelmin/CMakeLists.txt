add_library(nelmin STATIC
  nelmin.c
  include/nelmin.h)

target_include_directories(nelmin PRIVATE include)

set_target_properties(nelmin PROPERTIES
  PUBLIC_HEADER include/nelmin.h)

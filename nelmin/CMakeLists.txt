add_library(nelmin STATIC
  nelmin.c
  include/nelmin.h)

target_include_directories(nelmin PUBLIC include)

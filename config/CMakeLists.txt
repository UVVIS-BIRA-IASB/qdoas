add_library(config STATIC
  QdoasVersion.h
  )

configure_file(QdoasVersion.h.in QdoasVersion.h)
target_include_directories(config INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

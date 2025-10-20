// Helper module to manage output fields memory using C++ vectors.
//
// This is a simple way to get a dynamically growing array of output fields of the size we want.

#include <vector>


extern "C" {
  #include "output_fields.h"
}

using std::vector;

namespace {
  vector<struct output_field> output_data_calib_vec;
  vector<struct output_field> output_data_analysis_vec;
};

struct output_field *new_calib_field(void) {
  output_data_calib_vec.emplace_back();
  return &output_data_calib_vec.back();
}

struct output_field *output_data_calib(size_t i) {
  return &output_data_calib_vec[i];
}

size_t calib_num_fields(void) {
  return output_data_calib_vec.size();
}

struct output_field *new_analysis_field(void) {
  output_data_analysis_vec.emplace_back();
  return &output_data_analysis_vec.back();
}

struct output_field *output_data_analysis(size_t i) {
  return &output_data_analysis_vec[i];
}

size_t output_num_fields(void) {
  return output_data_analysis_vec.size();
}

void free_output_fields(void) {
  output_data_calib_vec.clear();
  output_data_analysis_vec.clear();
}

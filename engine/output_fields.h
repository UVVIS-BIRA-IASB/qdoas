#include "output_common.h"

struct output_field *new_calib_field(void);
struct output_field *new_analysis_field(void);

struct output_field *output_data_analysis(size_t i);
struct output_field *output_data_calib(size_t i);

size_t output_num_fields(void);
size_t calib_num_fields(void);

void free_output_fields(void);

#ifndef RADIANCE_REF_H
#define RADIANCE_REF_H

int radiance_ref_load(const char *filename, int pixel, double *lambda, double *spectrum, int n_wavel, int *use_row);

void radiance_ref_clear_cache(void);

#endif

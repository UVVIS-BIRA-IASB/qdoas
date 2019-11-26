#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ref_list.h"
#include "spline.h"
#include "vector.h"

// average_ref_spectra: interpolate all spectra in reflist onto lambda_target, and average
RC average_ref_spectra(const struct ref_list *reflist, const double *lambda_target, const int n_wavel, struct reference *average) {
  // reflist must not be NULL.
  assert(reflist != NULL);

  // buffers to store interpolated spectra & derivatives.
  double *tempspectrum = malloc(n_wavel*sizeof(*tempspectrum));
  double *derivs = malloc(n_wavel*sizeof(*derivs));

  average->n_wavel = n_wavel;
  for (int i=0; i<n_wavel; ++i) {
    average->spectrum[i] = 0.;
  }

  RC rc = ERROR_ID_NO;
  while (reflist != NULL) {
    rc = SPLINE_Deriv2(reflist->ref->lambda,reflist->ref->spectrum,derivs,n_wavel,__func__);
    if (rc)
      goto cleanup;

    SPLINE_Vector(reflist->ref->lambda,reflist->ref->spectrum,derivs,n_wavel,lambda_target,tempspectrum,n_wavel,SPLINE_CUBIC);
    ++average->n_spectra;

    for (int i=0; i<n_wavel; ++i) {
      average->spectrum[i]+=tempspectrum[i];
    }
    reflist = reflist->next;
  }

  for (int i=0; i<n_wavel; ++i) {
    average->spectrum[i] /= average->n_spectra;
  }
  VECTOR_NormalizeVector(average->spectrum-1,average->n_wavel, &average->norm,__func__);

 cleanup:
  free(tempspectrum);
  free(derivs);
  return rc;
}

void free_ref_list(struct ref_list *list, enum ref_list_free_mode how) {
  while (list != NULL) {
    struct ref_list *temp = list;
    list = list->next;
    if (how == FREE_DATA) {
      free(temp->ref->lambda);
      free(temp->ref->spectrum);
      free(temp->ref);
    }
    free(temp);
  }
}

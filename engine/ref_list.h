#ifndef REF_LIST_H
#define REF_LIST_H

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// Eartshine references spectrum
struct reference {
  double *spectrum;
  double shift, stretch, stretch2; // determined by alignreference
  double norm;
  size_t n_spectra; // number of spectra used in this reference
  size_t n_wavel;
};

// a single spectrum to be used in the earthshine reference:
struct ref_spectrum {
  double *lambda, *spectrum;
};

// linked list of candidate reference spectra
struct ref_list {
  struct ref_spectrum *ref;
  struct ref_list *next;
};

enum ref_list_free_mode {
  FREE_LIST_ONLY,
  FREE_DATA,
};

/** \brief Interpolate all spectra in the linked list reflist onto a
    common wavelength grid, average and normalize them.  The list
    *must not* be NULL. */
int average_ref_spectra(const struct ref_list *reflist, const double *lambda_target, const int n_wavel, struct reference *average);

/** \brief Free the linked list of reference spectra. When
    how==FREE_DATA, also free the reference spectra in the list. */
void free_ref_list(struct ref_list *list, enum ref_list_free_mode how);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif

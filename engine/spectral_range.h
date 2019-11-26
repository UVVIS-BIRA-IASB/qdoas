#ifndef SPECTRAL_RANGE_H
#define SPECTRAL_RANGE_H

/*! \file spectral_range.h \brief A linked list describing the valid
 * intervals of the spectrum.
 *  
 * Parts of the spectrum may be excluded for many reasons (not
 * relevant for the analysis, measurement error, ...).  spectral_range
 * describes the part of the spectrum which is to be used in the
 * analysis.
 *
 * The list of intervals is stored in the structure doas_spectrum.
 * Internally, this is a linked list of intervals (start, end).  To
 * work with a spectrum, initialize it using spectrum_new().  To free
 * the allocated memory when the structure is no longer needed, use
 * spectrum_destroy().
 *
 * The doas_iterator structure makes it easy to loop over all pixels
 * in a spectrum, skipping the excluded regions.
 *
 * Example of use:
 *
 * \code
 * doas_spectrum *my_spectrum = spectrum_new(); // initialize an empty spectrum
 * spectrum_append(my_spectrum, 200, 278); // add the interval (200, 278) to the list of valid regions
 * spectrum_append(my_spectrum, 280, 300); // my_spectrum now contains pixels 200-300, excluding 279.
 *
 * // loop over the spectrum 
 * doas_iterator my_iterator;
 * for(int i = iterator_start(&my_iterator, my_spectrum); i != ITERATOR_FINISHED; i = iterator_next(&my_iterator)) {
 *   do_something(i); // 279 will be skipped
 * }
 *
 * // free memory
 * spectrum_destroy(my_spectrum);
 * \endcode
 */

#include <stdbool.h>
#include <stdlib.h>

#define ITERATOR_FINISHED -1

struct doas_interval_private;
typedef struct doas_interval_private doas_interval;

struct doas_spectrum_private;

/*! \brief Structure maintaining the list of valid intervals of the spectrum. */
typedef struct doas_spectrum_private doas_spectrum;

/*! \brief A structure to iterate over a spectral_range.
 *
 * The iterator allows to create a loop over all pixels in a range,
 * \sa iterator_start(), iterator_next()
 */
typedef struct {
  doas_interval *current_interval;
  int current_pixel;
} doas_iterator;

#if defined(__cplusplus)
extern "C" {
#endif

/*! Removes a pixel from a spectrum.
 *
 * \param spectrum Pointer to the spectrum from which the pixel should be
 * removed.
 *
 * \param pixel Pixel to remove.
 *
 * \return True when the requested pixel was found in the spectrum.
 */
bool spectrum_remove_pixel(doas_spectrum *spectrum, int pixel);

/*! Adds a new interval at the end of a spectrum.
 * 
 * \param spectrum Existing spectrum the interval will be added to.
 * \param start, end Interval to be added.
 */
void spectrum_append(doas_spectrum *spectrum, int start, int end);

/*! Creates a new empty spectrum. */
doas_spectrum *spectrum_new(void);

/*! Creates a separate copy of an existing range.*/
doas_spectrum *spectrum_copy(const doas_spectrum *source);

/*! Cleans up memory 
 *
 * Traverses the list starting at \a range and frees all memory.
 * 
 */
void spectrum_destroy(doas_spectrum *spectrum);

/*! Returns the sum of the length of all intervals in the spectrum.*/
int spectrum_length(const doas_spectrum *spectrum);

/*! Returns the number of intervals in the spectrum.*/
int spectrum_num_windows(const doas_spectrum *spectrum);

/*! Returns the last pixel in the spectrum. */
int spectrum_end(const doas_spectrum *spectrum);

/*! Returns the first pixel of the spectrum */
int spectrum_start(const doas_spectrum *spectrum);

/*! Initializes an iterator to point at the first pixel of a range. 
 *
 * Prepares \a theiterator to loop over the pixels in \a
 * spectrum.  The first pixel is returned.
 *
 * \param theiterator Iterator to use.
 * \param spectrum The spectrum over which we want to loop.
 *
 * \return The first pixel in \a range.
 */
int iterator_start(doas_iterator *theiterator, const doas_spectrum *spectrum);

/*! \brief Returns the next pixel in the range of the iterator.
 *
 * Returns the next pixel in the range the iterator is currently
 * pointing to, and advances the internal pointer of the iterator to
 * the next pixel.  Returns ITERATOR_FINISHED if there are no
 * more pixels in the range.
 *
 * \param iterator Iterator to use.
 *
 * \return The next pixel in the range, or ITERATOR_FINISHED
 */
int iterator_next(doas_iterator *theiterator);

/*! \brief Compares two spectra for equality.*/
bool spectrum_isequal(const doas_spectrum *one, const doas_spectrum *two);

doas_interval *iterator_next_interval(doas_iterator *theiterator);

doas_interval *iterator_start_interval(doas_iterator *theiterator, const doas_spectrum *spectrum);

int interval_start(const doas_interval *interval);

int interval_end(const doas_interval *interval);

void spectrum_debug(doas_spectrum *spectrum);

#if defined(__cplusplus)
}
#endif

#endif

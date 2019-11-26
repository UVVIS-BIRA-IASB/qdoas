#include "spectral_range.h"
#include <stdio.h>


struct doas_interval_private {
  int start; /*!< Pixel where this range begins. */
  int end;   /*!< Pixel where this range ends.   */
  struct doas_interval_private *next; /*!< Pointer to next range in the spectrum.  NULL if this is the last range. */
};

/*! \brief position to add an interval */
typedef enum {BEFORE, AFTER } insert_position;

struct doas_spectrum_private {
  doas_interval *first;
};

doas_interval *interval_new(int start, int end);
doas_interval *insert_interval(doas_interval *theinterval, int start, int end, insert_position pos);

doas_spectrum *spectrum_new(void) {
  doas_spectrum *spectrum = malloc(sizeof(doas_spectrum));
  spectrum->first = NULL;
  return spectrum;
}

void spectrum_append(doas_spectrum *spectrum, int start, int end) {
  // look for the pointer of the last element of the current list
  doas_interval **last_pointer = &spectrum->first;
  while(*last_pointer != NULL) {
    last_pointer = &((*last_pointer)->next);
  }
  *last_pointer = interval_new(start, end); // make last element of the list point to new
}

bool spectrum_remove_pixel(doas_spectrum *spectrum, int pixel) {
  bool pixelfound = false;
  // find interval the pixel is in:
  doas_interval **previouspointer = &spectrum->first; // previouspointer will be changed when we have to remove an interval
  doas_interval *current = spectrum->first;

  while(current != NULL && !pixelfound) {
    if (current->start <=pixel && current->end >=pixel)
      pixelfound = true;
    else  {
      previouspointer = &(current->next);
      current = current->next;
    }
  }

  // remove the pixel from the interval.
  if(pixelfound) {
    if (pixel == current->start)
      {
        if (current->start != current->end) // check the interval is not 1 pixel long
	  current->start = pixel +1;
        else { // interval is one pixel, so we remove it
          *previouspointer = current->next; // point to the element after the current range
          free(current);
	}
      }
    else if (pixel == current->end)  // pixel was at the end of the range
      current->end = pixel-1;
    else { // pixel was somewhere in the middle of the interval
      insert_interval(current, pixel+1, current->end, AFTER); // insert new interval (pixel+1, end) after the current interval
      current->end = pixel -1; // shorten the previous interval
    }
  }
  return pixelfound;
}

void spectrum_destroy(doas_spectrum *spectrum) {
  while(spectrum->first != NULL) {
    doas_interval *next_interval = spectrum->first->next;
    free(spectrum->first);
    spectrum->first = next_interval;
  }
  free(spectrum);
}

doas_interval *interval_new(int start, int end) {
  doas_interval *the_interval = malloc(sizeof(doas_interval));
  the_interval->start = start;
  the_interval->end = end;
  the_interval->next = NULL;
  return the_interval;
}

doas_interval *insert_interval(doas_interval *theinterval, int start, int end, insert_position pos) { // add range before/after existing range
  doas_interval *new_interval = interval_new(start,end);
  if (pos == BEFORE) { // insert before theinterval
    new_interval->next = theinterval;
  } else if(theinterval != NULL) { // insert after theinterval
    new_interval->next = theinterval->next;
    theinterval->next = new_interval;
  } else { // theinterval is NULL
    new_interval->next = NULL;
  }
  return new_interval;
}

doas_spectrum *spectrum_copy(const doas_spectrum *source) {
  doas_spectrum *spectrum = spectrum_new();
  doas_interval **next_pointer  = &(spectrum->first); // pointer to the 'next' pointer of the last interval of the new spectrum
  doas_interval *current = source->first;
  while(current != NULL) {
    *next_pointer = interval_new(current->start, current->end);
    next_pointer = &(*next_pointer)->next;
    current = current->next;
  }
  return spectrum;
}

int spectrum_length(const doas_spectrum *spectrum) {
  int result = 0;
  doas_interval *current = spectrum->first;
  while(current != NULL) {
    result += current->end - current->start + 1;
    current = current->next;
  }
  return result;
}

int spectrum_num_windows(const doas_spectrum *spectrum) {
  int result = 0;
  doas_interval *current = spectrum->first;
  while(current != NULL) {
    result++;
    current = current->next;
  }
  return result;
}

int spectrum_start(const doas_spectrum *spectrum) {
  int result = -1;
  if(spectrum->first != NULL)
    result = spectrum->first->start;
  return result;
}

int spectrum_end(const doas_spectrum *spectrum) {
  int result = -1;
  doas_interval *current = spectrum->first;
  while(current != NULL) {
    result = current->end;
    current = current->next;
  }
  return result;
}

int iterator_start(doas_iterator *theiterator, const doas_spectrum *spectrum) {
  theiterator->current_interval = spectrum->first; // TODO avoid that current_range is NULL for empty spectrum?
  theiterator->current_pixel = theiterator->current_interval->start;
  return theiterator->current_pixel;
}

int iterator_next(doas_iterator *theiterator) {
  int result = ITERATOR_FINISHED;
  if(theiterator->current_pixel != theiterator->current_interval->end)
    result = ++(theiterator->current_pixel);
  else if (theiterator->current_interval->next != NULL) {
    theiterator->current_interval = theiterator->current_interval->next;
    result = theiterator->current_pixel = theiterator->current_interval->start;
  }
  return result;
}

doas_interval *iterator_next_interval(doas_iterator *theiterator) {
  doas_interval *result = NULL;
  if(theiterator->current_interval->next != NULL) {
    theiterator->current_interval = theiterator->current_interval->next;
    theiterator->current_pixel = theiterator->current_interval->start;
    result = theiterator->current_interval;
  }
  return result;
}

doas_interval *iterator_start_interval(doas_iterator *theiterator, const doas_spectrum *spectrum) {
  doas_interval *result = NULL;
  if(spectrum != NULL) {
    result = spectrum->first;
  }
  theiterator->current_interval = result;
  if (result != NULL)
    theiterator->current_pixel = result->start;
  return result;
}

int interval_start(const doas_interval *interval) {
  return interval->start;
}

int interval_end(const doas_interval *interval) {
  return interval->end;
}

bool spectrum_isequal(const doas_spectrum *one, const doas_spectrum *two) {
  bool isequal = true;
  doas_interval *interval1 = one->first;
  doas_interval *interval2 = two->first;
  while(isequal) {
    if(interval1 == NULL) {
      if (interval2 != NULL) // 1 is null, 2 is not
        isequal = false;
      else // 1 & 2 are null-> equal
        break;
    } else if (interval2 == NULL) // 1 is not null, 2 is
      isequal = false;
    else if(interval1->start != interval2->start ||  // both are not null, so we can compare
            interval1->end != interval2->end)
      isequal = false;
    else {
      interval1 = interval1->next;
      interval2 = interval2->next;
    }
  }
  return isequal;
}

void spectrum_debug(doas_spectrum *spectrum) {
  doas_interval *interval  = spectrum->first;
  while(interval != NULL) {
    printf("interval: %d - %d\n",interval->start, interval->end);
    interval = interval->next;
  }
}

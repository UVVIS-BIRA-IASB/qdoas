#ifndef TROPOMI_H
#define TROPOMI_H

#include "mediate_limits.h"

#define TROPOMI_BANDS                                   \
  EXPAND(UV1, "BAND1")                                  \
  EXPAND(UV2, "BAND2")                                  \
  EXPAND(UVIS1, "BAND3")                                                \
  EXPAND(UVIS2, "BAND4")                                                \
  EXPAND(NIR1, "BAND5")                                                 \
  EXPAND(NIR2, "BAND6")                                                 \
  EXPAND(SWIR1, "BAND7")                                                \
  EXPAND(SWIR2, "BAND8")

enum tropomiSpectralBand {
#define EXPAND(BAND, LABEL) BAND,
  TROPOMI_BANDS
#undef EXPAND
};

struct instrumental_tropomi {
  char calibrationFile[FILENAME_BUFFER_LENGTH];
  char instrFunctionFile[FILENAME_BUFFER_LENGTH];

   // directory where we want to look for reference spectra.  If
   // empty, QDOAS will look for orbits of the same day.
  char reference_orbit_dir[FILENAME_BUFFER_LENGTH];
  char trackSelection[TRACK_SELECTION_LENGTH];
  enum tropomiSpectralBand spectralBand;
};

#endif

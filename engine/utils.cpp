
#include <string>
#include <sstream>

extern "C" {
#include "utils.h"
#include "constants.h"
}

// Parse track_selection, a comma-separated list of rows "r" or ranges of rows "rstart-rstop".
// Row are counted starting from 1, until ANALYSE_swathSize.
RC parse_trackselection(const char *track_selection, bool *use_row) {
  const std::string tracks(track_selection);
  
  size_t i_start = 0;
  size_t i_end = 0;

  if ((tracks.empty())) {
    for (size_t i=0;i<MAX_SWATHSIZE;i++) {
      use_row[i]=true;
    }
    return ERROR_ID_NO;
  }

  for (size_t i=0;i<MAX_SWATHSIZE;i++) {
    use_row[i]=false;
  }

  bool error = false;
  while(i_end != std::string::npos) {
    i_end = tracks.find(",", i_start);
    std::istringstream iss(tracks.substr(i_start, i_end == std::string::npos ? i_end : i_end - i_start));
    i_start = 1 + i_end;
    int row_start = -1, row_end = -1;
    iss >> row_start;
    if (iss.fail() || row_start < 1 || row_start > MAX_SWATHSIZE) {
      error = true;
      break;
    }
    if (iss.eof()) { // we have a single row number:
      use_row[row_start -1] = true;
      continue;
    }
    iss.ignore(1,'-');
    if (iss.fail()) {
      error = true;
      break;
    }
    iss >> row_end;
    if (iss.fail() || !iss.eof() || row_end < 1 || row_end > MAX_SWATHSIZE || row_start > row_end) {
      error = true;
      break;
    }
    for (size_t i = row_start - 1; i <= row_end - 1; ++i) {
      use_row[i] = true;
    }
  }
  if (error) {
    std::stringstream err_msg;
    err_msg << "Invalid row selection \"" << track_selection << "\". "
      "Please provide a comma-separated list of row numbers \"i\" or ranges \"i-j\", with row numbers in the range 1 to " << MAX_SWATHSIZE;
      return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_ROWSELECTION, err_msg.str().c_str());
  }
  return ERROR_ID_NO;
}

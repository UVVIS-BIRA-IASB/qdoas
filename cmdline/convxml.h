#include <string>
#include <vector>

#include "comdefs.h"
#include "mediate_convolution.h"
#include "mediate_ring.h"

RC CONVXML_Parse(const std::vector<std::string> &xmlCommands,mediate_convolution_t *properties);
RC RINGXML_Parse(const std::vector<std::string> &xmlCommands,mediate_ring *properties);

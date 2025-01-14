#ifndef QDOASXML_H
#define QDOASXML_H

#include <string>
#include <vector>

#include "comdefs.h"
#include "CProjectConfigItem.h"

RC QDOASXML_Parse(std::vector<std::string> &xmlCommands,const CProjectConfigItem *p);

#endif

#ifndef QDOASXML_H
#define QDOASXML_H

#include <cstdio>
#include <cstring>

#include <iostream>
#include <string>

#include <QString>
#include <QList>
#include <QStringList>

#include "CProjectConfigItem.h"
#include "CEngineResponse.h"
#include "constants.h"
#include "comdefs.h"

RC QDOASXML_Parse(QList<QString> &xmlCommands,const CProjectConfigItem *p);

#endif


#include <cstdio>
#include <cstring>

#include <iostream>
#include <string>

#include <QString>
#include <QList>
#include <QStringList>

#include "mediate_convolution.h"
#include "mediate_ring.h"
#include "CEngineResponse.h"
#include "constants.h"
#include "comdefs.h"

RC CONVXML_Parse(QList<QString> &xmlCommands,mediate_convolution_t *properties);
RC RINGXML_Parse(QList<QString> &xmlCommands,mediate_ring *properties);

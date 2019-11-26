/* Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <iostream>
#include "CBatchEngineController.h"

#include "mediate_types.h"

#include "debugutil.h"

extern int verboseMode;


CBatchEngineController::CBatchEngineController() :
  CEngineController(),
  m_active(false)
{
}

void CBatchEngineController::notifyReadyToNavigateRecords(const QString &filename, int numberOfRecords)
{
  m_active = (numberOfRecords > 0);
}

void CBatchEngineController::notifyEndOfRecords(void)
{
  m_active = false;
}

void CBatchEngineController::notifyErrorMessages(int highestErrorLevel, const QList<CEngineError> &errorMessages)
{
  QList<CEngineError>::const_iterator it = errorMessages.begin();
  while (it != errorMessages.end())
   {
    switch (it->errorLevel())
     {
      case InformationEngineError:
        std::cout << "INFO:  ";
        break;
      case WarningEngineError:
        if (verboseMode)
         std::cout << "WARN:  ";
        break;
      case FatalEngineError:
        std::cout << "ERROR: ";
        break;
      default:
        std::cout << "???:   ";
     }

    if ((it->errorLevel()!=WarningEngineError) || verboseMode)
     std::cout << it->message().toStdString() << std::endl;
    ++it;
   }

  // abort if an error occurred
  if (highestErrorLevel == FatalEngineError)
    m_active = false;
}

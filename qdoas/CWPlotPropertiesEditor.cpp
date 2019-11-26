/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
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


#include <QGridLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QColorDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>

#include "CWPlotPropertiesEditor.h"
#include "CWPlotRegion.h"

#include "CHelpSystem.h"

#include "debugutil.h"

CWPlotPropertiesEditor::CWPlotPropertiesEditor(CWPlotRegion *plotRegion, QWidget *parent) :
  CWEditor(parent),
  m_plotRegion(plotRegion)
{

  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->setMargin(50);

  m_config = new CWPlotPropertiesConfig(m_plotRegion->properties(), this);
  mainLayout->addWidget(m_config, 0, 1);

  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(2, 1);
  mainLayout->setRowStretch(1, 1);

  // Update the caption and create a context tag 
  m_captionStr = "Edit Plot Properties";
  m_contextTag = "PlotProperties"; // only ever want one of these active at once

  notifyAcceptActionOk(true);
}

bool CWPlotPropertiesEditor::actionOk(void)
{
  // set the properties in the plot region.
  CPlotProperties prop = m_plotRegion->properties();

  m_config->apply(prop);

  m_plotRegion->setProperties(prop);

  return true;
}

void CWPlotPropertiesEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("plotconf", "Properties");
}


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


#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "CWPlotPropertiesDialog.h"

CWPlotPropertiesDialog::CWPlotPropertiesDialog(CPlotProperties &prop, QWidget *parent) :
  QDialog(parent),
  m_properties(prop)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_config = new CWPlotPropertiesConfig(m_properties, this);
  mainLayout->addWidget(m_config);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  mainLayout->addWidget(buttonBox);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void CWPlotPropertiesDialog::accept()
{
  m_config->apply(m_properties);

  QDialog::accept();
}


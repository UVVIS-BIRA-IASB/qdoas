/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

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


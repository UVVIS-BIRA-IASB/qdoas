/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>

#include "CWAboutDialog.h"
#include "QdoasVersion.h"

#include "debugutil.h"

CWAboutDialog::CWAboutDialog(QWidget *parent) :
  QDialog(parent, Qt::MSWindowsFixedSizeDialogHint|Qt::WindowTitleHint|Qt::WindowSystemMenuHint)
{
  setWindowTitle("About Qdoas");

  QPixmap qdoas(":/images/qdoas_atmospheric_toolbox_small.png");
  QPixmap logo(":/images/about_logo.png");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);

  QLabel *qdoasLabel = new QLabel(this);
  qdoasLabel->setPixmap(qdoas);
  mainLayout->addWidget(qdoasLabel, 1, Qt::AlignCenter);

  mainLayout->addSpacing(10);

  QLabel *mainLabel = new QLabel(cQdoasVersionString);
  QFont font = mainLabel->font();
  font.setPointSize(font.pointSize() + 2);
  font.setBold(true);
  mainLabel->setFont(font);
  mainLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(mainLabel, 0, Qt::AlignCenter);

  mainLayout->addSpacing(20);

  QString blurb("is copyright © 2007\n"
                "Belgian Institute for Space Aeronomy (BIRA-IASB)\n"
                "Avenue Circulaire 3,  1180 Uccle,  Belgium\n\nand\n\n"
                "Science [&] Technology BV,\nDelft, Netherlands.");

  QLabel *blurbLabel = new QLabel(blurb);
  blurbLabel->setAlignment(Qt::AlignCenter);

  mainLayout->addWidget(blurbLabel, 1, Qt::AlignCenter);

  mainLayout->addSpacing(10);

 QLabel *logoLabel = new QLabel(this);
 logoLabel->setPixmap(logo);
 mainLayout->addWidget(logoLabel, 1, Qt::AlignCenter);
 mainLayout->addSpacing(10);

  QPushButton *okButton = new QPushButton(QString("Ok"));
  okButton->setDefault(true);

  mainLayout->addWidget(okButton, 0, Qt::AlignCenter);

  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

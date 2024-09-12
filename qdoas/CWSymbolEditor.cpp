/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpressionValidator>

#include "CWSymbolEditor.h"
#include "CWorkSpace.h"
#include "CHelpSystem.h"

#include "mediate_limits.h"

#include "debugutil.h"

CWSymbolEditor::CWSymbolEditor(const QString &symbolName, const QString &description, QWidget *parent) :
  CWEditor(parent)
{
  bool modifying = !symbolName.isEmpty();

  QGridLayout *mainLayout = new QGridLayout(this);

  // row 0 - Name
  mainLayout->addWidget(new QLabel("Symbol Name", this), 0, 1);
  m_symbolName = new QLineEdit(this);
  m_symbolName->setMaxLength(SYMBOL_NAME_BUFFER_LENGTH - 1);

  // limit the possible input
  m_symbolName->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z][A-Za-z0-9()\\-_]*"), m_symbolName));

  if (modifying) {
    // can modify only the description - the enabled state of the symbolName edit is
    // used in the actionOk method to see if the action will create or modify the symbol
    m_symbolName->setText(symbolName);
    m_symbolName->setEnabled(false);
  }
  mainLayout->addWidget(m_symbolName, 0, 2);

  // row 1 - Description
  mainLayout->addWidget(new QLabel("Description", this), 1, 1);
  m_description = new QLineEdit(this);
  m_description->setMaxLength(SYMBOL_DESCR_BUFFER_LENGTH - 1);
  if (!description.isEmpty())
    m_description->setText(description);

  mainLayout->addWidget(m_description, 1, 2);

  mainLayout->setColumnStretch(0, 1);
  mainLayout->setColumnStretch(2, 2);
  mainLayout->setColumnStretch(3, 1);

  mainLayout->setRowStretch(2, 1);

  // Update the caption and create a context tag
  if (modifying) {
    m_captionStr = "Edit symbol description";
    m_contextTag = "Edit Symbol : " + symbolName;
  }
  else {
    m_captionStr = "Create new symbol";
    m_contextTag = "New Symbol"; // only ever want one of these active at once
  }

  connect(m_symbolName, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotNameChanged(const QString &)));
  connect(m_symbolName, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));
  connect(m_description, SIGNAL(returnPressed()),
          this, SLOT(slotReturnPressed()));

  notifyAcceptActionOk(modifying);
}

bool CWSymbolEditor::actionOk(void)
{
  if (m_symbolName->isEnabled()) {
    // new symbol
    // must have a non-empty symbol name
    bool ok = CWorkSpace::instance()->createSymbol(m_symbolName->text(), m_description->text());

    if (!ok) {
      notifyAcceptActionOk(false);
      // feed back ...
      QMessageBox::information(this, "New Symbol", "This symbol is already defined");
    }

    return ok;
  }

  // modify
  return CWorkSpace::instance()->modifySymbol(m_symbolName->text(), m_description->text());
}

void CWSymbolEditor::actionHelp(void)
{
  CHelpSystem::showHelpTopic("GUI", "GUI_Symbols");
}

void CWSymbolEditor::takeFocus(void)
{
  m_symbolName->setFocus(Qt::OtherFocusReason);
}

void CWSymbolEditor::slotNameChanged(const QString &text)
{
  notifyAcceptActionOk(!text.isEmpty() && !text.contains(';') && text != "Spectrum");
}

void CWSymbolEditor::slotReturnPressed()
{
  shortcutActionOk();
}

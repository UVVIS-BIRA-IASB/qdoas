/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <QMenu>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QAbstractItemView>
#include <QAction>
#include <QMenu>
#include <QModelIndex>

#include "CWAnalysisWindowDoasTables.h"
#include "CWorkSpace.h"
#include "CPreferences.h"

#include "constants.h"
#include "visual_c_compat.h"
#include "debugutil.h"
#include <stdio.h>

//------------------------------------------------------------

static const int cStandardRowHeight = 24;

CWMoleculesDiffOrthoCombo::CWMoleculesDiffOrthoCombo(const QString &excludedSymbol, const QStringList &symbols, QWidget *parent) :
  CDoasTableColumnComboBox(ANLYS_COMBO_DIFF_ORTHO,excludedSymbol,parent),
  m_symbols(symbols),
  m_menu(NULL)
{
     // m_ComboBox = new QComboBox(this);
     setMinimumWidth(180);

     addItem("None");
     addItem("Differential XS");
     addItem("Orthogonalize to ...");
     addItem("Subtract from ...");
}

CMoleculeDoasTableColumnDiffOrtho::CMoleculeDoasTableColumnDiffOrtho(const QString &label, CDoasTable *owner, int columnWidth) :
  CDoasTableColumn(label, owner, columnWidth)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);
}

QVariant CMoleculeDoasTableColumnDiffOrtho::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QComboBox *tmp = dynamic_cast<const QComboBox*>(p);
    if (tmp)
     return QVariant(tmp->currentText());
  }

  return QVariant();
}

QWidget* CMoleculeDoasTableColumnDiffOrtho::createCellWidget(const QVariant &cellData)
{
  // this is called when adding a new row

  CWMoleculesDoasTable *p = dynamic_cast<CWMoleculesDoasTable*>(owner());
  assert (p != NULL);

  const QStringList &symbolList = p->symbolList();
  QString excludedSymbol = symbolList.value(rowCount()); // the last entry

             // !!!!   should create a list of symbols involved in subtraction operation

  CWMoleculesDiffOrthoCombo *tmp = new CWMoleculesDiffOrthoCombo(excludedSymbol, symbolList);

  tmp->initialSelection(cellData.toString());

  connect(tmp, SIGNAL(currentIndexChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
      this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));
  connect(p, SIGNAL(signalSymbolListChanged(const QStringList &)),
      tmp, SLOT(slotSymbolListChanged(const QStringList &)));

  return tmp;
}
// -------------------------------

CWMoleculesCorrectionCombo::CWMoleculesCorrectionCombo(const QString &excludedSymbol, const QStringList &symbols, QWidget *parent) :
  CDoasTableColumnComboBox(ANLYS_COMBO_CORRECTION,excludedSymbol,parent),
  m_symbols(symbols),
  m_menu(NULL)
{
     // m_ComboBox = new QComboBox(this);
     setMinimumWidth(220);

     addItem("None");
     addItem("Slope");
     addItem("Pukite");
     addItem("Mol. ring ...");
     addItem("Slope+Mol. ring ...");
}

CMoleculeDoasTableColumnCorrection::CMoleculeDoasTableColumnCorrection(const QString &label, CDoasTable *owner, int columnWidth) :
  CDoasTableColumn(label, owner, columnWidth)
{
  setViewportBackgroundColour(QColor(0xFFFFFFFF));
  setCellBorders(1,1);
}

QVariant CMoleculeDoasTableColumnCorrection::getCellData(int rowIndex) const
{
  const QWidget *p = getWidget(rowIndex);
  if (p) {
    const QComboBox *tmp = dynamic_cast<const QComboBox*>(p);
    if (tmp)
     return QVariant(tmp->currentText());
  }

  return QVariant();
}

QWidget* CMoleculeDoasTableColumnCorrection::createCellWidget(const QVariant &cellData)
{
  // this is called when adding a new row

  CWMoleculesDoasTable *p = dynamic_cast<CWMoleculesDoasTable*>(owner());
  assert (p != NULL);

  const QStringList &symbolList = p->symbolList();
  QString excludedSymbol = symbolList.value(rowCount()); // the last entry

             // !!!!   should create a list of symbols involved in subtraction operation

  CWMoleculesCorrectionCombo *tmp = new CWMoleculesCorrectionCombo(excludedSymbol, symbolList);

  tmp->initialSelection(cellData.toString());

  connect(tmp, SIGNAL(currentIndexChanged(const QString&)), tmp, SLOT(slotTextChanged(const QString&)));
  connect(tmp, SIGNAL(signalTextChanged(const QWidget*,const QVariant&)),
      this, SLOT(slotCellDataChanged(const QWidget*,const QVariant&)));
  connect(p, SIGNAL(signalSymbolListChanged(const QStringList &)),
      tmp, SLOT(slotSymbolListChanged(const QStringList &)));

  return tmp;
}




CWMoleculesDoasTable::CWMoleculesDoasTable(const QString &label, int columnWidth,
                       int headerHeight, QWidget *parent) :
  CDoasTable(label, headerHeight, parent),
  m_selectedRow(-1)
{
  CMoleculeDoasTableColumnDiffOrtho *tmp = new CMoleculeDoasTableColumnDiffOrtho("Subtract/Orthog", this, 180);
  addColumn(tmp);                            // columnIndex = 0

  QStringList comboItems;
  comboItems << "None" << "Interpolate" << "Convolve Std" << "Convolve Io" << "Convolve Ring";
  createColumnCombo("Interp/Conv", 120, comboItems);

  comboItems.clear();
  comboItems << "None" << "SZA only" << "Climatology" << "Wavelength";

  createColumnCombo("AMF", 120, comboItems); // columnIndex = 2

  CMoleculeDoasTableColumnCorrection *tmp2 = new CMoleculeDoasTableColumnCorrection("Correction", this, 200);
  addColumn(tmp2);                            // columnIndex = 0

  createColumnCheck("Fit disp.", 60);
  createColumnCheck("Filter", 60);           // columnIndex = 4
  createColumnCheck("Cons. SCD", 60);
  createColumnCheck("SCD fit", 60);           // columnIndex = 6
  createColumnEdit("SCD Init", 80);
  createColumnEdit("SCD Delta", 80);          // columnIndex = 8
  createColumnEdit("SCD Io", 80);
  createColumnEdit("SCD min", 80);           // columnIndex = 10
  createColumnEdit("SCD max", 80);
}

void CWMoleculesDoasTable::populate(const cross_section_list_t *data)
{
  int row = 0;
  const struct anlyswin_cross_section *d = &(data->crossSection[0]);
  char str[100];

  while (row < data->nCrossSection) {
    QList<QVariant> initialValues;

    strcpy(str,d->orthogonal);

    if (d->subtractFlag)
     {
      if (!strlen(d->subtract))
       strcpy(str,"Subtract from ...");
      else if (!strncasecmp(d->subtract,"Subtract from",strlen("subtract from")))
       strcpy(str,d->subtract);
      else
       sprintf(str,"Subtract from %s",d->subtract);
     }
    else if (!strlen(d->orthogonal))
     strcpy(str,"Orthogonalize to ...");
    else if (strncasecmp(d->orthogonal,"None",4) && strncasecmp(d->orthogonal,"Differential XS",15))
     sprintf(str,"Orthogonalize to %s",d->orthogonal);
    else
     strcpy(str,d->orthogonal);

    initialValues.push_back(QString(str));
    initialValues.push_back(mapCrossTypeToComboString(d->crossType));
    initialValues.push_back(mapAmfTypeToComboString(d->amfType));

    if (d->correctionType==ANLYS_CORRECTION_TYPE_MOLECULAR_RING)
     {
      if (!strlen(d->molecularRing))
       strcpy(str,"Mol. ring ...");
      else
       sprintf(str,"Mol. ring (%s)",d->molecularRing);

      initialValues.push_back(QString(str));
     }
    else if (d->correctionType==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE)
     {
      if (!strlen(d->molecularRing))
       strcpy(str,"Slope+Mol. ring ...");
      else
       sprintf(str,"Slope+Mol. ring (%s)",d->molecularRing);

      initialValues.push_back(QString(str));
     }
    else
     initialValues.push_back(mapCorrectionTypeToComboString(d->correctionType));

    initialValues.push_back(d->requireFit);
    initialValues.push_back(d->requireFilter);
    initialValues.push_back(d->constrainedCc);
    initialValues.push_back(d->requireCcFit);
    initialValues.push_back(d->initialCc);
    initialValues.push_back(d->deltaCc);
    initialValues.push_back(d->ccIo);
    initialValues.push_back(d->ccMin);
    initialValues.push_back(d->ccMax);

    // add the row (with filenames)
    addRow(cStandardRowHeight, QString(d->symbol), initialValues,
       QString(d->crossSectionFile), QString(d->amfFile));

    ++d; // iterate over the array
    ++row;
  }
}

bool CWMoleculesDoasTable::apply(cross_section_list_t *data) const
{
  int row;
  int col;
  struct anlyswin_cross_section *d = &(data->crossSection[0]);
  QStringList subtractFrom;
  QString ortho,subtract;
  QString text;
  bool brc;     // Boolean return code

  brc=true;

  data->nCrossSection = rowCount();

  // Make a list with symbols used in subtraction operations to detect subtractions in cascade

  for (row=0;row < data->nCrossSection;row++)
   {
    QList<QVariant> state = getCellData(row);
    text=QString(state.at(0).toString());
    if (text.startsWith("Subtract from"))
     subtractFrom << m_symbols.at(row).toLocal8Bit().data();
   }

  for (row=0;row < data->nCrossSection;row++) {

    QList<QVariant> state = getCellData(row);

    // certain that the length of the symbol is below the limit ...
    strcpy(d->symbol, m_symbols.at(row).toLocal8Bit().data());

    if (m_csFilename.at(row).length() < (int)sizeof(d->crossSectionFile))
      strcpy(d->crossSectionFile, m_csFilename.at(row).toLocal8Bit().data());
    else
      *(d->crossSectionFile) = '\0';

    col=0;

    text=QString(state.at(col++).toString());
    d->subtractFlag=false;

    if (text.startsWith("Orthogonalize to ...") || text.startsWith("Subtract from ..."))
     {
      QMessageBox::warning((QWidget*)this, "No cross section specified", "\"Orthogonalize to\" and \"Subtract from\" options need that a cross section is specified.  Option replaced by None.");
      strcpy(d->orthogonal,"None");
      brc=false;
     }
    else if ((text=="None") || (text=="Differential XS"))
     strcpy(d->orthogonal,text.toLatin1().data());
    else
     {
      QStringList tokens = text.split( " " );
      QString xs = tokens.value( tokens.length() - 1 );

      if (((mapComboStringToCorrectionType(state.at(3).toString())==ANLYS_CORRECTION_TYPE_SLOPE) ||
           (mapComboStringToCorrectionType(state.at(3).toString())==ANLYS_CORRECTION_TYPE_PUKITE)) &&
          (text.startsWith("Orthogonalize to") || text.startsWith("Subtract from")))
       {
        QMessageBox::warning((QWidget*)this,"Incompatible options", QString::fromStdString(d->symbol)+QString::fromStdString(" \"Orthogonalize to/Subtract from\" options not compatible with Pukite correction"));
        brc=false;
       }

      else if (text.startsWith("Orthogonalize to"))
       strcpy(d->orthogonal,xs.toLatin1().data());
      else if (text.startsWith("Subtract from"))
       {
        d->subtractFlag=true;
        strcpy(d->subtract,xs.toLatin1().data());

        if (subtractFrom.contains(xs.toLatin1().data()))
         {
          QMessageBox::warning((QWidget*)this, "Subtractions in cascade not allowed",QString::fromStdString("Symbol ")+QString::fromStdString(xs.toLatin1().data())+QString::fromStdString(" already used in a subtraction operation"));
          brc=false;
         }
       }
     }

    d->crossType = mapComboStringToCrossType(state.at(col++).toString());
    d->amfType = mapComboStringToAmfType(state.at(col++).toString());

    if (m_amfFilename.at(row).length() < (int)sizeof(d->amfFile))
      strcpy(d->amfFile, m_amfFilename.at(row).toLocal8Bit().data());
    else
      *(d->amfFile) = '\0';

    int correctionIndex=col++;
    text=QString(state.at(correctionIndex).toString());

    if (text.startsWith("Mol. ring ..."))
     {
      QMessageBox::warning((QWidget*)this, "No cross section specified", "\"Mol. ring\" option need that a cross section is specified.  Option replaced by None.");
      d->correctionType=ANLYS_CORRECTION_TYPE_NONE;
      strcpy(d->molecularRing,"None");
      brc=false;
     }
    else if (text.startsWith("Slope+Mol. ring ..."))
     {
      QMessageBox::warning((QWidget*)this, "No cross section specified", "\"Slope + Molecular ring\" option need that a cross section is specified.  Option replaced by None.");
      d->correctionType=ANLYS_CORRECTION_TYPE_NONE;
      strcpy(d->molecularRing,"None");
      brc=false;
     }
    else if ((text=="None") || (text=="Slope") || (text=="Pukite"))
     d->correctionType = mapComboStringToCorrectionType(state.at(correctionIndex).toString());
    else if (text.startsWith("Mol. ring"))
     {
      QStringList tokens = text.split( " " );
      QString xs = tokens.value( tokens.length() - 1 );

      d->correctionType=ANLYS_CORRECTION_TYPE_MOLECULAR_RING;
      sscanf(xs.toLatin1().data(),"(%[^)])",d->molecularRing);
     }
    else if (text.startsWith("Slope+Mol. ring"))
     {
      QStringList tokens = text.split( " " );
      QString xs = tokens.value( tokens.length() - 1 );

      d->correctionType=ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE;
      sscanf(xs.toLatin1().data(),"(%[^)])",d->molecularRing);
     }

    d->requireFit = state.at(col++).toBool() ? 1 : 0;
    d->requireFilter = state.at(col++).toBool() ? 1 : 0;
    d->constrainedCc = state.at(col++).toBool() ? 1 : 0;
    d->requireCcFit = state.at(col++).toBool() ? 1 : 0;
    d->initialCc = state.at(col++).toDouble();
    d->deltaCc = state.at(col++).toDouble();
    d->ccIo = state.at(col++).toDouble();
    d->ccMin = state.at(col++).toDouble();
    d->ccMax = state.at(col++).toDouble();

    // if ((d->crossType==ANLYS_CROSS_ACTION_CONVOLUTE_RING) && ((d->correctionType==ANLYS_CORRECTION_TYPE_SLOPE) || (d->correctionType==ANLYS_CORRECTION_TYPE_PUKITE)))
    //  {
    //   QMessageBox::warning((QWidget*)this, "Incompatible options","Convolve Ring and Slope/Pukite corrections are not compatible");
    //   brc=false;
    //  }

    ++d;
  }

 // Return

 return brc;
}

void CWMoleculesDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData,
                  const QString &csFilename, const QString &amfFilename)
{
  addRow(height, label, cellData);

  // replace the values for the filenames ... at the tail of the list ...
  if (!csFilename.isEmpty()) {
    m_csFilename.pop_back();
    m_csFilename << csFilename;
  }

  if (!amfFilename.isEmpty()) {
    m_amfFilename.pop_back();
    m_amfFilename << amfFilename;
  }
}

void CWMoleculesDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData)
{
  // update the symbol list, rowlocks list and signal ...
  m_symbols << label;
  m_rowLocks << -1; // this new row does not lock any other row

  m_csFilename << QString();  // add an empty string ...
  m_amfFilename << QString(); // add an empty string ...

  // really create the new row ...
  CDoasTable::addRow(height, label, cellData);

  emit signalSymbolListChanged(m_symbols);

  // is the symbol the name of a previous analysis window ??
  // This is really awkward .... TODO
}

void CWMoleculesDoasTable::removeRow(int rowIndex)
{
  // removal is only permitted if the row is not locked by another row
  if (rowIndex >= 0 && rowIndex < rowCount() && !isRowLocked(rowIndex)) {

    m_csFilename.removeAt(rowIndex);
    m_amfFilename.removeAt(rowIndex);

    m_rowLocks.removeAt(rowIndex);
    m_symbols.removeAt(rowIndex);

    // really remove the row ...
    CDoasTable::removeRow(rowIndex);

    // notify the remaining diff/ortho combos that the symbol list has changed
    emit signalSymbolListChanged(m_symbols);
  }
}

bool CWMoleculesDoasTable::isRowLocked(int rowIndex) const
{
  // assumed rowIndex is within the valid range ...

  // a row is locked if ...
  //    a) There is an internal lock, which means another row references the symbol for rowIndex.
  //    b) There is an external lock, which means that an external entity is holding a lock on the symbol.

  // internal lock ?
  if (m_rowLocks.indexOf(rowIndex) != -1)
    return true;

  // at least one external lock on the symbol ?
  return (m_symbolLocks.find(m_symbols.at(rowIndex)) != m_symbolLocks.end());
}

QString CWMoleculesDoasTable::mapCrossTypeToComboString(int type)
{
  switch (type) {
  case ANLYS_CROSS_ACTION_INTERPOLATE: return QString("Interpolate"); break;
  case ANLYS_CROSS_ACTION_CONVOLUTE: return QString("Convolve Std"); break;
  case ANLYS_CROSS_ACTION_CONVOLUTE_I0: return QString("Convolve Io"); break;
  case ANLYS_CROSS_ACTION_CONVOLUTE_RING: return QString("Convolve Ring"); break;
  }

  return QString("None");
}

int CWMoleculesDoasTable::mapComboStringToCrossType(const QString &str)
{
  if (str == "Interpolate") return ANLYS_CROSS_ACTION_INTERPOLATE;
  if (str == "Convolve Std") return ANLYS_CROSS_ACTION_CONVOLUTE;
  if (str == "Convolve Io") return ANLYS_CROSS_ACTION_CONVOLUTE_I0;
  if (str == "Convolve Ring") return ANLYS_CROSS_ACTION_CONVOLUTE_RING;

  return ANLYS_CROSS_ACTION_NOTHING;
}

QString CWMoleculesDoasTable::mapAmfTypeToComboString(int type)
{
  switch (type) {
  case ANLYS_AMF_TYPE_SZA: return QString("SZA only"); break;
  case ANLYS_AMF_TYPE_CLIMATOLOGY: return QString("Climatology"); break;
  }

  return QString("None");
}

int CWMoleculesDoasTable::mapComboStringToAmfType(const QString &str)
{
  if (str == "SZA only") return ANLYS_AMF_TYPE_SZA;
  if (str == "Climatology") return ANLYS_AMF_TYPE_CLIMATOLOGY;
  if (str == "Wavelength") return ANLYS_AMF_TYPE_WAVELENGTH;

  return ANLYS_AMF_TYPE_NONE;
}

QString CWMoleculesDoasTable::mapCorrectionTypeToComboString(int type)
{
  switch (type) {
  case ANLYS_CORRECTION_TYPE_SLOPE: return QString("Slope"); break;
  case ANLYS_CORRECTION_TYPE_PUKITE: return QString("Pukite"); break;
  case ANLYS_CORRECTION_TYPE_MOLECULAR_RING: return QString("Mol. ring ..."); break;
  case ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE: return QString("Slope/Mol. ring ..."); break;
  }

  return QString("None");
}

int CWMoleculesDoasTable::mapComboStringToCorrectionType(const QString &str)
{
     if (str == "Slope") return ANLYS_CORRECTION_TYPE_SLOPE;
  if (str == "Pukite") return ANLYS_CORRECTION_TYPE_PUKITE;
  if (str == "Mol. ring ...") return ANLYS_CORRECTION_TYPE_MOLECULAR_RING;
  if (str == "Slope/Mol. ring ...") return ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE;

  return ANLYS_CORRECTION_TYPE_NONE;
}

void CWMoleculesDoasTable::slotLockSymbol(const QString &symbol, const QObject *holder)
{
  std::pair<symlockmap_t::const_iterator,symlockmap_t::const_iterator> pair =
    m_symbolLocks.equal_range(symbol);

  symlockmap_t::const_iterator it = pair.first;
  while (it != pair.second) {
    if (it->second == holder) break; // exists already
    ++it;
  }
  if (it == pair.second) {
    // does not exist already ...
    m_symbolLocks.insert(symlockmap_t::value_type(symbol, holder));
  }
}

void CWMoleculesDoasTable::slotUnlockSymbol(const QString &symbol, const QObject *holder)
{
  std::pair<symlockmap_t::iterator,symlockmap_t::iterator> pair =
    m_symbolLocks.equal_range(symbol);

  symlockmap_t::iterator it = pair.first;
  while (it != pair.second) {
    if (it->second == holder) {
      m_symbolLocks.erase(it);
      return; // done
    }
    ++it;
  }
}

const QStringList& CWMoleculesDoasTable::symbolList(void) const
{
  return m_symbols;
}

void CWMoleculesDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
  // TRACE("Changed : " << row << "," << column << " : " << cellData.toString().toStdString());

  if (column == 0) {
    // is the selection for row the symbol of another row - if so that symbol is locked.
    m_rowLocks.replace(row, m_symbols.indexOf(cellData.toString())); // sets/releases/clears internal locks
  }
  else if (column == 1) {
    setCellEnabled(row, 10, (cellData.toString() == "Convolve Io"));
  }
  else if (column == 6) {
    // contrained CC
    bool enable = !cellData.toBool();

    setCellEnabled(row, 7, enable);
    setCellEnabled(row, 8, enable);
    setCellEnabled(row, 9, enable);
  }
}

void CWMoleculesDoasTable::contextMenuEvent(QContextMenuEvent *e)
{
  const int amfColumn = 2;

  // create a popup menu
  QMenu menu;

  menu.addAction("Insert", this, SLOT(slotInsertRow()));

  m_selectedRow = rowIndexAtPosition(e->y());

  QAction *removeAction = menu.addAction("Remove", this, SLOT(slotRemoveRow()));
  removeAction->setEnabled(m_selectedRow != -1 && !isRowLocked(m_selectedRow));

  menu.addAction("XS Filename", this, SLOT(slotChangeCrossSectionFileName()));

  // AMF filename ... only of enabled and value != NULL
  if (isColumnEnabled(amfColumn)) {
    menu.addSeparator();
    QAction *amfAction = menu.addAction("AMF Filename", this, SLOT(slotAmfFileName()));
    amfAction->setEnabled(m_selectedRow != -1 && getCellData(m_selectedRow, amfColumn).toString() != "None");
  }

  menu.exec(e->globalPos()); // a slot will do the rest
}

void CWMoleculesDoasTable::slotInsertRow()
{
  auto allSymbols = CWorkSpace::instance()->symbolList();
  QStringList freeSymbols;
  QString filter;
  int index;

  // build a list of free symbol and a file filter string
  auto it = allSymbols.begin();
  while (it != allSymbols.end()) {
    QString symbol_name= QString::fromStdString(*it);
    index = m_symbols.indexOf(symbol_name);
    if (index == -1) {
      // a free symbol ...
      freeSymbols << symbol_name;

      // if the name of the symbol contains parenthesis (for example, slope and pukite), the selection is made on the symbol within parenthesis
      // Use only Slope* and Pukite* filters

      if (symbol_name.startsWith("Slope") || symbol_name.startsWith("Pukite"))
       {
        if (symbol_name.startsWith("Slope"))
         filter.append(symbol_name).append(" (Slope*.*);;");
        if (symbol_name.startsWith("Pukite"))
         filter.append(symbol_name).append(" (Pukite*.*);;");
       }
      else
       filter.append(symbol_name).append(" (").append(symbol_name).append("_*.*);;");
    }
    ++it;
  }

  // check that the selected file is appropriate ...
  if (!filter.isEmpty()) {
    filter.append("All files (*.*)");

    CPreferences *prefs = CPreferences::instance();

    QString fileName = QFileDialog::getOpenFileName(this, "Select Cross Section File",
                                                    prefs->directoryName("CrossSection"), filter);

    if (!fileName.isEmpty() && fileName.length() < FILENAME_BUFFER_LENGTH) {

      prefs->setDirectoryNameGivenFile("CrossSection", fileName);

      // need to compare 'symbol_' with the start of the basename of the file ...
      QString baseName = CPreferences::baseName(fileName);

      // the start of the filename MUST match a free symbol ...
      auto it = freeSymbols.begin();
      while (it != freeSymbols.end()) {
        QString tmp(*it);
        tmp.append('_');
        if (baseName.startsWith(tmp, Qt::CaseInsensitive)) {
          // a match to a symbol ... OK to add the row ...
          QList<QVariant> initialValues;
          // NOTE: initialized with default values here ...
          initialValues.push_back(QString("None"));
          initialValues.push_back(QString("Interpolate"));                             // cross type should be set to interpolate by default
          initialValues.push_back(QString("None"));
          initialValues.push_back(QString("None"));
          initialValues.push_back(true);
          initialValues.push_back(true);                                               // Require filter should be initialized at true !!!
          initialValues.push_back(false);
          initialValues.push_back(true);
          initialValues.push_back(0.0);
          initialValues.push_back(1.0e-3);
          initialValues.push_back(0.0);

          addRow(cStandardRowHeight, *it, initialValues, fileName, QString());
          return;
        }
        ++it;
      }
    }
  }
}

void CWMoleculesDoasTable::slotRemoveRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    removeRow(m_selectedRow);
    m_selectedRow = -1;
  }
}

void CWMoleculesDoasTable::slotChangeCrossSectionFileName()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    QString prefix = rowLabel(m_selectedRow);

    QString filter = prefix;
    filter.append(" (").append(prefix).append("_*.*);;All files (*)");

    // file dialog to change the cross section
    QString filename = QFileDialog::getOpenFileName(this, "Cross section Filename", m_csFilename.at(m_selectedRow), filter);
    QFileInfo fi(filename);                                                     // Done by Caroline (05/12/2007)
    QString base = fi.baseName();                                               // The file name includes the path, so filename.startsWith couldn't be compared with the prefix

    if (!base.isEmpty()) {
      // the file MUST match the prefix
      prefix.append('_');

      if (base.startsWith(prefix, Qt::CaseInsensitive)) {
    m_csFilename[m_selectedRow] = filename; // change the filename
      }
      else {
    QMessageBox::warning(this, "Invalid XS Filename", "The filename was NOT changed because the selected filename\ndid not correspond with the selected symbol.");
      }
    }
  }

}

void CWMoleculesDoasTable::slotAmfFileName()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {
    QString prefix = rowLabel(m_selectedRow);

    QString filter = prefix;
    filter.append(" (").append(prefix).append("_*.AMF_*);;All files (*)");      // file filter depends on the selected SZA type
                                                                                // AMF_SZA for SZA dependent AMF
                                                                                // AMF_CLI for SZA and climatology dependent AMF
                                                                                // AMF_WVE for SZA and wavelength dependent AMF
    // file dialog to change the cross section
    QString filename = QFileDialog::getOpenFileName(this, "AMF Filename", m_amfFilename.at(m_selectedRow), filter);
    QFileInfo fi(filename);                                                     // Done by Caroline (05/12/2007)
    QString base = fi.baseName();                                               // The file name includes the path, so filename.startsWith couldn't be compared with the prefix

    if (!base.isEmpty()) {
      // the file MUST match the prefix
      prefix.append('_');

      if (base.startsWith(prefix, Qt::CaseInsensitive)) {
    m_amfFilename[m_selectedRow] = filename; // change the filename
      }
      else {
    QMessageBox::warning(this, "Invalid AMF Filename", "The filename was NOT changed because the selected filename\ndid not correspond with the selected symbol.");
      }
    }
  }
}

void CWMoleculesDoasTable::slotFitColumnCheckable(int state)
{
  setColumnEnabled(4, (state == Qt::Checked));
}

//------------------------------------------------------------

CWNonLinearParametersDoasTable::CWNonLinearParametersDoasTable(const QString &label,
                                   int headerHeight, QWidget *parent) :
  CDoasTable(label, headerHeight, parent),
  m_selectedRow(-1)
{
  // construct and populate the table

  // columns
  createColumnCheck("Fit", 60);
  createColumnEdit("Val. Init", 80);
  createColumnEdit("Val. Delta", 80);
  createColumnCheck("Fit store", 60);
  createColumnCheck("Err store", 60);
}

void CWNonLinearParametersDoasTable::populate(const struct anlyswin_nonlinear *data)
{
  // predefined rows
  QList<QVariant> initialValues;

  // Sol
  initialValues.push_back(data->solFlagFit);
  initialValues.push_back(data->solInitial);
  initialValues.push_back(data->solDelta);
  initialValues.push_back(data->solFlagFitStore);
  initialValues.push_back(data->solFlagErrStore);
  addRow(cStandardRowHeight, "Sol", initialValues);

  initialValues.clear();

  // Off0
  initialValues.push_back(data->off0FlagFit);
  initialValues.push_back(data->off0Initial);
  initialValues.push_back(data->off0Delta);
  initialValues.push_back(data->off0FlagFitStore);
  initialValues.push_back(data->off0FlagErrStore);
  addRow(cStandardRowHeight, "Offset (Constant)", initialValues);

  initialValues.clear();

  // Off1
  initialValues.push_back(data->off1FlagFit);
  initialValues.push_back(data->off1Initial);
  initialValues.push_back(data->off1Delta);
  initialValues.push_back(data->off1FlagFitStore);
  initialValues.push_back(data->off1FlagErrStore);
  addRow(cStandardRowHeight, "Offset (Order 1)", initialValues);

  initialValues.clear();

  // Off2
  initialValues.push_back(data->off2FlagFit);
  initialValues.push_back(data->off2Initial);
  initialValues.push_back(data->off2Delta);
  initialValues.push_back(data->off2FlagFitStore);
  initialValues.push_back(data->off2FlagErrStore);
  addRow(cStandardRowHeight, "Offset (Order 2)", initialValues);

  initialValues.clear();

  // Com
  initialValues.push_back(data->comFlagFit);
  initialValues.push_back(data->comInitial);
  initialValues.push_back(data->comDelta);
  initialValues.push_back(data->comFlagFitStore);
  initialValues.push_back(data->comFlagErrStore);
  addRow(cStandardRowHeight, "Com", initialValues);

  initialValues.clear();

  // Usamp1
  initialValues.push_back(data->usamp1FlagFit);
  initialValues.push_back(data->usamp1Initial);
  initialValues.push_back(data->usamp1Delta);
  initialValues.push_back(data->usamp1FlagFitStore);
  initialValues.push_back(data->usamp1FlagErrStore);
  addRow(cStandardRowHeight, "Usamp 1", initialValues);

  initialValues.clear();

  // Usamp2
  initialValues.push_back(data->usamp2FlagFit);
  initialValues.push_back(data->usamp2Initial);
  initialValues.push_back(data->usamp2Delta);
  initialValues.push_back(data->usamp2FlagFitStore);
  initialValues.push_back(data->usamp2FlagErrStore);
  addRow(cStandardRowHeight, "Usamp 2", initialValues);

  initialValues.clear();

  // Resol

  initialValues.push_back(data->resolFlagFit);
  initialValues.push_back(data->resolInitial);
  initialValues.push_back(data->resolDelta);
  initialValues.push_back(data->resolFlagFitStore);
  initialValues.push_back(data->resolFlagErrStore);
  addRow(cStandardRowHeight, "Resol", initialValues);

  m_comFilename = data->comFile;
  m_usamp1Filename = data->usamp1File;
  m_usamp2Filename = data->usamp2File;
}

void CWNonLinearParametersDoasTable::apply(struct anlyswin_nonlinear *data) const
{
  QList<QVariant> state;

  state = getCellData(0); // Sol
  data->solFlagFit = state.at(0).toBool() ? 1 : 0;
  data->solInitial = state.at(1).toDouble();
  data->solDelta = state.at(2).toDouble();
  data->solFlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->solFlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(1); // Offset Constant
  data->off0FlagFit = state.at(0).toBool() ? 1 : 0;
  data->off0Initial = state.at(1).toDouble();
  data->off0Delta = state.at(2).toDouble();
  data->off0FlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->off0FlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(2); // Offset Order 1
  data->off1FlagFit = state.at(0).toBool() ? 1 : 0;
  data->off1Initial = state.at(1).toDouble();
  data->off1Delta = state.at(2).toDouble();
  data->off1FlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->off1FlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(3); // Offset Order 2
  data->off2FlagFit = state.at(0).toBool() ? 1 : 0;
  data->off2Initial = state.at(1).toDouble();
  data->off2Delta = state.at(2).toDouble();
  data->off2FlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->off2FlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(4); // Com
  data->comFlagFit = state.at(0).toBool() ? 1 : 0;
  data->comInitial = state.at(1).toDouble();
  data->comDelta = state.at(2).toDouble();
  data->comFlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->comFlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(5); // Usamp1
  data->usamp1FlagFit = state.at(0).toBool() ? 1 : 0;
  data->usamp1Initial = state.at(1).toDouble();
  data->usamp1Delta = state.at(2).toDouble();
  data->usamp1FlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->usamp1FlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(6); // Usamp2
  data->usamp2FlagFit = state.at(0).toBool() ? 1 : 0;
  data->usamp2Initial = state.at(1).toDouble();
  data->usamp2Delta = state.at(2).toDouble();
  data->usamp2FlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->usamp2FlagErrStore = state.at(4).toBool() ? 1 : 0;

  state = getCellData(7); // Resol
  data->resolFlagFit = state.at(0).toBool() ? 1 : 0;
  data->resolInitial = state.at(1).toDouble();
  data->resolDelta = state.at(2).toDouble();
  data->resolFlagFitStore = state.at(3).toBool() ? 1 : 0;
  data->resolFlagErrStore = state.at(4).toBool() ? 1 : 0;

  // filenames

  if (m_comFilename.length() < (int)sizeof(data->comFile))
    strcpy(data->comFile, m_comFilename.toLocal8Bit().data());
  else
    *(data->comFile) = '\0';

  if (m_usamp1Filename.length() < (int)sizeof(data->usamp1File))
    strcpy(data->usamp1File, m_usamp1Filename.toLocal8Bit().data());
  else
    *(data->usamp1File) = '\0';

  if (m_usamp2Filename.length() < (int)sizeof(data->usamp2File))
    strcpy(data->usamp2File, m_usamp2Filename.toLocal8Bit().data());
  else
    *(data->usamp2File) = '\0';
}

void CWNonLinearParametersDoasTable::contextMenuEvent(QContextMenuEvent *e)
{

  m_selectedRow = rowIndexAtPosition(e->y());

  if (m_selectedRow >= 4 && m_selectedRow <=6) {

    // create a popup menu
    QMenu menu;

    menu.addAction("Select File", this, SLOT(slotSelectFile()));

    menu.exec(e->globalPos()); // a slot will do the rest
  }
}

void CWNonLinearParametersDoasTable::slotSelectFile()
{
  switch (m_selectedRow) {
  case 4: // Com
    {
      QString filename = QFileDialog::getOpenFileName(this, "Com Filename", m_comFilename);
      if (!filename.isEmpty()) {
    m_comFilename = filename; // change the filename
      }
    }
    break;
  case 5: // Usamp1
    {
      QString filename = QFileDialog::getOpenFileName(this, "Usamp1 Filename", m_usamp1Filename);
      if (!filename.isEmpty()) {
    m_usamp1Filename = filename; // change the filename
      }
    }
    break;
  case 6: // Usamp2
    {
      QString filename = QFileDialog::getOpenFileName(this, "Usamp2 Filename", m_usamp2Filename);
      if (!filename.isEmpty()) {
    m_usamp2Filename = filename; // change the filename
      }
    }
    break;
  }
}

//------------------------------------------------------------

CWShiftAndStretchDialog::CWShiftAndStretchDialog(const QStringList &freeSymbols, const QStringList &selectedSymbols,
                         QWidget *parent) :
  QDialog(parent)
{
  setWindowTitle("Symbol Selection");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  m_symbolView = new QListWidget(this);
  mainLayout->addWidget(m_symbolView);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  mainLayout->addWidget(buttonBox);

  // populate the list .. selected symbols first
  QListWidgetItem *item;
  QStringList::const_iterator it = selectedSymbols.begin();
  while (it != selectedSymbols.end()) {
    item = new QListWidgetItem(*it, m_symbolView);
    item->setCheckState(Qt::Checked);
    ++it;
  }
  it = freeSymbols.begin();
  while (it != freeSymbols.end()) {
    item = new QListWidgetItem(*it, m_symbolView);
    item->setCheckState(Qt::Unchecked);
    ++it;
  }

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QStringList CWShiftAndStretchDialog::selectedSymbols(void) const
{
  QStringList selection;
  QListWidgetItem *item;
  int index = 0;

  while (index < m_symbolView->count()) {
    item = m_symbolView->item(index);
    if (item->checkState() == Qt::Checked)
      selection << item->text();
    ++index;
  }

  return selection;
}

//------------------------------------------------------------

CWShiftAndStretchDoasTable::CWShiftAndStretchDoasTable(const QString &label,
                               int headerHeight, QWidget *parent) :
  CDoasTable(label, headerHeight, parent),
  m_selectedRow(-1)
{
  m_specialSymbols << "Spectrum" << "Ref";
  m_freeSymbols = m_specialSymbols;

  QStringList orderItems, shiftItems;
  shiftItems << "Do not fit" << "Non linear"; // << "Linear";
  orderItems << "None" << "1st Order" << "2nd Order";

  // columns
  createColumnCombo("Shift fit", 90, shiftItems);
  createColumnCombo("Stretch fit", 90, orderItems);

  createColumnCheck("Sh store", 60);   // column 3
  createColumnCheck("St store", 60);
  createColumnCheck("Err store", 60);

  createColumnEdit("Sh Init (nm)", 80);
  createColumnEdit("St Init", 80);
  createColumnEdit("St Init (2)", 80);

  createColumnEdit("Sh Delta (nm)", 80);
  createColumnEdit("St Delta", 80);
  createColumnEdit("St Delta (2)", 80);

  createColumnEdit("Sh min (nm)", 80);
  createColumnEdit("Sh max (nm)", 80);
}

void CWShiftAndStretchDoasTable::populate(const shift_stretch_list_t *data)
{
  int row = 0;
  const struct anlyswin_shift_stretch *d = &(data->shiftStretch[0]);

  while (row < data->nShiftStretch) {
    QString label;
    QList<QVariant> initialValues;

    initialValues.push_back(mapShiftToComboString(d->shFit));
    initialValues.push_back(mapOrderToComboString(d->stFit));

    initialValues.push_back(d->shStore);
    initialValues.push_back(d->stStore);
    initialValues.push_back(d->errStore);

    initialValues.push_back(d->shInit);
    initialValues.push_back(d->stInit);
    initialValues.push_back(d->stInit2);

    initialValues.push_back(d->shDelta);
    initialValues.push_back(d->stDelta);
    initialValues.push_back(d->stDelta2);

    initialValues.push_back(d->shMin);
    initialValues.push_back(d->shMax);

    int i = 0;
    while (i < d->nSymbol) {
      if (i == 0) {
    label = QString(d->symbol[i]);
      }
      else {
    label.append("; ");
    label.append(QString(d->symbol[i]));
      }
      ++i;
    }

    addRow(cStandardRowHeight, label, initialValues);

    ++d;
    ++row;
  }
}

void CWShiftAndStretchDoasTable::apply(shift_stretch_list_t *data) const
{
  int row = 0;
  int col;
  struct anlyswin_shift_stretch *d = &(data->shiftStretch[0]);

  data->nShiftStretch = rowCount();

  while (row < data->nShiftStretch && row < MAX_AW_SHIFT_STRETCH) {

    const QStringList &tmp = m_selectedSymbolList.at(row);

    d->nSymbol = tmp.count();
    int nSym = 0;
    while (nSym < d->nSymbol && nSym < MAX_AW_SHIFT_STRETCH) {
      // certain that the length of the symbol is below the limit ...
      strcpy(&(d->symbol[nSym][0]), tmp.at(nSym).toLocal8Bit().data());
      ++nSym;
    }

    QList<QVariant> state = getCellData(row);

    col=0;

    d->shFit = mapComboStringToShift(state.at(col++).toString());
    d->stFit = mapComboStringToOrder(state.at(col++).toString());

    d->shStore = state.at(col++).toBool() ? 1 : 0;
    d->stStore = state.at(col++).toBool() ? 1 : 0;
    d->errStore = state.at(col++).toBool() ? 1 : 0;

    d->shInit = state.at(col++).toDouble();
    d->stInit = state.at(col++).toDouble();
    d->stInit2 = state.at(col++).toDouble();

    d->shDelta = state.at(col++).toDouble();
    d->stDelta = state.at(col++).toDouble();
    d->stDelta2 = state.at(col++).toDouble();

    d->shMin = state.at(col++).toDouble();
    d->shMax = state.at(col++).toDouble();

    ++d;
    ++row;
  }
}

void CWShiftAndStretchDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData)
{
  int index;
  QStringList symbols = label.split("; ");

  QStringList::iterator it = symbols.begin();
  while (it != symbols.end()) {
    // update the freeSymbol list
    index = m_freeSymbols.indexOf(*it);
    if (index != -1)
      m_freeSymbols.removeAt(index);

    // lock the symbol ... unless it is a special symbol
    if (m_specialSymbols.indexOf(*it) == -1)
      emit signalLockSymbol(*it, this);

    ++it;
  }

  m_selectedSymbolList.push_back(symbols); // add the row ...
  CDoasTable::addRow(height, label, cellData);
}

void CWShiftAndStretchDoasTable::removeRow(int rowIndex)
{
  if (rowIndex >= 0 && rowIndex < rowCount()) {

    const QStringList &symbols = m_selectedSymbolList.at(rowIndex);

    QStringList::const_iterator it = symbols.begin();
    while (it != symbols.end()) {
      // release locks ... unless special symbol
      if (m_specialSymbols.indexOf(*it) == -1)
    emit signalUnlockSymbol(*it, this);
      // put symbol back in the freeSymbol list
      m_freeSymbols << *it;
      ++it;
    }

    m_selectedSymbolList.removeAt(rowIndex);
    CDoasTable::removeRow(rowIndex);
  }
}

//void CWShiftAndStretchDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
//{
//}

void CWShiftAndStretchDoasTable::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  QAction *insertAction = menu.addAction("Insert", this, SLOT(slotInsertRow()));
  QAction *removeAction = menu.addAction("Remove", this, SLOT(slotRemoveRow()));
  QAction *modifyAction = menu.addAction("Modify", this, SLOT(slotModifyRow()));

  m_selectedRow = rowIndexAtPosition(e->y());

  insertAction->setEnabled(!m_freeSymbols.empty());
  removeAction->setEnabled(m_selectedRow != -1);
  modifyAction->setEnabled(m_selectedRow != -1);

  menu.exec(e->globalPos()); // a slot will do the rest
}

QString CWShiftAndStretchDoasTable::mapShiftToComboString(int shiftFit)
{
  switch (shiftFit) {
  case ANLYS_SHIFT_TYPE_NONE: return QString("Do not fit"); break;
  case ANLYS_SHIFT_TYPE_NONLINEAR: return QString("Non linear"); break;
  // case ANLYS_SHIFT_TYPE_LINEAR: return QString("Linear"); break;
  }

  return QString("None");
}

int CWShiftAndStretchDoasTable::mapComboStringToShift(const QString &str)
{
  if (str == "Do not fit") return ANLYS_SHIFT_TYPE_NONE;
  if (str == "Non linear") return ANLYS_SHIFT_TYPE_NONLINEAR;
  // if (str == "Linear") return ANLYS_SHIFT_TYPE_LINEAR;

  return ANLYS_SHIFT_TYPE_NONE;
}

QString CWShiftAndStretchDoasTable::mapOrderToComboString(int order)
{
  switch (order) {
  case ANLYS_STRETCH_TYPE_FIRST_ORDER: return QString("1st Order"); break;
  case ANLYS_STRETCH_TYPE_SECOND_ORDER: return QString("2nd Order"); break;
  }

  return QString("None");
}

int CWShiftAndStretchDoasTable::mapComboStringToOrder(const QString &str)
{
  if (str == "1st Order") return ANLYS_STRETCH_TYPE_FIRST_ORDER;
  if (str == "2nd Order") return ANLYS_STRETCH_TYPE_SECOND_ORDER;

  return ANLYS_STRETCH_TYPE_NONE;
}

void CWShiftAndStretchDoasTable::slotSymbolListChanged(const QStringList &symbols)
{
  bool found;
  QList<QStringList>::const_iterator listIt;

  m_freeSymbols.clear();

  // rebuild the free symbol list

  QStringList::const_iterator it = symbols.begin();
  while (it != symbols.end()) {
    found = false;
    listIt = m_selectedSymbolList.begin();
    while (!found && listIt != m_selectedSymbolList.end()) {
      found = (listIt->indexOf(*it) != -1);
      ++listIt;
    }
    if (!found)
      m_freeSymbols << *it;
    ++it;
  }

  // and the special symbols ...

  it = m_specialSymbols.begin();
  while (it != m_specialSymbols.end()) {
    found = false;
    listIt = m_selectedSymbolList.begin();
    while (!found && listIt != m_selectedSymbolList.end()) {
      found = (listIt->indexOf(*it) != -1);
      ++listIt;
    }
    if (!found)
      m_freeSymbols << *it;
    ++it;
  }

}

void CWShiftAndStretchDoasTable::slotInsertRow()
{
  // popup a dialog for selecting a symbol (list) ...
  CWShiftAndStretchDialog dialog(m_freeSymbols, QStringList(), this);

  if (dialog.exec() == QDialog::Accepted) {
    QStringList selection = dialog.selectedSymbols();

    if (!selection.isEmpty()) {

      QStringList::iterator it = selection.begin();
      QString tmp = *it;
      ++it;
      while (it != selection.end()) {
    tmp.append("; ");
    tmp.append(*it);
    ++it;
      }

      QList<QVariant> initialValues;
      // defaults ...
      initialValues.push_back(QVariant("None"));
      initialValues.push_back(QVariant("None"));
      initialValues.push_back(QVariant(false));
      initialValues.push_back(QVariant(false));
      initialValues.push_back(QVariant(false));
      initialValues.push_back(QVariant(0.0));
      initialValues.push_back(QVariant(0.0));
      initialValues.push_back(QVariant(0.0));
      initialValues.push_back(QVariant(0.001));
      initialValues.push_back(QVariant(0.001));
      initialValues.push_back(QVariant(0.001));
      initialValues.push_back(QVariant(0.0));
      initialValues.push_back(QVariant(0.0));

      addRow(cStandardRowHeight, tmp, initialValues);
    }
  }
}

void CWShiftAndStretchDoasTable::slotRemoveRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    removeRow(m_selectedRow);
    m_selectedRow = -1;
  }
}

void CWShiftAndStretchDoasTable::slotModifyRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    // popup a dialog for modyfying a symbol (list) ...
    CWShiftAndStretchDialog dialog(m_freeSymbols, m_selectedSymbolList.at(m_selectedRow), this);

    if (dialog.exec() == QDialog::Accepted) {
      QStringList selection = dialog.selectedSymbols();

      if (!selection.isEmpty()) {
    // OK to change ...

    int index;
    const QStringList &old = m_selectedSymbolList.at(m_selectedRow);

    // release the locks on the original selection and free those symbols
    QStringList::const_iterator it = old.begin();
    while (it != old.end()) {
      if (m_specialSymbols.indexOf(*it) == -1)
        emit signalUnlockSymbol(*it, this);

      ++it;
    }
    m_freeSymbols << old;

    // now change and lock the new selection ... and change the label ...
    m_selectedSymbolList.replace(m_selectedRow, selection);

    QString tmp;

    it = selection.begin();
    while (it != selection.end()) {
      if (m_specialSymbols.indexOf(*it) == -1)
        emit signalLockSymbol(*it, this);

      index = m_freeSymbols.indexOf(*it);
      if (index != -1)
        m_freeSymbols.removeAt(index);

      if (!tmp.isEmpty())
        tmp.append("; ");
      tmp.append(*it);

      ++it;
    }


    setHeaderLabel(m_selectedRow, tmp);
      }
    }
  }
}

//------------------------------------------------------------

CWGapDoasTable::CWGapDoasTable(const QString &label, int headerHeight, QWidget *parent) :
  CDoasTable(label, headerHeight, parent),
  m_selectedRow(-1)
{
  // two columns
  createColumnEdit("Min (nm)", 80);
  createColumnEdit("Max (nm)", 80);
}

void CWGapDoasTable::populate(const gap_list_t *data)
{
  int row = 0;
  const struct anlyswin_gap *d = &(data->gap[0]);

  while (row < data->nGap && row < MAX_AW_GAP) {

    QList<QVariant> initialValues;
    initialValues.push_back(QVariant(d->minimum));
    initialValues.push_back(QVariant(d->maximum));

    addRow(cStandardRowHeight, "Gap-Invalid", initialValues);

    ++d;
    ++row;
  }
}

void CWGapDoasTable::apply(gap_list_t *data) const
{
  int validRows = 0;
  int row = 0;
  struct anlyswin_gap *d = &(data->gap[0]);

  while (row < rowCount() && validRows < MAX_AW_GAP) {

    QList<QVariant> state = getCellData(row);

    d->minimum = state.at(0).toDouble();
    d->maximum = state.at(1).toDouble();
    // only store rows that are valid
    if (d->minimum > 0.0 && d->maximum > d->minimum) {
      ++validRows;
      ++d;
    }
    ++row;
  }
  data->nGap = validRows;
}

void CWGapDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
{
  // the label is defined by the contents of the row cells
  double minimum = 0.0;
  double maximum = 0.0;

  if (column == 0) {
    minimum = cellData.toDouble();
    maximum = getCellData(row, 1).toDouble();
  }
  else if (column == 1) {
    maximum = cellData.toDouble();
    minimum = getCellData(row, 0).toDouble();
  }

  if (minimum > 0.0 && maximum > minimum) {
    QString tmp;
    QTextStream stream(&tmp);
    stream << "Gap-" << minimum << "-" << maximum;

    setHeaderLabel(row, tmp);
  }
  else {
    setHeaderLabel(row, "Gap-Invalid");
  }
}

void CWGapDoasTable::contextMenuEvent(QContextMenuEvent *e)
{
  // create a popup menu
  QMenu menu;

  QAction *insertAction = menu.addAction("Insert", this, SLOT(slotInsertRow()));
  QAction *removeAction = menu.addAction("Remove", this, SLOT(slotRemoveRow()));

  insertAction->setEnabled(rowCount() < MAX_AW_GAP);
  m_selectedRow = rowIndexAtPosition(e->y());

  removeAction->setEnabled(m_selectedRow != -1);

  menu.exec(e->globalPos()); // a slot will do the rest
}

void CWGapDoasTable::slotInsertRow()
{
  QList<QVariant> initialValues;

  addRow(cStandardRowHeight, "Gap-Invalid", initialValues);
}

void CWGapDoasTable::slotRemoveRow()
{
  if (m_selectedRow >= 0 && m_selectedRow < rowCount()) {

    removeRow(m_selectedRow);
    m_selectedRow = -1;
  }
}


//------------------------------------------------------------

CWOutputDoasTable::CWOutputDoasTable(const QString &label, int headerHeight, QWidget *parent) :
  CDoasTable(label, headerHeight, parent)
{
  // columns
  createColumnCheck("AMFs", 60);                          // col 0
  createColumnEdit("Residuals", 60);
  createColumnCheck("Slnt Col", 60);                      // col 2
  createColumnCheck("Slnt Err", 60);
  createColumnEdit("Slnt Fact", 80);  // col 4
  createColumnCheck("Vrt Col", 60);
  createColumnCheck("Vrt Err", 60);                       // col 6
  createColumnEdit("Vrt Fact", 80);
}

void CWOutputDoasTable::populate(const output_list_t *data)
{
  // rows should already exist for these elements.
  int index;
  int row = 0;
  const struct anlyswin_output *d = &(data->output[0]);

  while (row < data->nOutput && row < MAX_AW_CROSS_SECTION) {
    index = m_symbols.indexOf(QString(d->symbol));
    if (index != -1) {
      // set data
      setCellData(index, 0, QVariant(d->amf));
      setCellData(index, 1, QVariant(d->resCol));

      setCellData(index, 2, QVariant(d->slantCol));
      setCellData(index, 3, QVariant(d->slantErr));
      setCellData(index, 4, QVariant(d->slantFactor));

      setCellData(index, 5, QVariant(d->vertCol));
      setCellData(index, 6, QVariant(d->vertErr));
      setCellData(index, 7, QVariant(d->vertFactor));
    }
    ++d;
    ++row;
  }
}

void CWOutputDoasTable::apply(output_list_t *data) const
{
  int row = 0;
  struct anlyswin_output *d = &(data->output[0]);

  data->nOutput = rowCount();

  while (row < data->nOutput && row < MAX_AW_CROSS_SECTION) {

    strcpy(d->symbol, m_symbols.at(row).toLocal8Bit().data());

    QList<QVariant> state = getCellData(row);

    d->amf = state.at(0).toBool() ? 1 : 0;
    d->resCol = state.at(1).toDouble();
    d->slantCol = state.at(2).toBool() ? 1 : 0;
    d->slantErr = state.at(3).toBool() ? 1 : 0;
    d->slantFactor = state.at(4).toDouble();
    d->vertCol = state.at(5).toBool() ? 1 : 0;
    d->vertErr = state.at(6).toBool() ? 1 : 0;
    d->vertFactor = state.at(7).toDouble();

    ++d;
    ++row;
  }
}

//void CWOutputDoasTable::cellDataChanged(int row, int column, const QVariant &cellData)
//{
//}

// prevent manual add/remove
void CWOutputDoasTable::addRow(int height, const QString &label, QList<QVariant> &cellData)
{
  CDoasTable::addRow(height, label, cellData);
}

void CWOutputDoasTable::removeRow(int rowIndex)
{
  CDoasTable::removeRow(rowIndex);
}

void CWOutputDoasTable::slotSymbolListChanged(const QStringList &symbols)
{
  int index;
  QList<QVariant> initialValues;

  // must be a row for every entry in the list ... and ONLY those in the list.
  QStringList::const_iterator it = symbols.begin();
  while (it != symbols.end()) {
    index = m_symbols.indexOf(*it);
    if (index == -1) {
      // not found => no row for this exists yet, so add one.
      if (initialValues.isEmpty()) {
    initialValues.push_back(QVariant(false));
    initialValues.push_back(QVariant());
    initialValues.push_back(QVariant(true));
    initialValues.push_back(QVariant(true));
    initialValues.push_back(QVariant(1.0));
    initialValues.push_back(QVariant(false));
    initialValues.push_back(QVariant(false));
    initialValues.push_back(QVariant(1.0));
      }
      m_symbols << *it;
      CDoasTable::addRow(cStandardRowHeight, *it, initialValues);
    }
    ++it;
  }
  // now make sure that only these rows exist ...
  int row = 0;
  while (row < m_symbols.count()) {
    index = symbols.indexOf(m_symbols.at(row));
    if (index == -1) {
      // this row should not exist, so remove it
      m_symbols.removeAt(row);
      CDoasTable::removeRow(row);
    }
    else {
      ++row;
    }
  }
}


//------------------------------------------------------------
CWSfpParametersDoasTable::CWSfpParametersDoasTable(const QString &label, int headerHeight, QWidget *parent) :
  CDoasTable(label, headerHeight, parent)
{
  // 4 fixed row
  createColumnCheck("Fit", 60);
  createColumnEdit("Init. Val.", 80);
  createColumnEdit("delta Val.", 80);
  createColumnCheck("Fit store", 60);
  createColumnCheck("Err store", 60);
}

// virtual void cellDataChanged(int row, int column, const QVariant &cellData); // no cell-coupling required

void CWSfpParametersDoasTable::populate(const struct calibration_sfp *data)
{
  for (int i = 0; i<NSFP; ++i) {
    QList<QVariant> initialValues;
    initialValues.push_back(QVariant(data->fitFlag));
    initialValues.push_back(QVariant(data->initialValue));
    initialValues.push_back(QVariant(data->deltaValue));
    initialValues.push_back(QVariant(data->fitStore));
    initialValues.push_back(QVariant(data->errStore));

    QString label("SFP");
    label.append(QString::number(1 + i));

    addRow(cStandardRowHeight, label, initialValues);
    ++data;
  }
}

void CWSfpParametersDoasTable::apply(struct calibration_sfp *data) const
{
  for (int i = 0; i<NSFP; ++i) {
    QList<QVariant> state = getCellData(i);

    data->fitFlag = (state.at(0).toBool() ? 1 : 0);

    data->initialValue = state.at(1).toDouble();
    data->deltaValue = state.at(2).toDouble();

    data->fitStore = (state.at(3).toBool() ? 1 : 0);
    data->errStore = (state.at(4).toBool() ? 1 : 0);

    ++data;
  }
}

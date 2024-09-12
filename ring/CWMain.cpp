/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#include <cstring>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QApplication>
#include <QLocale>
#include <clocale>

#include "CWMain.h"
#include "CWAboutDialog.h"
#include "CHelpSystem.h"
#include "CWPlotPropertiesDialog.h"

#include "CRingEngineController.h"

#include "CPathMgr.h"
#include "CRingConfigHandler.h"
#include "CPreferences.h"
#include "CRingConfigWriter.h"
#include "CEngineResponse.h"

#include "../mediator/mediate_response.h"
#include "../mediator/mediate_types.h"
#include "../mediator/mediate_xsconv.h"

#include "debugutil.h"

CWMain::CWMain(QWidget *parent) :
  QFrame(parent),
  m_plotArea(NULL)
{
  // ----------------------------------------------------------------------------

    // to avoid that a thousands comma separator (QT 4.7.3)

       QLocale qlocale=QLocale::system();
       qlocale.setNumberOptions(QLocale::OmitGroupSeparator);
       QLocale::setDefault(qlocale);

  setlocale(LC_NUMERIC, "C");

  // ----------------------------------------------------------------------------
  initializeMediateRing(&m_guiProperties);

  setConfigFileName(QString());

  // controller
  m_controller = new CRingEngineController(this);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(0);

  //------------------------------
  // Menu Bar

  m_menuBar = new QMenuBar(this);
  mainLayout->addWidget(m_menuBar, 0);

  // File Menu
  QMenu *fileMenu = new QMenu("File");

  // Open...
  QAction *openAct = new QAction(QIcon(QPixmap(":/icons/file_open_16.png")), "Open...", this);
  connect(openAct, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
  fileMenu->addAction(openAct);

  // New
  QAction *newProjAction = new QAction(QIcon(QPixmap(":/icons/file_new_16.png")), "New", this);
  connect(newProjAction, SIGNAL(triggered()), this, SLOT(slotNewFile()));
  fileMenu->addAction(newProjAction);

  fileMenu->addSeparator();

  // Save + Save As ...
  m_saveAction = new QAction(QIcon(QPixmap(":/icons/file_save_16.png")), "Save", this);
  connect(m_saveAction, SIGNAL(triggered()), this, SLOT(slotSaveFile()));
  fileMenu->addAction(m_saveAction);

  m_saveAsAction = new QAction(QIcon(QPixmap(":/icons/file_saveas_16.png")), "Save As...", this);
  connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(slotSaveAsFile()));
  fileMenu->addAction(m_saveAsAction);

  // Quit
  fileMenu->addSeparator();
  QAction *exitAct = new QAction(QIcon(QPixmap(":/icons/file_exit_16.png")), "Quit", this);
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
  fileMenu->addAction(exitAct);

  m_menuBar->addMenu(fileMenu);

  // Calculate menu
  QMenu *calcMenu = new QMenu("Calculate");
  QAction *convAct = new QAction("Calculate a Ring cross section", this);
  connect(convAct, SIGNAL(triggered()), this, SLOT(slotRunRing()));
  calcMenu->addAction(convAct);

  m_menuBar->addMenu(calcMenu);

  // Plot menu
  QMenu *plotMenu = new QMenu("Plot");

  // plot properties and printing
  QAction *plotPropAction = new QAction(QIcon(QPixmap(":/icons/plot_prop_22.png")), "Plot Properties...", this);
  connect(plotPropAction, SIGNAL(triggered()), this, SLOT(slotEditPlotProperties()));
  plotMenu->addAction(plotPropAction);

  QAction *plotPrintAction = new QAction(QIcon(QPixmap(":/icons/plot_print_22.png")), "Print Plots...", this);
  connect(plotPrintAction, SIGNAL(triggered()), this, SLOT(slotPrintPlots()));
  plotMenu->addAction(plotPrintAction);

  QAction *plotExportAction = new QAction(QIcon(QPixmap(":/icons/plot_export_22.png")), "Export Plots...", this);
  connect(plotExportAction, SIGNAL(triggered()), this, SLOT(slotExportPlots()));
  plotMenu->addAction(plotExportAction);

  m_menuBar->addMenu(plotMenu);

  // Help Menu
  QMenu *helpMenu = new QMenu("Help");

  // About
  helpMenu->addAction("Qdoas Help", this, SLOT(slotQdoasHelp()));
  // dual help systems ...
  QSettings &settings = CPreferences::instance()->settings();

  helpMenu->addAction("About Qdoas", this, SLOT(slotAboutQdoas()));
  helpMenu->addSeparator();
  helpMenu->addAction("About Qt", this, SLOT(slotAboutQt()));

  // NOT NEEDED ANYMORE connect(helpCheck, SIGNAL(triggered(bool)), this, SLOT(slotHelpBrowserPreference(bool)));

  m_menuBar->addMenu(helpMenu);

  // tab - config pages and eventually the plot.
  m_tab = new QTabWidget(this);

  m_generalTab = new CWRingTabGeneral(&m_guiProperties);
  m_tab->addTab(m_generalTab, "General");

  mainLayout->addWidget(m_tab, 1);

  // re-read the guiProperties and synchronize ... limits on the widgets can change the initialized state.
  fetchGuiProperties();
  m_properties = m_guiProperties;

  // icon
  setWindowIcon(QIcon(QPixmap(":/icons/qdoas_atmospheric_toolbox_ico.png")));

  // get the window size from the settings
  resize(CPreferences::instance()->windowSize("RingTool", QSize(450,350)));

  // connections
  connect(m_controller, SIGNAL(signalPlotPage(std::shared_ptr<const CPlotPageData>)),
      this, SLOT(slotPlotPage(std::shared_ptr<const CPlotPageData>)));
  connect(m_controller, SIGNAL(signalErrorMessages(int, const QString &)),
      this, SLOT(slotErrorMessages(int, const QString &)));
}

void CWMain::closeEvent(QCloseEvent *e)
{
  // save preferences ...
  CPreferences::instance()->setWindowSize("RingTool", size());

  if (m_plotArea != NULL)
    CWPlotPropertiesConfig::saveToPreferences(m_plotArea->properties());

  // flush write and close ...
  delete CPreferences::instance();

  if (checkStateAndConsiderSaveFile()) {

    e->accept();
    return;
  }

  e->ignore();
}

bool CWMain::checkStateAndConsiderSaveFile(void)
{

  fetchGuiProperties();

  if (compareProperties())
    return true; // state unchanged

  // prompt to choose to save changes or cancel
  QString msg;

  if (m_configFile.isEmpty()) {
    msg = "Save the current configuration file?";
  }
  else {
    msg = "Save changes to the configuration file\n";
    msg += m_configFile;
    msg += " ?";
  }

  QMessageBox::StandardButton choice = QMessageBox::question(this, "Save Changes", msg,
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                 QMessageBox::Save);

  if (choice == QMessageBox::Discard)
    return true; // discard changes
  else if (choice != QMessageBox::Save)
    return false; // cancel (or any other action that was not save)

  // save or saveAs (as appropriate).
  slotSaveFile();

  // a SaveAs could have been cancelled (for some reason).
  return compareProperties();
}

void CWMain::setConfigFileName(const QString &fileName)
{
  QString str("Ring - ");

  m_configFile = fileName;

  if (m_configFile.isEmpty()) {
    str += "Unnamed[*]";
  }
  else {
    str += m_configFile;
    str += "[*]";
  }

  setWindowTitle(str);
}

void CWMain::fetchGuiProperties(void)
{
  // get the state of the GUI and store it in m_guiProperties. Initialize first so that
  // strings are null padded (and memcmp) can be used to test for state changes.
  initializeMediateRing(&m_guiProperties);

  m_generalTab->apply(&m_guiProperties);
}

bool CWMain::compareProperties(void)
{
  // see if the properties (last saved/read) and last fetched from the GUI are
  // the same. returns true if they are equivalent.

  return (memcmp(&m_properties, &m_guiProperties, sizeof(mediate_ring_t)) == 0);
}

void CWMain::slotOpenFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;

  CPreferences *prefs = CPreferences::instance();

  QString fileName = QFileDialog::getOpenFileName(this, "Open Project File",
                          prefs->directoryName("RingConf"),
                          "Ring Config (*.xml);;All Files (*)");

  if (fileName.isEmpty()) {
    return;
  }

  // save the last directory
  prefs->setDirectoryNameGivenFile("RingConf", fileName);

  QString errMsg;

  // parse the file

  CRingConfigHandler handler;
  handler.set_substitute_entities(true);

  try {
    handler.parse_file(fileName.toStdString());

    // start with a clear configuration
    CPathMgr *pathMgr = CPathMgr::instance();

    pathMgr->removeAll();

    // store the paths in the pathMgr for simplification when saving ...
    for (int i = 0; i<10; ++i) {
      QString path = handler.getPath(i);
      if (path.isEmpty())
    pathMgr->removePath(i);
      else
    pathMgr->addPath(i, path);
    }

    // copy the properties data ...
    m_guiProperties = *(handler.properties());

    // update the GUI
    m_generalTab->reset(&m_guiProperties);

    fetchGuiProperties(); // see note above about synchronization ...
    m_properties = m_guiProperties;

    setConfigFileName(fileName);
  }
  catch(std::runtime_error& e) {
    errMsg = e.what();
  }

  if (!errMsg.isNull())
    QMessageBox::critical(this, "File Open", errMsg);
}

void CWMain::slotNewFile()
{
  if (!checkStateAndConsiderSaveFile())
    return;

  initializeMediateRing(&m_guiProperties);

  // update the GUI
  m_generalTab->reset(&m_guiProperties);

  fetchGuiProperties(); // see note above about synchronization ...
  m_properties = m_guiProperties;

  setConfigFileName(QString()); // no config file name
}

void CWMain::slotSaveAsFile()
{
  QMessageBox::StandardButton returnCode = QMessageBox::Retry;

  fetchGuiProperties();

  CRingConfigWriter writer(&m_guiProperties);

  while (returnCode == QMessageBox::Retry) {

    returnCode = QMessageBox::Cancel;

    QString fileName = QFileDialog::getSaveFileName(this, "SaveAs Config File",
                            CPreferences::instance()->directoryName("RingConf"),
                            "Ring Config (*.xml);;All Files (*)");

    // empty fileName implies cancel
    if (!fileName.isEmpty()) {

      if (!fileName.contains('.'))
    fileName += ".xml";

      // write the file
      QString msg = writer.write(fileName);
      if (!msg.isNull()) {
    msg += "\nPress Retry to select another output configuration file.";
    returnCode = QMessageBox::critical(this, "Configuration File Write Failure", msg,
                       QMessageBox::Retry | QMessageBox::Cancel,
                       QMessageBox::Retry);
      }
      else {
    // wrote the file ... change the project filename and store the properties
    setConfigFileName(fileName);
    m_properties = m_guiProperties;
      }
    }
  }

}

void CWMain::slotSaveFile()
{

  if (m_configFile.isEmpty()) {
    slotSaveAsFile();
  }
  else {

    fetchGuiProperties();
    CRingConfigWriter writer(&m_guiProperties);

    QString msg = writer.write(m_configFile);
    if (!msg.isNull())
      QMessageBox::critical(this, "Configuration File Write Failure", msg, QMessageBox::Ok);
    else
      m_properties = m_guiProperties;

  }

}

void CWMain::slotEditPlotProperties()
{
  CPlotProperties prop;

  if (m_plotArea != NULL)
    prop = m_plotArea->properties();
  else
    CWPlotPropertiesConfig::loadFromPreferences(prop);

  CWPlotPropertiesDialog dialog(prop, this);

  if (dialog.exec() == QDialog::Accepted) {

    if (m_plotArea != NULL)
      m_plotArea->setProperties(prop);
    else
      CWPlotPropertiesConfig::saveToPreferences(prop);

  }
}

void CWMain::slotPrintPlots()
{
  if (m_plotArea != NULL)
    m_plotArea->printPage();
}

void CWMain::slotExportPlots()
{
  if (m_plotArea != NULL)
    m_plotArea->exportPage();
}

void CWMain::slotQdoasHelp()
{
    CHelpSystem::showHelpTopic(QString("Tools"),QString("Tools_Ring"));
}

void CWMain::slotAboutQdoas()
{
  CWAboutDialog dialog(this);
  dialog.exec();
}

void CWMain::slotAboutQt()
{
  QApplication::aboutQt();
}

void CWMain::slotErrorMessages(int highestLevel, const QString &messages)
{
  switch (highestLevel) {
  case InformationEngineError:
    QMessageBox::information(this, "Ring tool ; Information", messages);
    break;
  case WarningEngineError:
    QMessageBox::warning(this, "Ring tool : Warning", messages);
    break;
  case FatalEngineError:
  default:
    QMessageBox::critical(this, "Ring tool : Fatal Error", messages);
    break;
  }
}

void CWMain::slotRunRing()
{
     int rc;

  // uses a snapshot of the guiProperties ...
  fetchGuiProperties();

  // get an engine context
  void *engineContext;
  CEngineResponseVisual *resp = new CEngineResponseVisual;

  if (mediateXsconvCreateContext(&engineContext, resp) != 0) {
    delete resp;
    return;
  }

  if ((rc=mediateRequestRing(engineContext, &m_guiProperties,resp))==ERROR_ID_NO)
   {
       QApplication::setOverrideCursor(Qt::WaitCursor);
    rc=mediateRingCalculate(engineContext,resp);
    QApplication::restoreOverrideCursor();
   }

  if (rc!=ERROR_ID_NO)
   ERROR_DisplayMessage(resp);

  // process the response - the controller will dispatch ...
  resp->process(m_controller);

  if (mediateXsconvDestroyContext(engineContext, resp) != 0) {
    delete resp;
    return;
  }

  delete resp;
}

void CWMain::slotPlotPage(std::shared_ptr<const CPlotPageData> page)
{
  if (page) {
    // lazy creation of the plot tab ...
    if (!m_plotArea) {
      CPlotProperties prop;
      CWPlotPropertiesConfig::loadFromPreferences(prop);

      m_plotArea = new CWPlotArea;
      m_plotArea->setProperties(prop);

      m_tab->addTab(m_plotArea, page->title());
    }

    m_plotArea->setPage(page);
    m_plotArea->show();

    m_tab->setCurrentWidget(m_plotArea);
  }
}



#include <cstdio>
#include <cstring>

#include <iostream>
#include <string>

#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QFile>
#include <QString>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <clocale>
#include <QTextCodec>

#include "CWorkSpace.h"
#include "CQdoasConfigHandler.h"
#include "CProjectConfigItem.h"
#include "CBatchEngineController.h"
#include "CEngineResponse.h"
#include "CProjectConfigTreeNode.h"
#include "constants.h"

#include "../convolution/CConvConfigHandler.h"
#include "../ring/CRingConfigHandler.h"
#include "../usamp/CUsampConfigHandler.h"

#include "mediate_xsconv.h"

#include "QdoasVersion.h"
#include "debugutil.h"
#include "qdoasxml.h"
#include "convxml.h"


//-------------------------------------------------------------------
// types
//-------------------------------------------------------------------

enum RunMode {
  None,
  Error,
  Help,
  Batch
};

enum BatchTool {
  Unknown,
  Qdoas,
  Convolution,
  Ring,
  Usamp
};

typedef struct commands
{
  QString configFile;
  QString projectName;
  QList<QString> filenames;
  QList<QString> xmlCommands;
  QString outputDir;
  QString calibDir;
} commands_t;

//-------------------------------------------------------------------
// declarations
//-------------------------------------------------------------------

enum RunMode parseCommandLine(int argc, char **argv, commands_t *cmd);
enum BatchTool requiredBatchTool(const QString &filename);
void showUsage();
void showHelp();
int batchProcess(commands_t *cmd);

int batchProcessQdoas(commands_t *cmd);
int readConfigQdoas(commands_t *cmd, QList<const CProjectConfigItem*> &projectItems);
int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir, const QString &calibDir, const QList<QString> &filenames);
int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir,const QString &calibDir);
int analyseProjectQdoasPrepare(void **engineContext, const CProjectConfigItem *projItem, const QString &outputDir,const QString &calibDir,
                   CBatchEngineController *controller);
int analyseProjectQdoasFile(void *engineContext, CBatchEngineController *controller, const QString &filename);
int analyseProjectQdoasTreeNode(void *engineContext, CBatchEngineController *controller, const CProjectConfigTreeNode *node);
int analyseProjectQdoasDirectory(void *engineContext, CBatchEngineController *controller, const QString &dir,
                 const QString &filters, bool recursive);



int batchProcessConvolution(commands_t *cmd);
int batchProcessRing(commands_t *cmd);
int batchProcessUsamp(commands_t *cmd);

int calibSwitch=0;
int calibSaveSwitch=0;
int xmlSwitch=0;
int verboseMode=0;

//-------------------------------------------------------------------

int main(int argc, char **argv)
{
  int retCode = 0;

  // ----------------------------------------------------------------------------

    // to avoid that a thousands comma separator (QT 4.7.3)

       QLocale qlocale=QLocale::system();
       qlocale.setNumberOptions(QLocale::OmitGroupSeparator);
       QLocale::setDefault(qlocale);

  setlocale(LC_NUMERIC, "C");

  // ----------------------------------------------------------------------------

  if (argc == 1) {

    showUsage();
  }
  else {
    commands_t cmd;

    enum RunMode runMode = parseCommandLine(argc, argv, &cmd);

    switch (runMode) {
    case None:
    case Error:
      showUsage();
      break;
    case Help:
      showHelp();
      break;
    case Batch:
      retCode = batchProcess(&cmd);
      break;
    }
  }

  return retCode;
}


//-------------------------------------------------------------------

enum RunMode parseCommandLine(int argc, char **argv, commands_t *cmd)
{
  // extract data from command line
  enum RunMode runMode = None;
  int i = 1;
  int fileSwitch = 0;

  while (runMode != Error && i < argc) {

    // options ...
    if (argv[i][0] == '-') {

      if (!strcmp(argv[i], "-c")) { // configuration file ...
    if (++i < argc && argv[i][0] != '-') {
         fileSwitch=0;
      if (cmd->configFile.isEmpty()) {
        cmd->configFile = argv[i];
        runMode = Batch;
      }
      else
        std::cout << "Duplicate '-c' option." << std::endl;
    }
    else {
      runMode = Error;
      std::cout << "Option '-c' requires an argument (configuration file)." << std::endl;
    }

      }
      else if (!strcmp(argv[i], "-a")) { // project name file ...
    if (++i < argc && argv[i][0] != '-') {
         fileSwitch=0;
      cmd->projectName = argv[i];
    }
    else {
      runMode = Error;
      std::cout << "Option '-a' requires an argument (project name)." << std::endl;
    }

      }
      else if (!strcmp(argv[i], "-k")) { // project name file ...
    if (++i < argc && argv[i][0] != '-') {
         fileSwitch=0;
         calibSwitch=1;
      cmd->projectName = argv[i];
    }
    else {
      runMode = Error;
      std::cout << "Option '-k' requires an argument (project name)." << std::endl;
    }

      }
      else if (!strcmp(argv[i], "-saveref")) { // save irradiances
            calibSaveSwitch=calibSwitch;
            if (!calibSwitch)
              std::cout << "Warning : Option '-saveref' has effect only with '-k' option." << std::endl;
      }

      else if (!strcmp(argv[i], "-f")) { // filename ...
    if (++i < argc && argv[i][0] != '-')
         {
    fileSwitch=1;
       cmd->filenames.push_back(argv[i]);
      }
    else {
      runMode = Error;
      std::cout << "Option '-f' requires an argument (filename)." << std::endl;
    }

      }
      else if (!strcmp(argv[i], "-new_irrad")) { // filename ...
        if (++i < argc && argv[i][0] != '-')
                 {
    calibSaveSwitch=1;
          cmd->calibDir=argv[i];
          }
        else {
          runMode = Error;
          std::cout << "Option '-new_irrad' requires an argument (filename)." << std::endl;
        }

      }
 else if (!strcmp(argv[i],"-xml"))
     if (++i < argc && argv[i][0] != '-')
          {
     xmlSwitch=1;
        cmd->xmlCommands.push_back(argv[i]);
       }
     else {
       runMode = Error;
       std::cout << "Option '-xml' requires at least an argument (xmlPath=xmlValue)." << std::endl;
     }
    else if (!strcmp(argv[i],"-v"))
     verboseMode=1;
 else if (!strcmp(argv[i], "-o")) { // output directory ...
    if (++i < argc && argv[i][0] != '-') {
         fileSwitch=0;
      cmd->outputDir = argv[i];
    }
    else {
      runMode = Error;
      std::cout << "Option '-o' requires an argument (directory)." << std::endl;
    }

      }
      else if (!strcmp(argv[i], "-h")) { // help ...
          fileSwitch=0;
    runMode = Help;
      }
    }
    else if (fileSwitch)
     cmd->filenames.push_back(argv[i]);
    else
    {
      runMode = Error;
      std::cout << "Invalid argument '" << argv[i] << "'" << std::endl;
    }

    ++i;
  }
  
  if ((runMode==None) && calibSaveSwitch && !calibSwitch)
   std::cout << "Warning : -new_irrad switch to use only with -k option; ignored " << std::endl;

  // consistency checks ??

  return runMode;
}

//-------------------------------------------------------------------

int batchProcess(commands_t *cmd)
{
  // determine the tool to use based on the config file ...

  enum BatchTool batchTool = requiredBatchTool(cmd->configFile);

  switch (batchTool) {
  case Qdoas:
    return batchProcessQdoas(cmd);
    break;
  case Convolution:
    return batchProcessConvolution(cmd);
    break;
  case Ring:
    return batchProcessRing(cmd);
    break;
  case Usamp:
    return batchProcessUsamp(cmd);
    break;
  default:
    std::cout << "Failed to open or determine configuration file type." << std::endl;
  }

  return 1;
}

//-------------------------------------------------------------------

enum BatchTool requiredBatchTool(const QString &filename)
{
  enum BatchTool type = Unknown;

  FILE *fp = fopen(filename.toLocal8Bit().constData(), "r");
  if (fp != NULL) {
    char buffer[256];

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
      // should begin with '<?xml'
      if (strlen(buffer) > 5 && !strncmp(buffer, "<?xml", 5)) {

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
      // opening tag ...
      int len = strlen(buffer);

      if (len >= 7 && !strncmp(buffer, "<qdoas>", 7))
        type = Qdoas;
      else if (len >= 13 && !strncmp(buffer, "<convolution>", 13))
        type = Convolution;
      else if (len >= 6 && !strncmp(buffer, "<ring>", 6))
        type = Ring;
      else if (len >= 7 && !strncmp(buffer, "<usamp>", 7))
        type = Usamp;

    }
      }
    }

    fclose(fp);
  }

  return type;
}

//-------------------------------------------------------------------

void showUsage()
{
  std::cout << "doas_cl -c <config file> [-a/-k <project name>] [-o <output>] [-f <file>]..." << std::endl << std::endl;
  std::cout << "    -c <config file>    : A QDoas, convolution, [ring or usamp] config file." << std::endl;
  std::cout << "                          The tool to invoke is determined from the type of" << std::endl;
  std::cout << "                          configuration file specified;" << std::endl << std::endl;
  std::cout << "    -a <project name>   : for QDoas, run analysis on the specified project" << std::endl;
  std::cout << "    -k <project name>   : for QDoas, run calibration on the specified project" << std::endl << std::endl;
  std::cout << "    -new_irrad <output> : for QDoas, run calibration, GEMS measurements, calibrated irradiances file" << std::endl << std::endl;
  std::cout << "    -v                  : verbose on (default is off)" << std::endl << std::endl;
  std::cout << "    -xml <path=value>   : advanced option to replace the values of some options " << std::endl;
  std::cout << "                          in the configuration file by new ones." << std::endl;
  std::cout << "------------------------------------------------------------------------------" << std::endl;
  std::cout << "doas_cl is a tool of QDoas, a product jointly developed by BIRA-IASB and S[&]T" << std::endl;
  std::cout << "version: " << cQdoasVersionString << std::endl ;
}

void showHelp()
{
  std::cout << cQdoasVersionString << std::endl << std::endl;
}

int batchProcessQdoas(commands_t *cmd)
{
  TRACE("batchProcessQdoas");

  QList<const CProjectConfigItem*> projectItems;

  int retCode = readConfigQdoas(cmd, projectItems);

  if (retCode)
    return retCode;

  // projectItems is now a list of projects to process ... guaranteed to have at least one.

  TRACE("Num Projects = " <<  projectItems.size());

  while (!projectItems.isEmpty() && retCode == 0) {

    if (!cmd->filenames.isEmpty()) {
      // if files were specified on the command-line, then ignore the files in the project.
      if (projectItems.size() == 1) {

    const CProjectConfigItem *p = projectItems.takeFirst();

    retCode = analyseProjectQdoas(p, cmd->outputDir, cmd->calibDir,cmd->filenames);

    delete p;
      }
      else {
    // error ... dont know which project to use ...
      }
    }
    else {
      // all projects ... all files ...
      const CProjectConfigItem *p = projectItems.takeFirst();

      retCode = analyseProjectQdoas(p, cmd->outputDir,cmd->calibDir);

      delete p;
    }
  }

  // just cleanup
  while (!projectItems.isEmpty())
    delete projectItems.takeFirst();

  return retCode;
}

int readConfigQdoas(commands_t *cmd, QList<const CProjectConfigItem*> &projectItems)
{
  // read the configuration file

  int retCode = 0;

  QFile *file = new QFile(cmd->configFile);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CQdoasConfigHandler *handler = new CQdoasConfigHandler;

  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);   // the xml file is read at this line

  if (ok) {

    CWorkSpace *ws = CWorkSpace::instance();
    ws->setConfigFile(cmd->configFile);

    // sites
    const QList<const CSiteConfigItem*> &siteItems = handler->siteItems();
    QList<const CSiteConfigItem*>::const_iterator siteIt = siteItems.begin();
    while (siteIt != siteItems.end()) {

      ws->createSite((*siteIt)->siteName(), (*siteIt)->abbreviation(),
             (*siteIt)->longitude(), (*siteIt)->latitude(), (*siteIt)->altitude());
      ++siteIt;
    }

    // symbols
    const QList<const CSymbolConfigItem*> &symbolItems = handler->symbolItems();
    QList<const CSymbolConfigItem*>::const_iterator symIt = symbolItems.begin();
    while (symIt != symbolItems.end()) {

      ws->createSymbol((*symIt)->symbolName(), (*symIt)->symbolDescription());
      ++symIt;
    }

    // projects - dont need to be in the workspace ... just keep the project items
    QList<const CProjectConfigItem*> tmpItems = handler->takeProjectItems();

    // is a specific project required ...
    if (!cmd->projectName.isEmpty()) {
      // select only the matching project and discard the rest ...
      while (!tmpItems.isEmpty()) {
    const CProjectConfigItem *p = tmpItems.takeFirst();

    if (p->name().toUpper() == cmd->projectName.toUpper())

      {
            if (xmlSwitch)
              QDOASXML_Parse(cmd->xmlCommands,p);

            projectItems.push_back(p);
      }

    else
      delete p;
      }
    }
    else if(xmlSwitch) {
      std::cout << "-xml switch can only be used when processing a single project.  Use switch -a <projectname>" << std::endl;
      retCode = 1;
    } else {

      while (!tmpItems.isEmpty()) {
    projectItems.push_back(tmpItems.takeFirst());
      }
    }

    // are there any projects in the result list (projectItems).
    if (projectItems.isEmpty())
      retCode = 1;

  }
  else {
    std::cout << handler->messages().toStdString() << std::endl;
    retCode = 1;
  }

  delete handler;
  delete source;
  delete file;

  return retCode;
}


int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir, const QString &calibDir, const QList<QString> &filenames)
{
  QString fileFilter="*.*";
  void *engineContext;
  int retCode;

  CBatchEngineController *controller = new CBatchEngineController;

  retCode = analyseProjectQdoasPrepare(&engineContext, projItem, outputDir, calibDir, controller);

  if (retCode)
    return retCode;

  // loop over files ...
  QList<QString>::const_iterator it = filenames.begin();
  while (it != filenames.end()) {
    QFileInfo info(*it);

    if (info.isFile())
        retCode = analyseProjectQdoasFile(engineContext, controller, *it);
       else if (info.isDir())
        retCode=analyseProjectQdoasDirectory(engineContext,controller,info.filePath(),fileFilter,1);
       else
     retCode=analyseProjectQdoasDirectory(engineContext,controller,info.path(),info.fileName(),1);

    ++it;
  }

  // destroy engine
  CEngineResponseMessage *msgResp = new CEngineResponseMessage;

  if (mediateRequestDestroyEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    retCode = 1;
  }

  delete msgResp;

  return retCode;
}

int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir,const QString &calibDir)
{
  void *engineContext;
  int retCode;

  CBatchEngineController *controller = new CBatchEngineController;

  retCode = analyseProjectQdoasPrepare(&engineContext, projItem, outputDir, calibDir, controller);

  if (retCode)
    return retCode;

  // recursive walk of the files in the config

  retCode = analyseProjectQdoasTreeNode(engineContext, controller, projItem->rootNode());

  // destroy engine
  CEngineResponseMessage *msgResp = new CEngineResponseMessage;

  if (mediateRequestDestroyEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    retCode = 1;
  }

  delete msgResp;

  return retCode;
}

int analyseProjectQdoasPrepare(void **engineContext, const CProjectConfigItem *projItem, const QString &outputDir,const QString &calibDir,
                   CBatchEngineController *controller)
{
  CWorkSpace *ws = CWorkSpace::instance();
  int n;
  const mediate_site_t *siteList = ws->siteList(n);
  int retCode = 0;
  CEngineResponseVisual *msgResp = new CEngineResponseVisual;

  // copy the project data and mask out any display flags (do not want
  // the engine to create and return visualization data)

  mediate_project_t projectData = *(projItem->properties()); // blot copy

  // TODO projectData.display.

  if (!outputDir.isEmpty() && outputDir.size() < FILENAME_BUFFER_LENGTH-1) {
    // override the output directory
    strcpy(projectData.output.path, outputDir.toLocal8Bit().data());
  }

  projectData.output.newcalibFlag=calibSaveSwitch;
  
  if (!calibDir.isEmpty() && calibDir.size() < FILENAME_BUFFER_LENGTH-1) {
    // override the output directory
    strcpy(projectData.output.newCalibPath, calibDir.toLocal8Bit().data());
  }

  // create engine
  if (mediateRequestCreateEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    delete msgResp;
    return 1;
  }

  // Retrieve observation sites

  // set project
  if (!retCode &&
     ((mediateRequestSetSites(*engineContext,n,siteList,msgResp)!=0) ||
      (mediateRequestSetProject(*engineContext, &projectData, (!calibSwitch)?THREAD_TYPE_ANALYSIS:THREAD_TYPE_KURUCZ, msgResp)!= 0))) {
    msgResp->process(controller);
    delete msgResp;
    // create a new response ready for the destroy engine context request
    msgResp = new CEngineResponseVisual;
    retCode = 1;
  }

  // set analysis windows
  if (!retCode) {
    const QList<const CAnalysisWindowConfigItem*> awList = projItem->analysisWindowItems();
    int nWindows = awList.size();
    mediate_analysis_window_t *awDataList = new mediate_analysis_window_t[nWindows];
    mediate_analysis_window_t *awCursor = awDataList;

    QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();
    while (awIt != awList.end()) {

         // Do not account for disabled analysis windows

         if ((*awIt)->isEnabled())
          {
        *awCursor = *((*awIt)->properties());
        // mask any display flags ...
        ++awCursor;
       }
      else
       nWindows--;

      ++awIt;
    }
    if (mediateRequestSetAnalysisWindows(*engineContext, nWindows, awDataList, (!calibSwitch)?THREAD_TYPE_ANALYSIS:THREAD_TYPE_KURUCZ, msgResp) != 0) {
      msgResp->process(controller);
      delete msgResp;
      msgResp = new CEngineResponseVisual;
      retCode = 1;
    }
  }

  if (retCode) {
    // cleanup ... destroy the engine
    if (mediateRequestDestroyEngineContext(*engineContext, msgResp) != 0) {
      msgResp->process(controller);
      retCode = 1;
    }

    *engineContext = NULL;
  }

  return retCode;
}

int analyseProjectQdoasFile(void *engineContext, CBatchEngineController *controller, const QString &filename)
{
  int retCode = 0;
  int result, oldResult;

  CEngineResponseBeginAccessFile *beginFileResp = new CEngineResponseBeginAccessFile(filename);

  result = (!calibSwitch)
    ? mediateRequestBeginAnalyseSpectra(engineContext,
                                        CWorkSpace::instance()->getConfigFile().toLocal8Bit().constData(),
                                        filename.toLocal8Bit().constData(), beginFileResp)
    : mediateRequestBeginCalibrateSpectra(engineContext, filename.toLocal8Bit().constData(), beginFileResp);

  beginFileResp->setNumberOfRecords(result);

  beginFileResp->process(controller);
  delete beginFileResp;

  if (result == -1)
    return 1;

  if (verboseMode)
   std::cout << "Processing file " << filename.toStdString() << std::endl;

  oldResult=-1;

  // loop based on the controller ...
  while (!retCode && controller->active() && (result!=oldResult)) {

    CEngineResponseSpecificRecord *resp = new CEngineResponseSpecificRecord;

    oldResult=result;
    result = (!calibSwitch) ? mediateRequestNextMatchingAnalyseSpectrum(engineContext, resp) :
                              mediateRequestNextMatchingCalibrateSpectrum(engineContext, resp);

    if ((result!=0) && (result!=oldResult))                                     // Try to debug
     {
      resp->setRecordNumber(result);

      TRACE("   record : " << result);

      if (result == -1)
        retCode = 1;
      else if (verboseMode)
        std::cout << "  completed record " << result << std::endl;

      resp->process(controller);
     }

    delete resp;
  }

  TRACE("   end file " << retCode);

  return retCode;
}

int analyseProjectQdoasTreeNode(void *engineContext, CBatchEngineController *controller, const CProjectConfigTreeNode *node)
{
  int retCode = 0;

  while (!retCode && node != NULL) {

    TRACE("analyseProjectQdoasTreeNode : " << node->name().toStdString());

    if (node->isEnabled()) {
      switch (node->type()) {
      case CProjectConfigTreeNode::eFile:
    retCode = analyseProjectQdoasFile(engineContext, controller, node->name());
    break;
      case CProjectConfigTreeNode::eFolder:
    retCode = analyseProjectQdoasTreeNode(engineContext, controller, node->firstChild());
    break;
      case CProjectConfigTreeNode::eDirectory:
    retCode = analyseProjectQdoasDirectory(engineContext, controller, node->name(), node->filter(), node->recursive());
    break;
      }
    }

    node = node->nextSibling();
  }

  return retCode;
}

int analyseProjectQdoasDirectory(void *engineContext, CBatchEngineController *controller,
                 const QString &dir, const QString &filter, bool recursive)
{
  TRACE("analyseProjectQdoasDirectory " << dir.toStdString());

  int retCode = 0;
  QFileInfoList entries;
  QFileInfoList::iterator it;

  QDir directory(dir);

  // first consder sub directories ...
  if (recursive) {
    entries = directory.entryInfoList(); // all entries ... but only take directories on this pass

    it = entries.begin();
    while (/*!retCode && */ it != entries.end()) {
      if (it->isDir() && !it->fileName().startsWith('.')) {

        retCode = analyseProjectQdoasDirectory(engineContext, controller, it->filePath(), filter, true);
      }
      ++it;
    }
  }

  // now the files that match the filters
  if (filter.isEmpty())
    entries = directory.entryInfoList();
  else
    entries = directory.entryInfoList(QStringList(filter));

  it = entries.begin();
  while (/* !retCode && */ it != entries.end()) {
    if (it->isFile()) {

      retCode = analyseProjectQdoasFile(engineContext, controller, it->filePath());
    }
    ++it;
  }

  return retCode;
}

int batchProcessConvolution(commands_t *cmd)
{
  TRACE("batchProcessConvolution");

  int retCode = 0;

  QFile *file = new QFile(cmd->configFile);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CConvConfigHandler *handler = new CConvConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);

  if (ok) {
    void *engineContext = NULL;

    CEngineResponseVisual *resp = new CEngineResponseVisual;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_convolution_t properties = *(handler->properties()); // blot copy

    if (xmlSwitch)
     CONVXML_Parse(cmd->xmlCommands,&properties);

    std::cout << "CONVXML_Parse : " << properties.conslit.file.filename << std::endl;

    if (!cmd->outputDir.isEmpty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1) {
      // override the output directory
      strcpy(properties.general.outputFile, cmd->outputDir.toLocal8Bit().data());
    }

    if (mediateXsconvCreateContext(&engineContext, resp) != 0) {
      retCode = 1;
    }
    else {

      const QList<QString> &filenames = cmd->filenames;

      if (!filenames.isEmpty()) {

    // can only process one file (because the output is a file name).

    QList<QString>::const_iterator it = filenames.begin();

    strcpy(properties.general.inputFile, it->toLocal8Bit().data());

    if ((retCode=mediateRequestConvolution(engineContext, &properties, resp))==ERROR_ID_NO)
     retCode = mediateConvolutionCalculate(engineContext,resp);
    resp->process(controller);

    ++it;
    if (it != filenames.end()) {
      // give a warning for the remaining files
      std::cout << "WARNING: Only one file can be processed. Ignoring the file(s)..." << std::endl;
      while (it != filenames.end()) {

        std::cout << "    " << it->toStdString() << std::endl;
        ++it;
      }
    }

      }
      else {
    // use the current input file
    mediateRequestConvolution(engineContext, &properties, resp);
    retCode = mediateConvolutionCalculate(engineContext,resp);
    resp->process(controller);
      }

      if (retCode)
       std::cout << "Convolution tool failed, please check your input";

      if (mediateXsconvDestroyContext(engineContext, resp) != 0) {
    retCode = 1;
      }
    }

    delete resp;
    delete controller;
  }
  else {
    retCode = 1;
  }

  delete handler;
  delete source;
  delete file;

  return retCode;
}

int batchProcessRing(commands_t *cmd)
{
  TRACE("batchProcessRing");

  int retCode = 0;

  QFile *file = new QFile(cmd->configFile);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CRingConfigHandler *handler = new CRingConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);

  if (ok)
   {
    void *engineContext = NULL;

    CEngineResponseVisual *resp = new CEngineResponseVisual;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_ring properties = *(handler->properties()); // blot copy

    if (xmlSwitch)
     RINGXML_Parse(cmd->xmlCommands,&properties);

    if (!cmd->outputDir.isEmpty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1)
     {
         strcpy(properties.outputFile,cmd->outputDir.toLocal8Bit().data());
     }

    if (mediateXsconvCreateContext(&engineContext, resp) != 0)
     {
      retCode = 1;
     }
    else
     {
      const QList<QString> &filenames = cmd->filenames;

      if (!filenames.isEmpty())
       {
           // can only process one file (because the output is a file name).

           QList<QString>::const_iterator it = filenames.begin();

           strcpy(properties.calibrationFile, it->toLocal8Bit().data());

           if ((retCode=mediateRequestRing(engineContext, &properties, resp))==ERROR_ID_NO)
            retCode = mediateRingCalculate(engineContext,resp);
           resp->process(controller);

           ++it;
           if (it != filenames.end())
            {
             // give a warning for the remaining files
             std::cout << "WARNING: Only one file can be processed. Ignoring the file(s)..." << std::endl;
             while (it != filenames.end())
              {
               std::cout << "    " << it->toStdString() << std::endl;
               ++it;
              }
            }
       }
      else
       {
        // use the current input file
           mediateRequestRing(engineContext, &properties, resp);
           retCode = mediateRingCalculate(engineContext,resp);
           resp->process(controller);
       }

      if (retCode)
       std::cout << "Ring tool failed, please check your input";

      if (mediateXsconvDestroyContext(engineContext, resp) != 0)
       {
           retCode = 1;
       }
     }

    delete resp;
    delete controller;
  }
  else {
    retCode = 1;
  }

  delete handler;
  delete source;
  delete file;

  return retCode;
}

int batchProcessUsamp(commands_t *cmd)
{
  TRACE("batchProcessUsamp");

  int retCode = 0;

  QFile *file = new QFile(cmd->configFile);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CUsampConfigHandler *handler = new CUsampConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);

  if (ok)
   {
    void *engineContext = NULL;

    CEngineResponseVisual *resp = new CEngineResponseVisual;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_usamp_t properties = *(handler->properties()); // blot copy

    if (!cmd->outputDir.isEmpty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1)
     {
         char *ptr;
         char  tmpFile[FILENAME_BUFFER_LENGTH];

         strcpy(tmpFile,cmd->outputDir.toLocal8Bit().data());

         if ((ptr=strchr(tmpFile,'_'))!=NULL)
          {
              *ptr++='\0';
              sprintf(properties.outputPhaseOneFile,"%s1_%s",tmpFile,ptr);
              sprintf(properties.outputPhaseTwoFile,"%s2_%s",tmpFile,ptr);
          }
         else
          {
              sprintf(properties.outputPhaseOneFile,"%s_1",tmpFile);
              sprintf(properties.outputPhaseTwoFile,"%s_2",tmpFile);
          }
     }

    if (mediateXsconvCreateContext(&engineContext, resp) != 0)
     {
      retCode = 1;
     }
    else
     {
      const QList<QString> &filenames = cmd->filenames;

      if (!filenames.isEmpty())
       {
           // can only process one file (because the output is a file name).

           QList<QString>::const_iterator it = filenames.begin();

           strcpy(properties.calibrationFile, it->toLocal8Bit().data());

           if ((retCode=mediateRequestUsamp(engineContext, &properties, resp))==ERROR_ID_NO)
            retCode = mediateUsampCalculate(engineContext,resp);
           resp->process(controller);

           ++it;
           if (it != filenames.end())
            {
             // give a warning for the remaining files
             std::cout << "WARNING: Only one file can be processed. Ignoring the file(s)..." << std::endl;
             while (it != filenames.end())
              {
               std::cout << "    " << it->toStdString() << std::endl;
               ++it;
              }
            }
       }
      else
       {
        // use the current input file
           mediateRequestUsamp(engineContext, &properties, resp);
           retCode = mediateUsampCalculate(engineContext,resp);
           resp->process(controller);
       }

      if (retCode)
       std::cout << "Undersampling tool failed, please check your input";

      if (mediateXsconvDestroyContext(engineContext, resp) != 0)
       {
           retCode = 1;
       }
     }

    delete resp;
    delete controller;
  }
  else {
    retCode = 1;
  }

  delete handler;
  delete source;
  delete file;

  return retCode;
}

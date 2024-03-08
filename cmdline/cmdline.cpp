//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  doas_cl command line entry point
//  Name of module    :  cmdline.cpp
//  Program Language  :  C/C++
//  Creation date     :  2007
//
//        Copyright  (C) Belgian Institute for Space Aeronomy (BIRA-IASB)
//                       Avenue Circulaire, 3
//                       1180     UCCLE
//                       BELGIUM
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//      jonasv@aeronomie.be
//      jeroenv@aeronomie.be
//      thomas.danckaert@aeronomie.be
//
//  ----------------------------------------------------------------------------
//
//  MODULE DESCRIPTION
//
//  ----------------------------------------------------------------------------
//
//  HISTORY :
//
//
//  September 2012 :
//
//        Some options of the configuration file in XML format can be changed
//        in the call of the doas_cl command line tool
//
//           doas_cl .... -xml "<xml path>=<xml value>"
//
//        where <xml path> is a path defining the field to modify according to the
//        xml file structure.  Several -xml switches can be used.
//
//        Example for QDOAS :
//
//            .../doas_cl -c <QDOAS config file> -a <project name> -o <output file> -f <file to process>  -xml /project/analysis_window/files/refone=<reference file>
//
//                   where /project/analysis_window/files/refone is the "xml path" for the reference file
//
//             /bira-iasb/projects/DOAS/Programmes/QDOAS/doas_cl -c /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/config/modules_specific/qdoas/stations/VIELSALM/1691_1/STRATO_VIELSALM_1691_1_UVVIS_fv002.xml -o /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/results/qdoas/stations/VIELSALM/1691_1/STRATO_UVVIS/fv002/2023/ESA-FRM4DOAS-L1.QAQC.STRATO.QDOAS-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv002.nc -f /bira-iasb/data/GROUNDBASED/FRM4DOAS/L1_v03.11/L1_validated/stations/VIELSALM/1691_1/fv001/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001.nc -a STRATO_VIELSALM_1691_1_UVVIS -xml /project/analysis_window/files/refone=/bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/1691_1/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001-331-ZS.ref
//
//              With this command, the reference file will be replaced in all analysis windows
//
//             /project/analysis_window/files/refone : /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/VIELSALM_1691_1_UVVIS_ref_20210729.ref replaced by /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/1691_1/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001-331-ZS.ref (w_o3_320_340)
//             /project/analysis_window/files/refone : /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/VIELSALM_1691_1_UVVIS_ref_20210729.ref replaced by /bira-iasb/pdrojects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/1691_1/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001-331-ZS.ref (w_o4_338_370)
//             /project/analysis_window/files/refone : /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/VIELSALM_1691_1_UVVIS_ref_20210729.ref replaced by /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/1691_1/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001-331-ZS.ref (w_no2_411_445)
//             /project/analysis_window/files/refone : /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/VIELSALM_1691_1_UVVIS_ref_20210729.ref replaced by /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/1691_1/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001-331-ZS.ref (w_no2_425_490)
//             /project/analysis_window/files/refone : /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/VIELSALM_1691_1_UVVIS_ref_20210729.ref replaced by /bira-iasb/projects/FRM4DOAS/applications/validation_fv002/data/ref_files/stations/VIELSALM/1691_1/2023/ESA-FRM4DOAS-L1.QAQC-BIRA.IASB-VIELSALM-1691-1-20230410T042536Z-20230410T154557Z-fv001-331-ZS.ref (w_o3_450_540)
//
//        IMPORTANT NOTE : currently, the program supports the change of the following fields from a QDOAS config file
//
//                  /project/selection/sza/min
//                  /project/selection/sza/max
//                  /project/selection/sza/delta
//                  /project/selection/record/min
//                  /project/selection/record/max
//                  /project/selection/cloud/min
//                  /project/selection/cloud/max
//                  /project/selection/geolocation/circle/radius
//                  /project/selection/geolocation/circle/long
//                  /project/selection/geolocation/circle/lat
//                  /project/selection/geolocation/rectangle/west
//                  /project/selection/geolocation/rectangle/east
//                  /project/selection/geolocation/rectangle/south
//                  /project/selection/geolocation/rectangle/north
//                  /project/selection/geolocation/sites/radius
//                  /project/analysis/converge
//                  /project/analysis/max_iterations
//                  /project/instrumental/omi/trackSelection
//                  /project/instrumental/omi/xTrackMode
//                  /project/instrumental/tropomi/trackSelection
//                  /project/instrumental/apex/trackSelection
//                  /project/calibration/line/slfFile
//                  /project/analysis_window/min
//                  /project/analysis_window/max
//                  /project/analysis_window/resol_fwhm
//                  /project/analysis_window/lambda0
//                  /project/analysis_window/refsel
//                  /project/analysis_window/files/refone
//                  /project/analysis_window/files/reftwo
//                  /project/analysis_window/files/residual
//                  /project/analysis_window/files/szacenter
//                  /project/analysis_window/files/szadelta/minlon
//                  /project/analysis_window/files/maxlon
//                  /project/analysis_window/files/minlat
//                  /project/analysis_window/files/maxlat
//                  /project/analysis_window/files/refns
//                  /project/analysis_window/files/cloudfmin
//                  /project/analysis_window/files/cloudfmax
//                  /project/analysis_window/files/maxdoasrefmode
//                  /project/analysis_window/files/scanmode
//
//              Change of any other field should be implemented
//
//        Example for the convolution tool :
//
//            .../doas_cl -c <convolution config file> -xml /con_slit/slit_func/file/file=<slit function file>
//
//                   where /con_slit/slit_func/file/file should modify the name of the slit function file
//
//        IMPORTANT NOTE : currently, the program supports the change of the following fields from a convolution config file
//
//                  /general/calib
//                  /con_slit/slit_func/file/file
//                  /con_slit/slit_func/file/file2
//                  /con_slit/slit_func/file/wveDptFlag
//
//  February 2019 : add -new_irrad, an option specific for GEMS
//
//        This switch should be used with -k (Run calibration mode).
//        When this option is activated, the program
//
//             1. reads irradiance files (GEMS format as irradiance files),
//             2. performs a "Run calib" on the irradiance spectra
//             3. saves the irradiance with the corrected grid
//
//        Example of call :
//
//             ./doas_cl -c <application path>/GEMS/S5_O3_BremenTDS_config_v1_20180613.xml
//                       -k S5_O3
//                       -new_irrad <application path>/GEMS/ref/s5-spectra_convolved_irradiance_v20180613_nonoise_shift_corrected
//                       -f <application path>/GEMS/ref/s5-spectra_convolved_irradiance_v20180613_nonoise_shifted
//                       -o <application path>/Applications/GEMS/ref/s5-spectra_convolved_irradiance_v20180613_nonoise_output
//
//        where :
//
//             -k uses Run Calib instead of Run analysis (the name of the project has to be specified)
//             -new_irrad is a new switch to save the irradiances with the corrected grid
//             -f should apply for this application on a irradiance file instead of a radiance
//
//             -o the name of the output file (as before : shift for individual calibration sub-windows)
//
//        Not that for output files, it's not necessary to add nc extension (it will be automatically added by QDOAS)
//        The s5-spectra_convolved_irradiance_v20180613_nonoise_shift_corrected.nc should contain the same irradiance as the input file but with corrected grid.
//
//
//  April 2023 : add triggering mode (-t switch or -trigger)
//
//        Triggering mode is useful when doas_cl is coupled to the acquisition
//        program to obtain concentrations in near real time.
//
//        doas_cl makes the calibration (see important note below) and then,
//        enters a waiting loop for trigger lists with files to process.
//
//        -f switch should be replaced by -t switch (or -trigger switch)
//        followed by the path with trigger lists.
//
//        Example : qdoas\release\doas_cl -c <config file>
//                                        -a <project name>
//                                        -o <output file> (better to use automatic as file name)
//                                        -t <trigger path>
//
//        1. the acquisition program should save spectra in individual files
//           the name of the files should include an index number or a timestamp
//
//        2. each file is appended to a trigger list with file name as follows
//
//                      trigger_qdoas_<YYYYMMDD_hhmmss>.list
//
//           the frequency of the timestamp YYYYMMDD_hhmmss (every x seconds, every x minutes)
//           is determined by the acquisition program.
//
//        3. every 15 seconds (value currently hard coded), doas_cl checks if there
//           are new trigger lists in the trigger path.
//
//        4. doas_cl processes files contained in each new trigger list found
//        5. doas_cl changes the .list file extension of the trigger list into .ok
//        6. doas_cl creates/updates a trigger_qdoas.tmstmp in the trigger path with
//           the timestamp of the last processed list.
//
//        7. the user is responsible to clean *.ok from the trigger path
//           a reprocessing is always possible after renaming *.ok into *.list and
//           changing the timestamp in trigger_qdoas.tmstmp
//
//        IMPORTANT NOTE :
//
//           this method is very interesting as the calibration is performed
//           at the start of doas_cl.  It's important that the "Ref selection mode"
//           is "File" in the properties of all analysis windows of the project
//
//        Example of use (should be written in one line) :
//
//           qdoas\release\doas_cl -c C:/My_GroundBased_Activities/GB_Campaigns/Seosan/Data/slant_columns/MAXDOAS_Airyx_v01.xml
//                                 -a MAXDOAS_Airyx
//                                 -o C:/My_Applications/Temp/automatic
//                                 -t C:/My_Applications/Temp/trigger
//
//  ----------------------------------------------------------------------------
//
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QXmlStreamReader>
#include <QFile>
#include <QString>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <clocale>
#include <QTextCodec>

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#include "dirent.h"
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

// kbhit function doesn't exist in Linux libraries (implementation found on the web)

bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}
#endif
#include "dir_iter.h"

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


extern "C" {
#include "stdfunc.h"
#include "zenithal.h"
}

#define TRIGGER_DEFAULT_PAUSE  15   // checks the trigger path every 15 sec


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
  UnknownConfig,
  Qdoas,
  Convolution,
  Ring,
  Usamp
};

typedef struct commands
{
  QString configFile;
  QString projectName;
  QString triggerDir;
  QList<QString> filenames;
  QList<QString> xmlCommands;
  QString outputDir;
  QString calibDir;
} commands_t;

// -----------------------------------------------------------------------------
// Triggers
// -----------------------------------------------------------------------------

typedef struct timestamp
 {
  struct date theDate;
  struct time theTime;
 } timestamp_t;

// GetCurrentTimestamp : returns a number of seconds as timestamp for now

double GetCurrentTimestamp(timestamp_t *pTimestamp)
 {
  return ZEN_NbSec(&pTimestamp->theDate,&pTimestamp->theTime,1);
 }

// SaveTimestamp : save in the timestampFile file, a timestamp provided in seconds (Tm) as a YYYYMMDD_hhmmss string

void SaveTimestamp(char *timestampFile,double Tm)
 {
  // Declarations

  char timestampString[256];
  FILE *fp;

  // Initialize the string

  memset(timestampString,0,256);

  if ((fp=fopen(timestampFile,"w+t"))==NULL)
    std::cout << "Can not create " << timestampFile << std::endl;
  else
   {
    // ZEN_Tm2Str : returns the timestamp as a string YYYYMMDD_hhmmss

    fprintf(fp,"%s",ZEN_Tm2Str(&Tm,timestampString));
    fclose(fp);
   }
 }

// GetTimestamp : converts the timestamp string YYYMMDD_hhmmss into a number of seconds

double GetTimestamp(char *timestampString,timestamp_t *pTimestamp)
 {
  // Declarations

  int year,month,day,hour,minute,sec;
  double Tm;

  // Separate the date and time fields from the string YYYMMDD_hhmmss

  sscanf(timestampString,"%4d%02d%02d_%02d%02d%02d",&year,&month,&day,&hour,&minute,&sec);

  pTimestamp->theDate.da_year=(int)year;
  pTimestamp->theDate.da_mon=(char)month;
  pTimestamp->theDate.da_day=(char)day;

  pTimestamp->theTime.ti_hour=(char)hour;
  pTimestamp->theTime.ti_min=(char)minute;
  pTimestamp->theTime.ti_sec=(char)sec;

  // Calculate the timestamp as a number of seconds

  Tm=ZEN_NbSec(&pTimestamp->theDate,&pTimestamp->theTime,0);

  // Return

  return Tm;
 }

// GetLastTimestamp : converts the timestamp string YYYMMDD_hhmmss read from a timestampFile (supposed
//                    to contain the last trigger timestamp) into a number of seconds

double GetLastTimestamp(char *timestampFile,timestamp_t *pTimestamp)
 {
  // Declarations

  FILE *fp=NULL;
  double Tm=0.;
  char timestampString[20];

  // Create timestamp file if it doesn't exist

  if ((fp=fopen(timestampFile,"rt"))==NULL)
   {
    if ((fp=fopen(timestampFile,"w+t"))==NULL)
     std::cout << "Trigger path " << " do not exist !" << std::endl;
    else
     Tm=GetCurrentTimestamp(pTimestamp);
   }

  // Read the timestamp from file
  else
   {
    fseek(fp,0L,SEEK_SET);
    fscanf(fp,"%s",timestampString);
    Tm=GetTimestamp(timestampString,pTimestamp);
   }

  fclose(fp);

  // Return

  return Tm;
 }

// GetFiles : returns the list of files to process
//            these files come from trigger lists in the trigger path with timestamp higher than the provided lastTimestamp

double GetFiles(const QString &triggerPath,QList<QString> &filenames,double lastTimestamp)
 {
  // Declarations

  DIR *hDir=opendir(triggerPath.toLocal8Bit().data());
  struct dirent *fileInfo = NULL;
  char newFileName[DOAS_MAX_PATH_LEN+1],fileToProcess[DOAS_MAX_PATH_LEN+1],*ptr;
  char renameCmd[MAX_ITEM_TEXT_LEN];
  double nowTm,fileTm,newTm;
  timestamp_t nowTimestamp,fileTimestamp;
  FILE *fp,*gp;

  // Initializations

  nowTm=GetCurrentTimestamp(&nowTimestamp)-5;                                   // take 5 seconds security
  newTm=lastTimestamp;

  // Browse files in the trigger path

  while (hDir!=NULL && ((fileInfo=readdir(hDir))!=NULL) )
   {
    sprintf(newFileName,"%s/%s",triggerPath.toLocal8Bit().data(),fileInfo->d_name);

    if (!STD_IsDir(newFileName) &&                                              // not a folder
        !strncmp(fileInfo->d_name,"trigger_qdoas_",14) &&                       // file name starts with trigger_qdoas_
       ((ptr=strrchr(newFileName,'.'))!=NULL) && !strncmp(ptr,".list",5) &&     // file extension is .list
        (ptr-fileInfo->d_name!=29) &&                                           // trigger_qdoas_ should be followed by the timestamp as yyyymmdd_HHMMSS
       ((fileTm=GetTimestamp(fileInfo->d_name+14,&fileTimestamp))>lastTimestamp) &&
        (fileTm<nowTm))
      {
       // Open the trigger list

       if ((fp=fopen(newFileName,"rt"))!=NULL)
        {
         // Browse files in the trigger list

         while (fgets(fileToProcess,DOAS_MAX_PATH_LEN,fp)!= NULL)
          {
           for (ptr=fileToProcess+(strlen(fileToProcess)-1);*ptr=='\n' || *ptr=='\r';ptr--)
            *ptr='\0';

           // If the file exists (it should) and add it to the list of files to process

           if ((gp=fopen(fileToProcess,"rb"))!=NULL)
            {
             filenames.push_back(fileToProcess);
             fclose(gp);
            }
          }
         fclose(fp);
        }

       // Update the last timestamp

       if (fileTm>newTm)
        newTm=fileTm;

        // change the trigger list file extension from .list to .ok

        #ifdef _WIN32
        sprintf(renameCmd,"move %s %s",newFileName,newFileName);
        for (ptr=strchr(renameCmd,'/');ptr!=NULL;ptr=strchr(ptr,'/'))
         {
          *ptr='\\';
          *ptr++;
         }
        #else
        sprintf(renameCmd,"mv %s %s",newFileName,newFileName);
        #endif
        ptr=strrchr(renameCmd,'.');                                            // we know that the extension is .list
        sprintf(ptr,".ok");
        std::cout << renameCmd << std::endl;
        system(renameCmd);
      }

   }

  if (hDir != NULL)
   closedir(hDir);

  // Return

  return newTm;
 }

//-------------------------------------------------------------------
// declarations
//-------------------------------------------------------------------

enum RunMode parseCommandLine(int argc, char **argv, commands_t *cmd);
enum BatchTool requiredBatchTool(const QString &filename);
void showUsage();
void showHelp();
int  batchProcess(commands_t *cmd);

int batchProcessQdoas(commands_t *cmd);
int readConfigQdoas(commands_t *cmd, QList<const CProjectConfigItem*> &projectItems);
int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir, const QString &calibDir, const QList<QString> &filenames);
int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir,const QString &calibDir);
int analyseProjectQdoasTrigger(const CProjectConfigItem *projItem, const QString &outputDir,const QString &calibDir,const QString &triggerDir);
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
int triggerSwitch=0;
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
      // -----------------------------------------------------------------------
      // configuration file ...
      if (!strcmp(argv[i], "-c")) {

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
      // -----------------------------------------------------------------------
      // project name file (analysis mode) ...
      else if (!strcmp(argv[i], "-a")) {
       if (++i < argc && argv[i][0] != '-') {
         fileSwitch=0;
         cmd->projectName = argv[i];
       }
       else {
        runMode = Error;
        std::cout << "Option '-a' requires an argument (project name)." << std::endl;
       }
      }
      // -----------------------------------------------------------------------
      // project name file ...
      else if (!strcmp(argv[i], "-k")) {
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
      // -----------------------------------------------------------------------
      // trigger path ...
      else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "-trigger")) {
       if (++i < argc && argv[i][0] != '-') {
         fileSwitch=0;
         triggerSwitch=1;
         cmd->triggerDir = argv[i];
       }
       else {
         runMode = Error;
         std::cout << "Option '-t' requires an argument (trigger path)." << std::endl;
       }
      }
      // -----------------------------------------------------------------------
      // save irradiances
      else if (!strcmp(argv[i], "-saveref")) {
            calibSaveSwitch=calibSwitch;
            if (!calibSwitch)
              std::cout << "Warning : Option '-saveref' has effect only with '-k' option." << std::endl;
      }
      // -----------------------------------------------------------------------
      // filename to analyze ...
      else if (!strcmp(argv[i], "-f")) {
       if (++i < argc && argv[i][0] != '-') {
         fileSwitch=1;
         cmd->filenames.push_back(argv[i]);
       }
       else {
         runMode = Error;
         std::cout << "Option '-f' requires an argument (filename)." << std::endl;
       }
      }
      // -----------------------------------------------------------------------
      // save new irradiance ...
      else if (!strcmp(argv[i], "-new_irrad")) {
        if (++i < argc && argv[i][0] != '-') {
          calibSaveSwitch=1;
          cmd->calibDir=argv[i];
        }
        else {
          runMode = Error;
          std::cout << "Option '-new_irrad' requires an argument (filename)." << std::endl;
        }
      }
      // -----------------------------------------------------------------------
      // change an option in the xml file ...
      else if (!strcmp(argv[i],"-xml")) {
        if (++i < argc && argv[i][0] != '-') {
          xmlSwitch=1;
          cmd->xmlCommands.push_back(argv[i]);
        }
        else {
          runMode = Error;
          std::cout << "Option '-xml' requires at least an argument (xmlPath=xmlValue)." << std::endl;
        }
      }
      // -----------------------------------------------------------------------
      // verbose mode ...
      else if (!strcmp(argv[i],"-v"))
       verboseMode=1;
      // -----------------------------------------------------------------------
      // output directory ...
      else if (!strcmp(argv[i], "-o")) {
        if (++i < argc && argv[i][0] != '-') {
          fileSwitch=0;
          cmd->outputDir = argv[i];
        }
        else {
          runMode = Error;
          std::cout << "Option '-o' requires an argument (directory)." << std::endl;
        }
      }
      // -----------------------------------------------------------------------
      // help ...
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
  else if (!cmd->filenames.isEmpty() && triggerSwitch)
   {
    std::cout << "Warning : -t/-trigger switch ignored if switch -f is used" << std::endl;
    triggerSwitch=0;
   }

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
  QFile config_file(filename);
  if (!config_file.open(QIODevice::ReadOnly)) {
    return UnknownConfig; // Can't open the file
  }

  QXmlStreamReader reader(&config_file);
  if (!reader.readNextStartElement()) {
    return UnknownConfig; // XML parsing failed
  }

  auto toolname = reader.name();

  if (toolname == "qdoas") {
    return Qdoas;
  } else if (toolname == "convolution") {
    return Convolution;
  } else if (toolname == "ring") {
    return Ring;
  } else if (toolname == "usamp") {
    return Usamp;
  }

  // XML unknown toolname:
  return UnknownConfig;
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
  std::cout << "    -t, -trigger <path=value> : advanced option to trigger the files to process" << std::endl;
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

    if (triggerSwitch)
     {
      const CProjectConfigItem *p = projectItems.takeFirst();
      retCode = analyseProjectQdoasTrigger(p, cmd->outputDir,cmd->calibDir,cmd->triggerDir);


      delete p;
     }
    else if (!cmd->filenames.isEmpty()) {
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

int analyseProjectQdoasTrigger(const CProjectConfigItem *projItem, const QString &outputDir, const QString &calibDir,const QString &triggerDir)
{
  void *engineContext;
  char timestampFile[DOAS_MAX_PATH_LEN+1];
  int retCode;
  int nsec=0;
  timestamp_t last_timestamp;
  double lastTm,newTm;
  QList<QString> filenames;

  CBatchEngineController *controller = new CBatchEngineController;
  sprintf(timestampFile,"%s/trigger_qdoas.tmstmp",triggerDir.toLocal8Bit().data());

  if ((lastTm=GetLastTimestamp(timestampFile,&last_timestamp))>0.5)
   retCode=analyseProjectQdoasPrepare(&engineContext, projItem, outputDir, calibDir, controller);

  filenames.clear();

  if (!retCode)
   {
    // wait for new files

    while (!kbhit())
     {

       if (nsec==0)
        {
         newTm=GetFiles(triggerDir,filenames,lastTm);

         if (newTm-lastTm>0.5)
          {
           lastTm=newTm;

           // loop trigger files ...

           QList<QString>::const_iterator it = filenames.begin();
           while (it != filenames.end())
            {
             QFileInfo info(*it);

             if (info.isFile())
              retCode = analyseProjectQdoasFile(engineContext, controller, *it);

             ++it;
            }

           std::cout << "Found files" << std::endl;
           filenames.clear();
          }
         else
          std::cout << "Wait for trigger list" << std::endl;
        }

       #ifdef _WIN32
       Sleep(1000L);
       #else
       usleep(1000000);
       #endif
       nsec=(nsec+1)%TRIGGER_DEFAULT_PAUSE;
       // retCode = analyseProjectQdoasTreeNode(engineContext, controller, projItem->rootNode());
     }
   }

  SaveTimestamp(timestampFile,lastTm);

  // destroy engine
  CEngineResponseMessage *msgResp = new CEngineResponseMessage;

  if (mediateRequestDestroyEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    retCode = 1;
  }

  delete msgResp;

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

    if (info.isFile()) {
      retCode = analyseProjectQdoasFile(engineContext, controller, *it);
      std::cout << "Process file " << it->toStdString() << ", retCode = " << retCode << std::endl;
    } else if (info.isDir())
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
 // if (verboseMode)
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

  CEngineResponseMessage resp;
  result = mediateRequestStop(engineContext,&resp);
  if (result == -1)
    retCode = 1;

  resp.process(controller);

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

    // std::cout << "CONVXML_Parse : " << properties.conslit.file.filename << std::endl;

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
       std::cout << "Convolution tool failed, please check your input" << std::endl;;

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

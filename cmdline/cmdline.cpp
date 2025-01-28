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
#include <clocale>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>

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
#include "glob_match.hpp"

extern "C" {
#include "stdfunc.h"
#include "zenithal.h"
}

#define TRIGGER_DEFAULT_PAUSE  15   // checks the trigger path every 15 sec

using std::string;
using std::vector;

namespace fs = std::filesystem;

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
  string configFile;
  string projectName;
  string triggerDir;
  vector<string> filenames;
  vector<string> xmlCommands;
  string outputDir;
  string calibDir;
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
    std::cerr << "Can not create " << timestampFile << std::endl;
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
     std::cerr << "Trigger path " << " do not exist !" << std::endl;
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

double GetFiles(const string &triggerPath,vector<string> &filenames,double lastTimestamp)
 {
  // Declarations

  DIR *hDir=opendir(triggerPath.c_str());
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
    sprintf(newFileName,"%s/%s",triggerPath.c_str(),fileInfo->d_name);

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

       if (fileTm>newTm) {
         newTm=fileTm;
       }

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
enum BatchTool requiredBatchTool(const string &filename);
void showUsage();
void showHelp();
int  batchProcess(commands_t *cmd);

int batchProcessQdoas(commands_t *cmd);
int readConfigQdoas(commands_t *cmd, vector<CProjectConfigItem> &projectItems);
int analyseProjectQdoasPrepare(void **engineContext, const CProjectConfigItem *projItem, const string &outputDir,const string &calibDir,
                   CBatchEngineController *controller);

int batchProcessConvolution(commands_t *cmd);
int batchProcessRing(commands_t *cmd);
int batchProcessUsamp(commands_t *cmd);

int calibSwitch=0;
int calibSaveSwitch=0;
int xmlSwitch=0;
int triggerSwitch=0;
int verboseMode=0;


class QdoasBatch {

public:
  QdoasBatch(const CProjectConfigItem& projItem, const string &outputDir, const string &calibDir, int& rc) :
    projItem(projItem), outputDir(outputDir), calibDir(calibDir), have_enginecontext(false), rc(rc), files_processed(0) {
  };

  ~QdoasBatch() {
    if (have_enginecontext) {
      CEngineResponseMessage msgResp;

      if (mediateRequestDestroyEngineContext(engineContext, &msgResp) != 0) {
        msgResp.process(&controller);
        rc = 1;
      }
    }
  }

  int analyse_project(const string& triggerDir);

  int analyse_project(const vector<string> &filenames);

  int analyse_project() {
    // recursive walk of the files in the config
    return analyse_treeNode(projItem.rootNode());
  }

  int analyse_file(const string &filename);

  int analyse_directory(const string &dir, const string &filter, bool recursive);

  int analyse_treeNode(std::shared_ptr<CProjectConfigTreeNode> node);

private:
  CBatchEngineController controller;
  const CProjectConfigItem& projItem;
  const string &outputDir;
  const string &calibDir;
  void *engineContext;
  bool have_enginecontext;
  int& rc; // for final return code from destructor
  size_t files_processed; // count number of files found for processing
};

//-------------------------------------------------------------------

int main(int argc, char **argv)
{
  int retCode = 0;

  // ----------------------------------------------------------------------------

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
          if (cmd->configFile.empty()) {
            cmd->configFile = argv[i];
            runMode = Batch;
          }
         else
           std::cerr << "Duplicate '-c' option." << std::endl;
        }
        else {
          runMode = Error;
          std::cerr << "Option '-c' requires an argument (configuration file)." << std::endl;
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
        std::cerr << "Option '-a' requires an argument (project name)." << std::endl;
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
         std::cerr << "Option '-k' requires an argument (project name)." << std::endl;
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
         std::cerr << "Option '-t' requires an argument (trigger path)." << std::endl;
       }
      }
      // -----------------------------------------------------------------------
      // save irradiances
      else if (!strcmp(argv[i], "-saveref")) {
            calibSaveSwitch=calibSwitch;
            if (!calibSwitch)
              std::cerr << "Warning : Option '-saveref' has effect only with '-k' option." << std::endl;
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
         std::cerr << "Option '-f' requires an argument (filename)." << std::endl;
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
          std::cerr << "Option '-new_irrad' requires an argument (filename)." << std::endl;
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
          std::cerr << "Option '-xml' requires at least an argument (xmlPath=xmlValue)." << std::endl;
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
          std::cerr << "Option '-o' requires an argument (directory)." << std::endl;
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
      std::cerr << "Invalid argument '" << argv[i] << "'" << std::endl;
    }
    ++i;
  }

  if ((runMode==None) && calibSaveSwitch && !calibSwitch)
   std::cerr << "Warning : -new_irrad switch to use only with -k option; ignored " << std::endl;
  else if (!cmd->filenames.empty() && triggerSwitch)
   {
    std::cerr << "Warning : -t/-trigger switch ignored if switch -f is used" << std::endl;
    triggerSwitch=0;
   }

  // consistency checks ??

  return runMode;
}

//-------------------------------------------------------------------

int batchProcess(commands_t *cmd)
{
  try {
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
      break;
    }
  } catch (std::exception& e) {
    // catch all, report error below
  }
  std::cerr << "Failed to parse configuration file \""
            << cmd->configFile  << "\"" << std::endl;
  return 1;
}

//-------------------------------------------------------------------

enum BatchTool requiredBatchTool(const string &filename)
{
  xmlpp::TextReader reader(filename);
  if (!reader.read()) {
    return UnknownConfig; // XML parsing failed
  }

  auto toolname = reader.get_name();

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
  std::cout <<
    "doas_cl -c <config file> [-a/-k <project name>] [-o <output>] [-f <file>]...\n"
    "\n"
    "    -c <config file>    : A QDoas, convolution, [ring or usamp] config file.\n"
    "                          The tool to invoke is determined from the type of\n"
    "                          configuration file specified;\n"
    "\n"
    "    -a <project name>   : for QDoas, run analysis on the specified project\n"
    "    -k <project name>   : for QDoas, run calibration on the specified project\n"
    "\n"
    "    -new_irrad <output> : for QDoas, run calibration, GEMS measurements, \n"
    "                          calibrated irradiances file\n"
    "\n"
    "    -v                  : verbose on (default is off)\n"
    "\n"
    "    -xml <path=value>   : advanced option to replace the values of some options \n"
    "                          in the configuration file by new ones.\n"
    "    -t, -trigger <path=value> : advanced option to trigger the files to process\n"
    "------------------------------------------------------------------------------\n"
    "doas_cl is a tool of QDoas, a product jointly developed by BIRA-IASB and S[&]T\n"
    "version: " << cQdoasVersionString << std::endl ;
}

void showHelp()
{
  std::cout << cQdoasVersionString << std::endl << std::endl;
}

int batchProcessQdoas(commands_t *cmd)
{
  TRACE("batchProcessQdoas");

  vector<CProjectConfigItem> projectItems;
  int retCode = readConfigQdoas(cmd, projectItems);

  if (retCode)
    return retCode;

  // projectItems is now a list of projects to process ... guaranteed to have at least one.

  TRACE("Num Projects = " <<  projectItems.size());

  int rc_batch = 0;
  for (const auto& config_item : projectItems) {
    QdoasBatch batch(config_item, cmd->outputDir, cmd->calibDir, rc_batch);
    if (triggerSwitch) {
      retCode = batch.analyse_project(cmd->triggerDir);
    } else if (!cmd->filenames.empty()) {
      // if files were specified on the command-line, then ignore the files in the project.
      if (projectItems.size() == 1) { // projectitem was the only project
        retCode = batch.analyse_project(cmd->filenames);
      } else { // multiple projects ~> ambiguous which project the input files belong to
        std::cerr << "ERROR: Configuration contains multiple projects. Use option -a to specify which project to analyse the files with." << std::endl;
        retCode = -1;
      }
    } else {
      retCode = batch.analyse_project();
    }
    if (retCode) {
      break;
    }
  }

  // Check for error from QdoasBatch destructor
  if (rc_batch != 0) {
    retCode = -1;
  }

  return retCode;
}

int readConfigQdoas(commands_t *cmd, vector<CProjectConfigItem>& projectItems)
{
  // read the configuration file

  int retCode = 0;

  CQdoasConfigHandler handler;
  handler.set_substitute_entities(true);

  try {
    handler.parse_file(cmd->configFile);

    CWorkSpace *ws = CWorkSpace::instance();
    ws->setConfigFile(cmd->configFile);

    // sites
    for (const auto& site : handler.siteItems()) {
      ws->createSite(site.siteName(), site.abbreviation(),
                     site.longitude(), site.latitude(), site.altitude());
    }

    // symbols
    for (const auto& symbol : handler.symbolItems()) {
      ws->createSymbol(symbol.symbolName(), symbol.symbolDescription());
    }

    // is a specific project required ...
    if (!cmd->projectName.empty()) {
      // select only the matching project and discard the rest ...
      for (auto &p : handler.projectItems()) {
        if (boost::iequals(p.name(), cmd->projectName)) {
          if (xmlSwitch) {
            QDOASXML_Parse(cmd->xmlCommands,&p);
          }
          projectItems.push_back(std::move(p));
        }
      }
    } else if(xmlSwitch) {
      std::cerr << "-xml switch can only be used when processing a single project.  Use switch -a <projectname>" << std::endl;
      retCode = 1;
    } else {
      for (auto& p : handler.projectItems()) {
        projectItems.push_back(p);
      }
    }

    // are there any projects in the result list (projectItems).
    if (projectItems.empty())
      retCode = 1;
  }
  catch (std::runtime_error& e) {
    std::cerr << "Failed to parse configuration file: " << e.what() << std::endl;
    retCode = 1;
  }

  return retCode;
}

int QdoasBatch::analyse_project(const string& triggerDir) {
  char timestampFile[DOAS_MAX_PATH_LEN+1];
  int nsec = 0;
  int retCode = 0;
  timestamp_t last_timestamp;

  sprintf(timestampFile,"%s/trigger_qdoas.tmstmp",triggerDir.c_str());

  double lastTm=GetLastTimestamp(timestampFile,&last_timestamp);

  vector<string> filenames;
  filenames.clear();

  // wait for new files

  while (!kbhit()) {

    if (nsec==0)
      {
        double newTm=GetFiles(triggerDir,filenames,lastTm);

        if (newTm-lastTm>0.5)
          {
            lastTm=newTm;

            // loop trigger files ...

            for(vector<string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it) {
              if (fs::is_regular_file(*it)) {
                retCode = analyse_file(*it);
              }
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
  }

  SaveTimestamp(timestampFile,lastTm);
  return retCode;
}


int analyseProjectQdoasPrepare(void **engineContext, const CProjectConfigItem *projItem, const string &outputDir,const string &calibDir,
                   CBatchEngineController *controller)
{
  CWorkSpace *ws = CWorkSpace::instance();
  int n;
  std::unique_ptr<const mediate_site_t[]>siteList(ws->siteList(n));
  int retCode = 0;
  CEngineResponseVisual *msgResp = new CEngineResponseVisual;

  // copy the project data and mask out any display flags (do not want
  // the engine to create and return visualization data)

  mediate_project_t projectData = *(projItem->properties()); // blot copy

  // TODO projectData.display.

  if (!outputDir.empty() && outputDir.size() < FILENAME_BUFFER_LENGTH-1) {
    // override the output directory
    strcpy(projectData.output.path, outputDir.c_str());
  }

  projectData.output.newcalibFlag=calibSaveSwitch;

  if (!calibDir.empty() && calibDir.size() < FILENAME_BUFFER_LENGTH-1) {
    // override the output directory
    strcpy(projectData.output.newCalibPath, calibDir.c_str());
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
      ((mediateRequestSetSites(*engineContext,n,siteList.get(),msgResp)!=0) ||
      (mediateRequestSetProject(*engineContext, &projectData, (!calibSwitch)?THREAD_TYPE_ANALYSIS:THREAD_TYPE_KURUCZ, msgResp)!= 0))) {
    msgResp->process(controller);
    delete msgResp;
    // create a new response ready for the destroy engine context request
    msgResp = new CEngineResponseVisual;
    retCode = 1;
  }

  // set analysis windows
  if (!retCode) {
    const auto awList = projItem->analysisWindowItems();
    int nWindows = awList.size();
    mediate_analysis_window_t *awDataList = new mediate_analysis_window_t[nWindows];
    mediate_analysis_window_t *awCursor = awDataList;

    for (auto awIt = awList.begin(); awIt != awList.end(); ++awIt) {
      // Do not account for disabled analysis windows

      if (awIt->isEnabled()) {
        *awCursor = *(awIt->properties());
        // mask any display flags ...
        ++awCursor;
      } else {
        nWindows--;
      }
    }
    int rc = mediateRequestSetAnalysisWindows(*engineContext, nWindows, awDataList, (!calibSwitch)?THREAD_TYPE_ANALYSIS:THREAD_TYPE_KURUCZ, msgResp);
    msgResp->process(controller);
    if (rc != 0) {
      delete msgResp;
      msgResp = new CEngineResponseVisual;
      retCode = 1;
    }
    delete[] awDataList;
  }

  if (retCode) {
    // cleanup ... destroy the engine
    if (mediateRequestDestroyEngineContext(*engineContext, msgResp) != 0) {
      msgResp->process(controller);
      retCode = 1;
    }

    *engineContext = NULL;
  }

  delete msgResp;
  return retCode;
}

int QdoasBatch::analyse_project(const vector<string> &filenames)  {
  // analyse provided list of files
  int retCode = 0;
  CBatchEngineController controller;

  for(auto it = filenames.begin(); it != filenames.end(); ++it) {
    if (fs::is_regular_file(*it)) {
      retCode = analyse_file(*it);
    } else if (fs::is_directory(*it)) {
      retCode = analyse_directory(*it, "*.*", 1);
    } else {  // not an existing file or directory -> assume a filename pattern was provided
              // and recursively search for matching files:
      auto path = fs::path(*it);
      retCode = analyse_directory(path.parent_path(), path.filename(), 1);
      if (files_processed == 0) {
        std::cerr << "ERROR: No files matching pattern " << path.filename()
                  << " in directory " << path.parent_path() << " or subdirectories." << std::endl;
        return -1;
      }
    }
  }

  return retCode;
}

int QdoasBatch::analyse_file(const string &filename) {
  if (verboseMode)
    std::cout << "Processing file " << filename << std::endl;

  ++files_processed;
  int retCode = 0;
  // If this is the first file we process, we still have to run analyseProjectQdoasPrepare()
  if (!have_enginecontext) {
    retCode = analyseProjectQdoasPrepare(&engineContext, &projItem, outputDir, calibDir, &controller);

    if (retCode) {
      return retCode;
    }
    have_enginecontext = true;
  }

  CEngineResponseBeginAccessFile beginFileResp(filename);

  int result = (!calibSwitch)
    ? mediateRequestBeginAnalyseSpectra(engineContext,
                                        CWorkSpace::instance()->getConfigFile().c_str(),
                                        filename.c_str(), &beginFileResp)
    : mediateRequestBeginCalibrateSpectra(engineContext, filename.c_str(), &beginFileResp);

  beginFileResp.setNumberOfRecords(result);
  beginFileResp.process(&controller);

  if (result == -1)
    return 1;

  int oldResult=-1;
  // loop based on the controller ...
  while (!retCode && controller.active() && (result!=oldResult)) {
    CEngineResponseSpecificRecord resp;

    oldResult=result;
    result = (!calibSwitch) ? mediateRequestNextMatchingAnalyseSpectrum(engineContext, &resp) :
      mediateRequestNextMatchingCalibrateSpectrum(engineContext, &resp);
    if ((result!=0) && (result!=oldResult)) {
      resp.setRecordNumber(result);
      if (result == -1)
        retCode = 1;
      else if (verboseMode)
        std::cout << "  completed record " << result << std::endl;
    }
    resp.process(&controller);
  }

  CEngineResponseMessage resp;
  result = mediateRequestStop(engineContext,&resp);
  if (result == -1)
    retCode = 1;

  resp.process(&controller);
  return retCode;
}

int QdoasBatch::analyse_treeNode(std::shared_ptr<CProjectConfigTreeNode> node) {
  int retCode = 0;

  while (!retCode && node != NULL) {
    if (node->isEnabled()) {
      switch (node->type()) {
      case CProjectConfigTreeNode::eFile:
        retCode = analyse_file(node->name());
        break;
      case CProjectConfigTreeNode::eFolder:
        retCode = analyse_treeNode(node->firstChild());
        break;
      case CProjectConfigTreeNode::eDirectory:
        retCode = analyse_directory(node->name(), node->filter(), node->recursive());
        break;
      }
    }
    node = node->nextSibling();
  }

  return retCode;
}

int QdoasBatch::analyse_directory(const string &dir, const string &filter, bool recursive) {
  // analyse all files found in a directory (recursively).  Return an
  // error if no files were processed successfully.
  int result = -1;

  if(!fs::exists(dir)) {
    std::cerr << "ERROR: directory \"" << dir << "\" does not exist." << std::endl;
    return -1;
  }

  // first consider sub directories ...
  if (recursive) {
    for (auto& p : fs::directory_iterator(dir)) {
      if (fs::is_directory(p)) {
        int dir_result = analyse_directory(p.path(), filter, true);
        if (dir_result == 0) {  // got positive result from at least one subdir
          result = 0;
        }
      }
    }
  }

  for (auto &p : fs::directory_iterator(dir)) {
    if (fs::is_regular_file(p) &&
        (filter.empty() || glob_match(filter, string(p.path().filename())))) {
      int file_result = analyse_file(p.path());
      if (file_result == 0) {
        result = 0;
      }
    }
  }

  return result;
}

int batchProcessConvolution(commands_t *cmd)
{
  TRACE("batchProcessConvolution");

  int retCode = 0;

  // parse the file

  CConvConfigHandler handler;
  handler.set_substitute_entities(true);

  try {
    handler.parse_file(cmd->configFile);

    void *engineContext = NULL;

    CEngineResponseVisual *resp = new CEngineResponseVisual;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_convolution_t properties = *(handler.properties()); // blot copy

    if (xmlSwitch)
     CONVXML_Parse(cmd->xmlCommands,&properties);

    if (!cmd->outputDir.empty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1) {
      // override the output directory
      strcpy(properties.general.outputFile, cmd->outputDir.c_str());
    }

    if (mediateXsconvCreateContext(&engineContext, resp) != 0) {
      retCode = 1;
    }
    else {

      const vector<string> &filenames = cmd->filenames;

      if (!filenames.empty()) {

    // can only process one file (because the output is a file name).

    auto it = filenames.begin();

    strcpy(properties.general.inputFile, it->c_str());

    if ((retCode=mediateRequestConvolution(engineContext, &properties, resp))==ERROR_ID_NO)
     retCode = mediateConvolutionCalculate(engineContext,resp);
    resp->process(controller);

    ++it;
    if (it != filenames.end()) {
      // give a warning for the remaining files
      std::cout << "WARNING: Only one file can be processed. Ignoring the file(s)..." << std::endl;
      while (it != filenames.end()) {

        std::cout << "    " << *it << std::endl;
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
  catch (std::runtime_error &e) {
    std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
    retCode = 1;
  }

  return retCode;
}

int batchProcessRing(commands_t *cmd)
{
  TRACE("batchProcessRing");

  int retCode = 0;

  // parse the file

  CRingConfigHandler handler;
  handler.set_substitute_entities(true);

  try {
    handler.parse_file(cmd->configFile);
    void *engineContext = NULL;

    CEngineResponseVisual *resp = new CEngineResponseVisual;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_ring properties = *(handler.properties()); // blot copy

    if (xmlSwitch)
     RINGXML_Parse(cmd->xmlCommands,&properties);

    if (!cmd->outputDir.empty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1)
     {
       strcpy(properties.outputFile,cmd->outputDir.c_str());
     }

    if (mediateXsconvCreateContext(&engineContext, resp) != 0)
     {
      retCode = 1;
     }
    else
     {
      const vector<string> &filenames = cmd->filenames;

      if (!filenames.empty())
       {
         // can only process one file (because the output is a file name).

         auto it = filenames.begin();

           strcpy(properties.calibrationFile, it->c_str());

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
               std::cout << "    " << *it << std::endl;
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
  catch(std::runtime_error &e) {
    std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
    retCode = 1;
  }

  return retCode;
}

int batchProcessUsamp(commands_t *cmd)
{
  TRACE("batchProcessUsamp");

  int retCode = 0;

  // parse the file

  CUsampConfigHandler handler;
  handler.set_substitute_entities(true);

  try {
    handler.parse_file(cmd->configFile);

    void *engineContext = NULL;

    CEngineResponseVisual *resp = new CEngineResponseVisual;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_usamp_t properties = *(handler.properties()); // blot copy

    if (!cmd->outputDir.empty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1)
     {
         char *ptr;
         char  tmpFile[FILENAME_BUFFER_LENGTH];

         strcpy(tmpFile,cmd->outputDir.c_str());

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
      const vector<string> &filenames = cmd->filenames;

      if (!filenames.empty())
       {
           // can only process one file (because the output is a file name).

           auto it = filenames.begin();

           strcpy(properties.calibrationFile, it->c_str());

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
               std::cout << "    " << *it << std::endl;
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
       std::cerr << "Undersampling tool failed, please check your input";

      if (mediateXsconvDestroyContext(engineContext, resp) != 0)
       {
           retCode = 1;
       }
     }

    delete resp;
    delete controller;
  }
  catch(std::runtime_error &e) {
    std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
    retCode = 1;
  }


  return retCode;
}

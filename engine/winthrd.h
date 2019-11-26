#ifndef WINTHRD_H
#define WINTHRD_H

#include "doas.h"

extern char      THRD_asciiFile[];             // ASCII file for exporting spectra
extern void *    THRD_hEvents[];               // list of events
extern unsigned int THRD_id;                   // thread identification number
extern int       THRD_levelMax;                // level of thread
extern int       THRD_lastEvent;               // last event
extern unsigned long THRD_delay;               // wait for next event
extern double    THRD_localShift;
extern int       THRD_correction;
extern int       THRD_browseType;
extern int       THRD_treeCallFlag;
extern int       THRD_increment;
extern int       THRD_isFolder;
extern int       THRD_recordLast;

enum _thrdBrowse
 {
  THREAD_BROWSE_SPECTRA,
  THREAD_BROWSE_DARK,
  THREAD_BROWSE_ERROR,
  THREAD_BROWSE_EXPORT,
  THREAD_BROWSE_MFC_OFFSET,
  THREAD_BROWSE_MFC_DARK,
  THREAD_BROWSE_MFC_INSTR,
  THREAD_BROWSE_MFC_LAMP
 };

enum _thrdLevels
 {
  THREAD_LEVEL_RECORD,
  THREAD_LEVEL_FILE,
  THREAD_LEVEL_PROJECT,
  THREAD_LEVEL_MAX
 };

enum _threadEvents
 {
  THREAD_EVENT_FIRST,
  THREAD_EVENT_PREVIOUS,
  THREAD_EVENT_NEXT,
  THREAD_EVENT_LAST,
  THREAD_EVENT_GOTO,
  THREAD_EVENT_PLAY,
  THREAD_EVENT_PAUSE,
  THREAD_EVENT_STOP,
  THREAD_EVENT_STOP_PROGRAM,
  THREAD_EVENT_MAX
 };

enum _thrdGoto
 {
  THREAD_GOTO_RECORD,
  THREAD_GOTO_PIXEL
 };

// ----------
// PROTOTYPES
// ----------

double           THRD_GetDist(double longit, double latit, double longitRef, double latitRef);
RC               THRD_SpectrumCorrection(ENGINE_CONTEXT *pEngineContext,double *spectrum, const int n_wavel);
RC               THRD_CopySpecInfo(ENGINE_CONTEXT *pSpecInfoTarget,ENGINE_CONTEXT *pSpecInfoSource);
RC               THRD_NewRef(ENGINE_CONTEXT *pEngineContext);

#endif

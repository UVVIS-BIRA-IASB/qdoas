/*! \file output_ascii.c \brief Functions for ascii output.*/

#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "stdfunc.h"
#include "output_common.h"
#include "output.h"
#include "engine_context.h"
#include "omi_read.h"

/*! current output file (NULL if no file is open) */
static FILE *output_file;

static void write_automatic_reference_info(FILE *fp);
static void write_calib_output(FILE *fp);
static void OutputAscPrintTitles(FILE *fp);

static int total_records=0;

/*! \brief Open ascii file */
RC ascii_open(const ENGINE_CONTEXT *pEngineContext, const char *filename) {
  const char *ptr;
  char *filename_dup = NULL;

  if ((ptr=strrchr(filename,PATH_SEP))==NULL)             // avoid problem when dot is used in the directory path as "./<filename>
    ptr=filename;
  else
    ptr++;

  if (strrchr(ptr,'.')==NULL) {
    // If no filename extension is provided, add '.ASC':
    filename_dup = malloc(strlen(filename) + strlen(output_file_extensions[ASCII]) + 1);
    strcpy(filename_dup, filename);
    strcat(filename_dup, output_file_extensions[ASCII]);
    filename = filename_dup;
  }

  const PROJECT *pProject= &pEngineContext->project;

  output_file = fopen(filename, "a+t");
  if (filename_dup != NULL) {
    free(filename_dup);
  }

  if (output_file == NULL) {
    return ERROR_ID_FILE_OPEN;
  }
  int rc = fseek(output_file, 0, SEEK_END);
  if ( rc != 0) {
    return ERROR_ID_FILE_OPEN; // shouldn't happen...
  } else {
    size_t size = ftell(output_file);

    if (size == 0) { // we have a new output file -> print column titles etc
      // Satellites measurements and automatic reference selection : save information on the selected reference
      if(pProject->instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI
         && pEngineContext->analysisRef.refAuto
         && pEngineContext->project.asciiResults.referenceFlag) {
        write_automatic_reference_info(output_file);
      }
      if(calib_num_fields)
        write_calib_output(output_file);

      if (OUTPUT_exportSpectraFlag)
       {
        fprintf(output_file,"# File generated by Qdoas (%s), Royal Belgian Institute for Space Aeronomy (BIRA-IASB), http://uv-vis.aeronomie.be/software/QDOAS\n",cQdoasVersionString);
        fprintf(output_file,"# Size of the detector = %d\n",NDET[0]);    // take the swath size into account ???
        // TO COMPLETE LATER fprintf(output_file,"# Number of records = %-7d\n",0);

        total_records=0;
       }
      else
       {
        fprintf(output_file,"# Results obtained using Qdoas (%s), Royal Belgian Institute for Space Aeronomy (BIRA-IASB), http://uv-vis.aeronomie.be/software/QDOAS\n",cQdoasVersionString);
        OutputAscPrintTitles(output_file);
       }
    }
    else if (OUTPUT_exportSpectraFlag)
     {
      // TODO : update the number of records (in case of append)
     }
  }
  return ERROR_ID_NO;
}

/*! \brief close ascii file.*/

void ascii_close_file(void) {
  assert(output_file != NULL);
  // !!! STILL TO DO -> modify open fseek(output_file, 0, SEEK_SET);
  // !!! STILL TO DO -> modify open fprintf(output_file,"# Number of records = %-7d\n",total_records);

  fclose(output_file);
  output_file = NULL;
}

/*! \brief Print a tab-separated list of the names of the output
    fields or the file header (in case the spectra have to be
    output.)*/
static void OutputAscPrintTitles(FILE *fp){
    fprintf(fp, "# ");
    for(unsigned int i=0; i<output_num_fields; i++) {
      struct output_field thefield = output_data_analysis[i];
      for(unsigned int col=0; col<thefield.data_cols; col++) {
        if (thefield.windowname) {
          fprintf(fp, "%s.", thefield.windowname);
        }
        fprintf(fp, "%s",thefield.fieldname);

        if(thefield.data_cols > 1) {
          // in ascii format: print field name for each column, followed
          // by the column number
          fprintf(fp, thefield.column_number_format,
                  thefield.column_number_alphabetic ? 'A' + col : 1 + col);
        }
        fprintf(fp, "\t");
      }
    }
     fprintf(fp, "\n");
}

/*! \brief Print a single record of an output field to the output
    file, using the correct format string and data type. */
static void print_output_field(FILE *fp, const struct output_field *thefield, int recordno) {
  size_t ncols = thefield->data_cols;

  switch(thefield->memory_type) {
  case OUTPUT_INT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((int *)thefield->data)[recordno * ncols + i]);
    break;
  case OUTPUT_SHORT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((short *)thefield->data)[recordno * ncols + i]);
    break;
  case OUTPUT_USHORT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((unsigned short *)thefield->data)[recordno * ncols + i]);
    break;
  case OUTPUT_STRING:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((char* *)thefield->data)[recordno * ncols + i]);
    break;
  case OUTPUT_FLOAT:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t"))
     if (((float *)thefield->data)[recordno * ncols + i]<9.969e36)
      fprintf(fp,thefield->format, ((float *)thefield->data)[recordno * ncols + i]);
     else
      fprintf(fp,thefield->format,(float)999.999);
    break;
  case OUTPUT_DOUBLE:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) fprintf(fp,thefield->format, ((double *)thefield->data)[recordno * ncols + i]);
    break;
  case OUTPUT_DATE:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) {
      struct date *thedate = &((struct date *)thefield->data)[recordno * ncols + i];
      fprintf(fp, thefield->format,  thedate->da_day, thedate->da_mon, thedate->da_year);
    }
    break;
  case OUTPUT_TIME:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) {
      struct time *thetime = &((struct time *)thefield->data)[recordno * ncols + i];
      fprintf(fp, thefield->format, thetime->ti_hour, thetime->ti_min, thetime->ti_sec );
    }
    break;
  case OUTPUT_DATETIME:
    for(size_t i=0; i<ncols; i++,fprintf(fp,"\t")) {
      struct date *thedate = &((struct datetime *)thefield->data)[recordno * ncols + i].thedate;
      struct time *thetime = &((struct datetime *)thefield->data)[recordno * ncols + i].thetime;
      int millis = ((struct datetime *)thefield->data)[recordno * ncols + i].millis;
      int micros = ((struct datetime *)thefield->data)[recordno * ncols + i].microseconds;
      fprintf(fp, thefield->format, thedate->da_year, thedate->da_mon, thedate->da_day, thetime->ti_hour, thetime->ti_min, thetime->ti_sec,
              (millis != -1) ? millis : micros );
    }
    break;
  case OUTPUT_RESIDUAL:    // in ASCII output format, ignore OUTPUT_RESIDUAL
  default:                 // residuals are not saved in the output file but in a separate file
   break;
  }
}

/*! \brief Write records (data and analysis results).*/

void ascii_write_analysis_data(const bool selected_records[], int num_records) {
  assert(output_file != NULL);
  for(int recordno=0; recordno < num_records; recordno++){
    if(selected_records[recordno]) {
      for(unsigned int i=0; i<output_num_fields; i++ ) {
        print_output_field(output_file, &output_data_analysis[i], recordno);
      }
      fprintf(output_file,"\n");
    }
  }
}

/*! \brief Print the field name when <field name> = <field value> syntax is used
            to save data before spectra */
static void print_output_field_title(FILE *fp,const struct output_field *thefield,int recordno,int col)
 {
  if (thefield->windowname)
   {
    fprintf(fp, "%s.", thefield->windowname);
   }
  fprintf(fp, "%s",thefield->fieldname);

  if (thefield->data_cols > 1)
   {
    // in ascii format: print field name for each column, followed
    // by the column number

    fprintf(fp, thefield->column_number_format,thefield->column_number_alphabetic ? 'A' + col : 1 + col);
   }

  fprintf(fp, " = ");
 }

/*! \brief Print the field value when <field name> = <field value> syntax is used
            to save data before spectra */
static void print_output_field_value(FILE *fp, const struct output_field *thefield, int recordno,int col) {
  size_t ncols = thefield->data_cols;
  struct date *thedate;
  struct time *thetime;
  int millis, micros;

  char strFormat[100];

  strcpy(strFormat,thefield->format);
  if (OUTPUT_exportSpectraFlag)
   STD_StrRep(strFormat,'#','-');


  switch(thefield->memory_type) {
  case OUTPUT_INT:
    fprintf(fp,strFormat, ((int *)thefield->data)[recordno * ncols + col]);
    break;
  case OUTPUT_SHORT:
    fprintf(fp,strFormat, ((short *)thefield->data)[recordno * ncols + col]);
    break;
  case OUTPUT_USHORT:
    fprintf(fp,strFormat, ((unsigned short *)thefield->data)[recordno * ncols + col]);
    break;
  case OUTPUT_STRING:
    fprintf(fp,strFormat, ((char* *)thefield->data)[recordno * ncols + col]);
    break;
  case OUTPUT_FLOAT:
    fprintf(fp,strFormat, ((float *)thefield->data)[recordno * ncols + col]);
    break;
  case OUTPUT_DOUBLE:
    fprintf(fp,strFormat, ((double *)thefield->data)[recordno * ncols + col]);
    break;
  case OUTPUT_DATE:
    thedate = &((struct date *)thefield->data)[recordno * ncols + col];
    fprintf(fp, thefield->format,  thedate->da_day, thedate->da_mon, thedate->da_year);
    break;
  case OUTPUT_TIME:
    thetime = &((struct time *)thefield->data)[recordno * ncols + col];
    fprintf(fp, thefield->format, thetime->ti_hour, thetime->ti_min, thetime->ti_sec );
    break;
  case OUTPUT_DATETIME:
    thedate = &((struct datetime *)thefield->data)[recordno * ncols + col].thedate;
    thetime = &((struct datetime *)thefield->data)[recordno * ncols + col].thetime;
    millis = ((struct datetime *)thefield->data)[recordno * ncols + col].millis;
    micros = ((struct datetime *)thefield->data)[recordno * ncols + col].microseconds;

    if ((strchr(thefield->format,'/')!=NULL) && (strchr(thefield->format,':')!=NULL))
     fprintf(fp, thefield->format, thedate->da_day, thedate->da_mon, thedate->da_year, thetime->ti_hour, thetime->ti_min, thetime->ti_sec);
    else
     fprintf(fp, thefield->format, thedate->da_year, thedate->da_mon, thedate->da_day, thetime->ti_hour, thetime->ti_min, thetime->ti_sec,(millis != -1) ? millis : micros );

    break;
    
  case OUTPUT_RESIDUAL:    // in ASCII output format, ignore OUTPUT_RESIDUAL
  default:                 // residuals are not saved in the output file but in a separate file
   break;    
  }
}

/*! \brief Print a single record of an output field to the output
    file, using the correct format string and data type. The format is <field name>=<field value>*/
static void print_output_field_with_title(FILE *fp, const struct output_field *thefield, int recordno)
 {
  size_t ncols = thefield->data_cols;

  for(size_t i=0; i<ncols; i++,fprintf(fp,"\n"))
   {
    print_output_field_title(fp,thefield,recordno,i);
    print_output_field_value(fp,thefield,recordno,i);
   }
 }

/*! \brief Print the measurement type in the format <field name>=<field value>
           but instead of a integer value, use a string */
static void print_output_meastype(FILE *fp, const struct output_field *thefield, int recordno)
 {
  int meastype=((int (*)[1])thefield->data)[recordno][0];    // only one element

  print_output_field_title(fp,thefield,recordno,0);

  switch(meastype)
   {
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_MAXDOAS_TYPE_AZIMUTH :
    case PRJCT_INSTR_MAXDOAS_TYPE_PRINCIPALPLANE :
    case PRJCT_INSTR_MAXDOAS_TYPE_HORIZON :
    case PRJCT_INSTR_MAXDOAS_TYPE_OFFAXIS :
         fprintf(fp,"OFF\n");
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_MAXDOAS_TYPE_ALMUCANTAR :
         fprintf(fp,"ALM\n");
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_MAXDOAS_TYPE_DIRECTSUN :
         fprintf(fp,"DS\n");
    break;
 // ----------------------------------------------------------------------------
    case PRJCT_INSTR_MAXDOAS_TYPE_ZENITH :
    default :
         fprintf(fp,"ZEN\n");
    break;
 // ----------------------------------------------------------------------------
   }
 }

/*! \brief Print the calibration and/or the spectrum */
static void print_spectra(FILE *fp, const struct output_field *lambda, const struct output_field *spectra,int recordno)
 {
  size_t ncols,i;

  if ((lambda!=NULL) && (spectra!=NULL))
   for(ncols = lambda->data_cols,i=0; i<ncols; i++)
    fprintf(fp,"%#12.6f %#12.6f\n",((double *)lambda->data)[ncols * recordno + i],((double *)spectra->data)[ncols * recordno + i]);
  else if (lambda!=NULL)
   for(ncols = lambda->data_cols,i=0; i<ncols; i++)
    fprintf(fp,"%#12.6f\n",((double *)lambda->data)[ncols * recordno + i]);
  else if (spectra!=NULL)
   for(ncols = spectra->data_cols,i=0; i<ncols; i++)
    fprintf(fp,"%#12.6f\n",((double *)spectra->data)[ncols * recordno + i]);
 }

/*! \brief Save data + calibration and/or spectra */

void ascii_write_spectra_data(const bool selected_records[], int num_records)
 {
  int spectraIndex;
  int lambdaIndex;
  int nrecords;

  spectraIndex=lambdaIndex=ITEM_NONE;
  nrecords=0;

  assert(output_file != NULL);

  for(int recordno=0; recordno < num_records; recordno++)
   {
    if(selected_records[recordno])
     {
      for(unsigned int i=0; i<output_num_fields; i++ )
       {
        // Specific cases (calibration, spectra, and change of formats)

        if (output_data_analysis[i].resulttype==PRJCT_RESULTS_LAMBDA)
         lambdaIndex=i;
        else if (output_data_analysis[i].resulttype==PRJCT_RESULTS_SPECTRA)
         spectraIndex=i;
        else if (output_data_analysis[i].resulttype==PRJCT_RESULTS_MEASTYPE)
         print_output_meastype(output_file, &output_data_analysis[i], recordno);

        // General cases

        else
         print_output_field_with_title(output_file, &output_data_analysis[i], recordno);
       }

      if ((spectraIndex!=ITEM_NONE) && (lambdaIndex!=ITEM_NONE))
       print_spectra(output_file,&output_data_analysis[lambdaIndex],&output_data_analysis[spectraIndex],recordno);
      else if (lambdaIndex!=ITEM_NONE)
       print_spectra(output_file,&output_data_analysis[lambdaIndex],NULL,recordno);
      else if (spectraIndex!=ITEM_NONE)
       print_spectra(output_file,NULL,&output_data_analysis[spectraIndex],recordno+total_records);

      nrecords++;
     }
   }

  total_records+=nrecords;
 }

/*! \brief Reference info for OMI automatic reference selection.

  Write the file name of each L1B file used in the automatic reference
  generation, with a comma-separated list of the spectra from that
  file that were used. */
static void write_automatic_reference_info(FILE *fp)
{
  for(int analysiswindow = 0; analysiswindow < NFeno; analysiswindow++ ){
    for(int row=0; row< ANALYSE_swathSize; ++row ) {
      FENO *pTabFeno = &TabFeno[row][analysiswindow];
      if (!pTabFeno->hidden
          && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
          && pTabFeno->ref_description != NULL) {
        fprintf(fp, "%c %s, row %d: automatic reference:\n%s\n",
                COMMENT_CHAR,
                pTabFeno->windowName,
                row,
                pTabFeno->ref_description);
      }
    }
  }
}

/*! \brief Write calibration data to a file.*/
static void write_calib_output(FILE *fp)
{
  fprintf(fp, "%c ", COMMENT_CHAR);
  for(unsigned int i=0; i<calib_num_fields; i++) {
    struct output_field thefield = output_data_calib[i];

    if (ANALYSE_swathSize > 1)
      fprintf(fp,"Calib(%d/%d).", thefield.index_row+1, ANALYSE_swathSize);
    else
      fprintf(fp,"Calib.");

    if (thefield.windowname) {
      fprintf(fp, "%s.", thefield.windowname);
    }
    fprintf(fp, "%s\t", thefield.fieldname);
  }
  fprintf(fp, "\n");

  int nbWin = KURUCZ_buffers[output_data_calib[0].index_row].Nb_Win; // kurucz settings are same for all detector rows
  for(int recordno=0; recordno < nbWin; recordno++, fprintf(fp,"\n") ){
    fprintf(fp,"%c ", COMMENT_CHAR);
    for(unsigned int i=0; i<calib_num_fields; i++ ) {
      print_output_field(fp, &output_data_calib[i], recordno);
    }
  }
}

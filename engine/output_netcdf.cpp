#include <array>
#include <algorithm>
#include <map>
#include <cassert>
#include <sstream>
#include <ctime>

#include "netcdfwrapper.h"
#include "output_netcdf.h"

extern "C" {
#include "engine_context.h"
#include "omi_read.h"
#include "spectral_range.h"
#include "fit_properties.h"
#include "engine_xsconv.h"
#include "winfiles.h"
}

using std::string;
using std::vector;
using std::array;
using std::map;

static NetCDFFile output_file,output_file_calib;
static NetCDFGroup output_group;

static size_t n_alongtrack, n_crosstrack, n_calib;
static int successFlag;

const static string calib_subgroup_name = "Calib";

// map of dimension-name -> dimension-size.
map<const string, size_t> dimensions = {
  { "date", 3}, // year, month, day
  { "time", 3}, // hour, min, secs
  { "datetime", 7}, // hour, min, secs, milliseconds
  

  // The following numbered dimensions are used for fields with
  // different numbers of columns. for example: fields such as
  // azimuth/longitude/... can contain 3, 4 or 5 values
  { "2", 2},
  { "3", 3},
  { "4", 4},
  { "5", 5},
  { "6", 6},
  { "7", 7},
  { "8", 8},
  { "9", 9} };

enum vartype { Analysis, Calibration};

// Get the id for a given dimension in output_group.  Create the
// dimension if it doesn't exist yet.
static int get_dimid(const string& dim_name) {
  int id;
  int rc = nc_inq_dimid(output_group.groupID(), dim_name.c_str(), &id);
  if (rc == NC_NOERR) {
    return id;
  }

  // The dimension does not exist yet -> create it.
  rc = nc_redef(output_file.groupID()); // Dataset must be put back into "define mode".
  if (rc != NC_NOERR && rc != NC_EINDEFINE) {
    throw std::runtime_error("Cannot create dimension '" + dim_name + "': nc_redef() failed.");
  }
  return output_group.defDim(dim_name, dimensions[dim_name]);
}

static void getDims(NetCDFGroup &group,const struct output_field& thefield, vector<int>& dimids, vector<size_t>& chunksizes) {
  // for dimensions simply numbered "2, 3, ... 9"
  const array<const char*, 8> dim_names { { "2", "3", "4", "5", "6", "7", "8", "9" } };


  if ((thefield.data_cols > 1) && (thefield.memory_type!=OUTPUT_RESIDUAL)) {
    // not suitable for residuals assert(thefield.data_cols < 10);
    dimids.push_back(get_dimid(dim_names[thefield.data_cols -2]));
    chunksizes.push_back(thefield.data_cols);
  }
  switch (thefield.memory_type) {
  break;  
  case OUTPUT_DATE:
    dimids.push_back(get_dimid("date"));
    chunksizes.push_back(3);
    break;
  case OUTPUT_TIME:
    dimids.push_back(get_dimid("time"));
    chunksizes.push_back(3);
    break;
  case OUTPUT_DATETIME:
    dimids.push_back(get_dimid("datetime"));
    chunksizes.push_back(7);
    break;
  default:
    break;
  }
}

static nc_type getNCType(enum output_datatype xtype) {
  switch(xtype) {
  case OUTPUT_STRING:
    return NC_STRING;
    break;
  case OUTPUT_SHORT:
    return NC_SHORT;
    break;
  case OUTPUT_USHORT:
    return NC_USHORT;
    break;
  case OUTPUT_FLOAT:
    return NC_FLOAT;
    break;
  case OUTPUT_INT:
    return NC_INT;
    break;
  case OUTPUT_RESIDUAL:
  case OUTPUT_DOUBLE:
    return NC_DOUBLE;
    break;
  case OUTPUT_TIME:
  case OUTPUT_DATE:
  case OUTPUT_DATETIME:
    return NC_INT; // date/time/datetime are stored as a set of integers in NetCDF
    break;
  default:
  assert(false);
  return 0;
  }
}

static string get_netcdf_varname(const char *varname_in) {
  string result {varname_in};
  std::replace(result.begin(), result.end(), '/', '-');
  return result;
}

static void define_variable(NetCDFGroup &group, const struct output_field& thefield, const string& varname, enum vartype vtype) {

  vector<int> dimids;
  vector<size_t>chunksizes;
  
  if (vtype == Calibration) {
    if (n_crosstrack > 1) {
      dimids.push_back(get_dimid("n_crosstrack"));
      chunksizes.push_back((size_t)n_crosstrack);
    }

    dimids.push_back(get_dimid("n_calib"));
    chunksizes.push_back((size_t)n_calib);
  } else {
    dimids.push_back(get_dimid("n_alongtrack"));
    chunksizes.push_back(std::min<size_t>(1000, n_alongtrack));

    if (n_crosstrack > 1) {
      dimids.push_back(get_dimid("n_crosstrack"));
      chunksizes.push_back((size_t)n_crosstrack);
    }
    
    if (thefield.memory_type==OUTPUT_RESIDUAL)
     {
      string str=group.getName()+".n_datapoint";
      dimensions[str]=thefield.data_cols;
      dimids.push_back(get_dimid(str));
      chunksizes.push_back((size_t)thefield.data_cols);      
     }
    
  }
  
  getDims(group,thefield, dimids, chunksizes);

  const int varid = group.defVar(varname, dimids, getNCType(thefield.memory_type));

   if (thefield.memory_type!=OUTPUT_STRING){
  group.defVarChunking(varid, NC_CHUNKED, chunksizes.data());
    group.defVarDeflate(varid);
   group.defVarFletcher32(varid, NC_FLETCHER32);
   }
  switch (thefield.memory_type) {
  case OUTPUT_STRING:
    group.putAttr("_FillValue", QDOAS_FILL_STRING, varid);
    break;
  case OUTPUT_SHORT:
    group.putAttr("_FillValue", QDOAS_FILL_SHORT, varid);
    break;
  case OUTPUT_USHORT:
    group.putAttr("_FillValue", QDOAS_FILL_USHORT, varid);
    break;
  case OUTPUT_FLOAT:
    group.putAttr("_FillValue", QDOAS_FILL_FLOAT, varid);
    break;
  case OUTPUT_RESIDUAL:  
  case OUTPUT_DOUBLE:
    group.putAttr("_FillValue", QDOAS_FILL_DOUBLE, varid);
    break;
  case OUTPUT_INT:
  case OUTPUT_TIME:     // date/time/datetime are stored as a set of integers in our NetCDF files
  case OUTPUT_DATE:
  case OUTPUT_DATETIME:
    group.putAttr("_FillValue", QDOAS_FILL_INT, varid);
    break;
  default:
    assert(false);
  }
}

static void write_global_attrs(const ENGINE_CONTEXT*pEngineContext, NetCDFGroup &group) {
  static const string qdoas_attr = string("Results obtained using Qdoas (")
    + cQdoasVersionString + "), \n"
    "Belgian Institute for Space Aeronomy (BIRA-IASB)\n"
    "http://uv-vis.bira.be/software/QDOAS";

  group.putAttr("Qdoas", qdoas_attr);

  time_t curtime = time(NULL);
  group.putAttr("CreationTime",string(ctime(&curtime) ) );

  // const char *input_filename = strrchr(pEngineContext->fileInfo.fileName,PATH_SEP);
  // if (input_filename) {
  //  ++input_filename; // if we have found PATH_SEP, file name starts at character behind PATH_SEP
  //} else { // no PATH_SEP found -> just use fileinfo.fileName
  //  input_filename = pEngineContext->fileInfo.fileName;
  // }
  group.putAttr("InputFile", pEngineContext->fileInfo.fileName);  // better to have the full path name
  group.putAttr("QDOASConfig", pEngineContext->project.config_file);
  group.putAttr("QDOASConfigProject", pEngineContext->project.project_name);
}

static void write_calibration_field(const struct output_field& calibfield, NetCDFGroup &group, const string& varname,
                                    const vector<size_t> start, const vector<size_t> count) {

  switch (calibfield.memory_type) {
  case OUTPUT_INT:
    group.putVar(varname, start.data(), count.data(), static_cast<const int *>(calibfield.data));
    break;
  case OUTPUT_SHORT:
    group.putVar(varname, start.data(), count.data(), static_cast<const short *>(calibfield.data));
    break;
  case OUTPUT_USHORT:
    group.putVar(varname, start.data(), count.data(), static_cast<const unsigned short *>(calibfield.data));
    break;
  case OUTPUT_STRING:
    group.putVar(varname, start.data(), count.data(), static_cast<const char **>(calibfield.data));
    break;
  case OUTPUT_FLOAT:
    group.putVar(varname, start.data(), count.data(), static_cast<const float *>(calibfield.data));
    break;
  case OUTPUT_DOUBLE:
    group.putVar(varname, start.data(), count.data(), static_cast<const double *>(calibfield.data));
    break;
  case OUTPUT_DATE:
  case OUTPUT_TIME:
  case OUTPUT_DATETIME:
    assert(false && "date, time or datetime output for calibration not supported");
    break;
  case OUTPUT_RESIDUAL:    // we do not consider to save residuals in the calibration file
  default:                 
   break;    
  }
}

static void write_calibration_data(NetCDFGroup& group) {

  for (unsigned int i=0; i<calib_num_fields; ++i) {
    const struct output_field& calibfield = output_data_calib[i];

    NetCDFGroup calib_group = group.getGroup(calibfield.windowname).getGroup(calib_subgroup_name);

    string varname = get_netcdf_varname(calibfield.fieldname);

    if ( !calib_group.hasVar(varname)) {
      /* when crosstrack dimension is > 1, output_data_calib contains
       * a copy of each calibration field for each detector row.  We
       * want to define one variable with n_crosstrack columns for
       * each calibration field.  -> check if variable already exists
       * and create it if needed. */
      define_variable(calib_group, calibfield, varname, Calibration);
    }

    vector<int> dimids(calib_group.dimIDs(varname));
    vector<size_t> start(dimids.size());
    start[0] = calibfield.index_row;
    vector<size_t> count;
    for (auto dim : dimids) {
      if (n_crosstrack > 1 && dim == get_dimid("n_crosstrack")) {
        count.push_back(1);
      } else { // we want to write across the full extent of each dimension, except crosstrack
        count.push_back(group.dimLen(dim));
      }
    }
    write_calibration_field(calibfield, calib_group, varname, start, count);
  }

}

void write_automatic_reference_info(const ENGINE_CONTEXT *pEngineContext, NetCDFGroup& group) {

  if ( pEngineContext->project.asciiResults.referenceFlag
       && pEngineContext->analysisRef.refAuto ) {
    switch(pEngineContext->project.instrumental.readOutFormat) {
    case PRJCT_INSTR_FORMAT_OMI:
    case PRJCT_INSTR_FORMAT_TROPOMI:
      for(int analysiswindow=0; analysiswindow < NFeno; ++analysiswindow) {
        for(int row=0; row< ANALYSE_swathSize; row++ ) {
          const FENO *pTabFeno = &TabFeno[row][analysiswindow];
          if (!pTabFeno->hidden
              && pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC
              && pTabFeno->ref_description != NULL) {
            std::stringstream attrname;
            attrname << pTabFeno->windowName << " - row " << row << " automatic reference";
            group.putAttr(attrname.str(), pTabFeno->ref_description);
          }
        }
      }
      break;
    case PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF :

      for(int analysiswindow=0; analysiswindow < NFeno; ++analysiswindow)
       {
        const FENO *pTabFeno = &TabFeno[0][analysiswindow];
        if (!pTabFeno->hidden)
         {
          std::stringstream attrname;
          std::stringstream attrdesc;

          attrname << pTabFeno->windowName << " ref mode";

          if (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE)
           attrdesc << "file";
          else if (pTabFeno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA)
           attrdesc << "SZA";
          else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_BEFORE)
           attrdesc << "scan before";
          else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_AFTER)
           attrdesc << "scan after";
          else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_AVERAGE)
           attrdesc << "scans average";
          else if (pTabFeno->refSpectrumSelectionScanMode==ANLYS_MAXDOAS_REF_SCAN_INTERPOLATE)
           attrdesc << "scans interpolate";
          else
           attrdesc << "unknown";

          group.putAttr(attrname.str(), attrdesc.str());
         }
       }

    break;
    default:
      break;
    }
  }
}


void print_metadata_sensor(const ENGINE_CONTEXT *pEngineContext,NetCDFGroup &maingroup){
	string sensor="Sensor";
	vector<string> omi_bands {"UV_1","UV_2","VIS"};
	vector<string> tropomi_bands {"UV_1 / BAND1","UV_2 / BAND2","UVIS_3 / BAND3","UVIS_4 / BAND4","NIR5 / BAND5","NIR6 / BAND6","SWIR7 / BAND7","SWIR8 / BAND8 "};
	vector<string> gome2_bands {"Band_1a","Band_1b","Band_2a","Band_2b","Band_3", "Band_4"};

	if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI){
		maingroup.putAttr(sensor,"OMI");
		maingroup.putAttr("L1 spectral_band",omi_bands[pEngineContext->project.instrumental.omi.spectralType]);
		} 
	if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMPS) maingroup.putAttr(sensor,"OMPS");
	if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI) {
		maingroup.putAttr(sensor,"TROPOMI");
		string band=tropomi_bands[pEngineContext->project.instrumental.tropomi.spectralBand];
		maingroup.putAttr("L1 spectral_band",band);
	}
	if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_SCIA_PDS) maingroup.putAttr(sensor,"SCIAMACHY");
	if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME2){
		maingroup.putAttr(sensor,"GOME-2");
		maingroup.putAttr("L1 spectral_band",gome2_bands[pEngineContext->project.instrumental.user]);
	}
	if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME1_NETCDF) maingroup.putAttr(sensor,"GOME-1");
}

// create a subgroup for each analysis window, and for the calibration
// data belonging to that window
void create_subgroups(const ENGINE_CONTEXT *pEngineContext,NetCDFGroup &group) {

	int row;
	int k=0;
	print_metadata_sensor(pEngineContext, group); 
	for(row=0; row< ANALYSE_swathSize; row++ )
		if (pEngineContext->project.instrumental.use_row[row]){
			break;
		}
	
  for (unsigned int i=0; i<calib_num_fields; ++i) {
    if (group.groupID(output_data_calib[i].windowname) < 0) {
      // group not yet created
		k++;
      auto subgroup = group.defGroup(output_data_calib[i].windowname);
      subgroup.defGroup(calib_subgroup_name);
	  int z=TabFeno[row][k].fit_properties.Z;
	  double  wvl_fitwin_min=TabFeno[row][k].fit_properties.LFenetre[0][0];
	  double  wvl_fitwin_max=TabFeno[row][k].fit_properties.LFenetre[z-1][1];
	  char str1[200];
	  sprintf(str1,"%.3lf : %.3lf",wvl_fitwin_min, wvl_fitwin_max);
	  subgroup.putAttr("fitting window range",str1);
	  
    }
  }

      for(int analysiswindow=0; analysiswindow < NFeno; ++analysiswindow)
       {
        const FENO *pTabFeno = &TabFeno[row][analysiswindow];

        if (!pTabFeno->hidden && (group.groupID(pTabFeno->windowName)<0))
         {
          auto subgroup=group.defGroup(pTabFeno->windowName);
		  int z=TabFeno[row][analysiswindow].fit_properties.Z;
		  double  wvl_fitwin_min=pTabFeno->fit_properties.LFenetre[0][0];
		  double  wvl_fitwin_max=pTabFeno->fit_properties.LFenetre[z-1][1];
		  char str1[200];
		  sprintf(str1,"%.3lf : %.3lf",wvl_fitwin_min, wvl_fitwin_max);
		  subgroup.putAttr("fitting window range",str1);
	  
          CROSS_REFERENCE *pTabCross;

          // needs the wavelength at the center of the fitting window for profiling algorithm

          if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_FRM4DOAS_NETCDF)
           {
            char str[80];
            sprintf(str,"%.3lf",pTabFeno->lambda0);
            subgroup.putAttr("lambda0", str);
           }

          for (int i=0;i<pTabFeno->NTabCross;i++)
           {
            pTabCross=(CROSS_REFERENCE *)&pTabFeno->TabCross[i];

            if ((pTabCross->IndSvdA>0) && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
             subgroup.putAttr(WorkSpace[pTabCross->Comp].symbolName,WorkSpace[pTabCross->Comp].crossFileName);
           }
         }
       }

   for (unsigned int i=0; i<output_num_fields; ++i) {
    // printf("%s\n",output_data_analysis[i].fieldname);
    if (output_data_analysis[i].windowname &&
        group.groupID(output_data_analysis[i].windowname) < 0) {

      group.defGroup(output_data_analysis[i].windowname);
    
    }
  }
}

RC netcdf_open(const ENGINE_CONTEXT *pEngineContext, const char *filename,int num_records) {
  try {
    output_file = NetCDFFile(filename + string(output_file_extensions[NETCDF]), NetCDFFile::Mode::append);
    output_group = output_file.defGroup(pEngineContext->project.asciiResults.swath_name);

    successFlag = (pEngineContext->maxdoasFlag && (pEngineContext->n_crosstrack==1)) ? pEngineContext->project.asciiResults.successFlag:0;

    n_crosstrack = pEngineContext->n_crosstrack; // ANALYSE_swathSize;
    n_alongtrack = (successFlag)?num_records:pEngineContext->n_alongtrack;

    n_calib = 0;
    if ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
        (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_TROPOMI) &&
        (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GEMS) &&
        (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME1_NETCDF))
        n_calib = KURUCZ_buffers[0].Nb_Win;
    else if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GEMS) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI)) {
       for (int firstrow = 0; firstrow<ANALYSE_swathSize; firstrow++)
         // look up the number of calibration windows by searching the first valid entry in KURUCZ_buffers[]
         if (pEngineContext->project.instrumental.use_row[firstrow]) {
           n_calib = KURUCZ_buffers[firstrow].Nb_Win;
           break;
         }
        }
    else // for GOME1_NETCDF format we use second index 1, because 0 is for calibration
       for (int firstrow = 0; firstrow<ANALYSE_swathSize; firstrow++)
         if (TabFeno[firstrow][1].useRefRow) {
           n_calib = KURUCZ_buffers[firstrow].Nb_Win;
           break;
         }

    // Now that we know n_alongtrack, n_crosstrack and n_calib, we can
    // initialize the map of dimension sizes:
    dimensions["n_crosstrack"] = n_crosstrack;
    dimensions["n_alongtrack"] = n_alongtrack;
    dimensions["n_calib"] = n_calib;
    
    create_subgroups(pEngineContext,output_group);
    write_global_attrs(pEngineContext, output_group);
    write_automatic_reference_info(pEngineContext, output_group);
    write_calibration_data(output_group);

   for (unsigned int i=0; i<output_num_fields; ++i) {
      NetCDFGroup group = output_data_analysis[i].windowname ?
        output_group.getGroup(output_data_analysis[i].windowname) : output_group;
       define_variable(group, output_data_analysis[i], get_netcdf_varname(output_data_analysis[i].fieldname), Analysis);
    }
  } catch (std::runtime_error& e) {
    output_file.close();
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
  }
  return 0;
}

void netcdf_close_file() {
  output_file.close();
}

template<typename T, typename U>
inline static void assign_buffer(T buffer, const U& source) {
  *buffer = source;
}

inline static void assign_buffer(int *buffer, const struct time& t) {
  buffer[0] = t.ti_hour;
  buffer[1] = t.ti_min;
  buffer[2] = t.ti_sec;
}

inline static void assign_buffer(int *buffer, const struct date& d) {
  buffer[0] = d.da_year;
  buffer[1] = d.da_mon;
  buffer[2] = d.da_day;
}

inline static void assign_buffer(int *buffer, const struct datetime& d) {
  buffer[0] = d.thedate.da_year;
  buffer[1] = d.thedate.da_mon;
  buffer[2] = d.thedate.da_day;
  buffer[3] = d.thetime.ti_hour;
  buffer[4] = d.thetime.ti_min;
  buffer[5] = d.thetime.ti_sec;
  buffer[6] = (d.millis != -1)
    ? d.millis * 1000 // SCIA: milliseconds->convert to micro
    : d.microseconds; // GOME-2: microseconds
}

template<typename T>
inline static size_t vardimension() {
  return 1;
}

template<>
size_t vardimension<struct date>() {
  return 3;
}

template<>
size_t vardimension<struct time>() {
  return 3;
}

template<>
size_t vardimension<struct datetime>() {
  return 7;
}

static void write_data_field(const struct output_field& datafield, NetCDFGroup &group, const string& varname,
                                    const vector<size_t> start, const vector<size_t> count) {
  switch (datafield.memory_type) {
  case OUTPUT_INT:
    group.putVar(varname, start.data(), count.data(), static_cast<const int *>(datafield.data));
    break;
  case OUTPUT_SHORT:
    group.putVar(varname, start.data(), count.data(), static_cast<const short *>(datafield.data));
    break;
  case OUTPUT_USHORT:
    group.putVar(varname, start.data(), count.data(), static_cast<const unsigned short *>(datafield.data));
    break;
  case OUTPUT_STRING:
    group.putVar(varname, start.data(), count.data(), static_cast<const char **>(datafield.data));
    break;
  case OUTPUT_FLOAT:
    group.putVar(varname, start.data(), count.data(), static_cast<const float *>(datafield.data));
    break;
  case OUTPUT_RESIDUAL:    
  case OUTPUT_DOUBLE:
    group.putVar(varname, start.data(), count.data(), static_cast<const double *>(datafield.data));
    break;
  case OUTPUT_DATE:
  case OUTPUT_TIME:
  case OUTPUT_DATETIME:
    assert(false && "date, time or datetime output for calibration not supported");
    break;
  }
}

template<typename T, typename U = T>
static void write_buffer(const struct output_field *thefield, const bool selected[], int num_records, const OUTPUT_INFO *recordinfo) {

  size_t ncols = thefield->data_cols;
  size_t dimension = vardimension<U>();
  size_t nrecs = (n_crosstrack>1)?n_alongtrack:num_records;

  // variables that depend on the analysis window go the the
  // appropriate subgroup for their analysis window:

  NetCDFGroup group = thefield->windowname ? output_group.getGroup(thefield->windowname) : output_group;

  string varname { get_netcdf_varname(thefield->fieldname) };
  T fill = group.getFillValue<T>(group.varID(varname));

  // buffer will hold all output data for this variable
  vector<T> buffer(n_alongtrack * n_crosstrack * ncols * dimension, fill);

  for (int record=0; record < num_records; ++record) {
    if (selected[record] && (recordinfo[record].i_crosstrack!=ITEM_NONE) && (recordinfo[record].i_alongtrack!=ITEM_NONE)) {

      int i_crosstrack = recordinfo[record].i_crosstrack; // (recordinfo[record].specno-1) % n_crosstrack; //specno is 1-based
      int i_alongtrack = (successFlag)?record:recordinfo[record].i_alongtrack;  // (recordinfo[record].specno-1) / n_crosstrack;

      for (size_t i=0; i< ncols; ++i) {

           // write into the buffer at the correct index position, using
        // the correct layout for the type of data stored:
        int index =  i_alongtrack*n_crosstrack*ncols + i_crosstrack*ncols + i;
        assign_buffer(&buffer[dimension*index], static_cast<U*>(thefield->data)[record*ncols+i]);
      }
    }
  }

  //  vector<int> dimids(output_group.dimIDs(varname));
  //  vector<size_t> start(dimids.size());
  //  start[0] = thefield->index_row;
  //  vector<size_t> count;
  //  for (auto dim : dimids) {
  //    if (n_crosstrack > 1 && dim == get_dimid("n_crosstrack")) {
  //      count.push_back(1);
  //    } else { // we want to write across the full extent of each dimension, except crosstrack
  //      count.push_back(group.dimLen(dim));
  //    }
  //  }
  //
  // write_data_field(*thefield, output_group, varname, start, count);
  group.putVar(varname, buffer.data() );   // This causes erros under Windows

}

// specialization to deal with string variable types...
template<>
void write_buffer<const char*>(const struct output_field *thefield, const bool selected[], int num_records, const OUTPUT_INFO *recordinfo) {

  size_t ncols = thefield->data_cols;

  // variables that depend on the analysis window go the the
  // appropriate subgroup for their analysis window:
  NetCDFGroup group = thefield->windowname ? output_group.getGroup(thefield->windowname) : output_group;
  string varname { get_netcdf_varname(thefield->fieldname) };
  string fill = group.getFillValue<string>(group.varID(varname));

  // buffer will hold all output data for this variable
  vector<const char*> buffer(n_alongtrack * n_crosstrack * ncols, fill.c_str());

  for (int record=0; record < num_records; ++record) {
    if (selected[record] && (recordinfo[record].i_crosstrack!=ITEM_NONE) && (recordinfo[record].i_alongtrack!=ITEM_NONE)) {

      int i_crosstrack = recordinfo[record].i_crosstrack; // (recordinfo[record].specno-1) % n_crosstrack; //specno is 1-based
      int i_alongtrack = (successFlag)?record:recordinfo[record].i_alongtrack; // (recordinfo[record].specno-1) / n_crosstrack;

      for (size_t i=0; i< ncols; ++i) {
        // write into the buffer at the correct index position, using
        // the correct layout for the type of data stored:
        int index = i_alongtrack*n_crosstrack*ncols + i_crosstrack*ncols + i;
        assign_buffer(&buffer[index], static_cast<const char**>(thefield->data)[record*ncols+i]);
      }
    }
  }

  group.putVar(varname, buffer.data() );
}

RC netcdf_write_analysis_data(const bool selected_records[], int num_records, const OUTPUT_INFO *recordinfo) {
  int rc = ERROR_ID_NO;

  try {
    for (unsigned int i=0; i<output_num_fields; ++i) {
      struct output_field *thefield = &output_data_analysis[i];

      switch(thefield->memory_type) {
      case OUTPUT_INT:
        write_buffer<int>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_SHORT:
        write_buffer<short>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_USHORT:
        write_buffer<unsigned short>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_STRING:
        write_buffer<const char*>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_FLOAT:
        write_buffer<float>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_DOUBLE:
        write_buffer<double>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_RESIDUAL:
        write_buffer<double>(thefield, selected_records, num_records, recordinfo);
        break;                
      case OUTPUT_DATE:
        write_buffer<int, struct date>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_TIME:
        write_buffer<int, struct time>(thefield, selected_records, num_records, recordinfo);
        break;
      case OUTPUT_DATETIME:
        write_buffer<int, struct datetime>(thefield, selected_records, num_records, recordinfo);
        break;
      }
    }
  } catch (std::runtime_error& e) {
    rc =  ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }

  return rc;
}

RC netcdf_allow_file(const char *filename, const PRJCT_RESULTS *results) {
  int rc = ERROR_ID_NO;
  try {
    NetCDFFile test(filename, NetCDFFile::Mode::append);
    // check if the group we want to create already exists:
    if (test.groupID(results->swath_name) >= 0 ) {
      rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_OUTPUT_NETCDF, filename, results->swath_name);
    }
  } catch (std::runtime_error& e) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what());
  }
  return rc;
}

RC netcdf_create_calib_var(const char *varname,vector<int>& dimids,vector<size_t>& chunksizes)
 {
  try
   {
    const int varid = output_file_calib.defVar(varname, dimids, NC_DOUBLE);

    output_file_calib.defVarChunking(varid, NC_CHUNKED, chunksizes.data());
    output_file_calib.defVarDeflate(varid);
    output_file_calib.defVarFletcher32(varid, NC_FLETCHER32);


    output_file_calib.putAttr("_FillValue", QDOAS_FILL_DOUBLE, varid);
   }
  catch (std::runtime_error& e)
   {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
   }

  return 0;
 }

RC netcdf_save_calib(double *lambda,double *reference,int indexFenoColumn,int n_wavel)
 {
  const size_t start[] = {0,(size_t)indexFenoColumn};
  const size_t count[] = {(size_t)n_wavel,1};

  try
   {
    output_file_calib.putVar("wavelength", start, count, lambda);
    output_file_calib.putVar("image_pixel_values", start, count, reference);
   }
  catch (std::runtime_error& e)
   {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
   }

  return 0;
 }

RC netcdf_open_calib(const ENGINE_CONTEXT *pEngineContext, const char *filename,int col_dim,int spectral_dim) {
  vector<int> dimids;
  vector<size_t>chunksizes;
  
  FILE *fp;
  
  char new_filename[DOAS_MAX_PATH_LEN+1],error_message[DOAS_MAX_PATH_LEN+1];
  strcpy(new_filename,filename);
  FILES_BuildFileName(new_filename,filename,FILE_TYPE_NETCDF);
  
  if ((fp=fopen(new_filename,"rb"))!=NULL)
   {
    fclose(fp);
    sprintf(error_message,"%s already exists",new_filename);
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, error_message);
   }
  
  try {
    // Open the file in writing mode

    output_file_calib = NetCDFFile(new_filename, NetCDFFile::Mode::write);

    // Create attributes

    time_t curtime = time(NULL);
    output_file_calib.putAttr("created",string(ctime(&curtime) ) );
    output_file_calib.putAttr("description","Solar irradiances with grid corrected by QDOAS");
    output_file_calib.putAttr("title","Solar irradiances with grid corrected by QDOAS");

    // Create dimensions

    dimids.push_back(output_file_calib.defDim("dim_image_band",spectral_dim));
    dimids.push_back(output_file_calib.defDim("dim_image_y",col_dim));

    // Create variables

    chunksizes.push_back(pEngineContext->n_alongtrack);
    chunksizes.push_back(pEngineContext->n_crosstrack);

    netcdf_create_calib_var("wavelength",dimids,chunksizes);
    netcdf_create_calib_var("image_pixel_values",dimids,chunksizes);
  } catch (std::runtime_error& e) {
    output_file_calib.close();
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, e.what() );
  }
  return 0;
}

void netcdf_close_calib() {
  output_file_calib.close();
}



#include "netcdfwrapper.h"

extern "C" {
  #include "comdefs.h"
}

#include <stdexcept>
#include <cassert>
#include <iostream>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

int NetCDFGroup::read_data_fields(struct netcdf_data_fields *new_fields,int n)
 {
  // Allocation

  struct netcdf_data_fields *pField;
  int varSize;
  size_t start[6];
  size_t dimlen;
  int nelemts;
  int rc=ERROR_ID_NO;

  if ((new_fields!=NULL) && n)
   {
    if (data_fields_list!=NULL)
     release_data_fields();

    data_fields_list=new_fields;
    nfields=n;

    for (int i=0;(i<nfields);i++)
     {
      int varId;
      int dimIds[6];

      pField=&data_fields_list[i];
      int groupId=(pField->varGroupName.size()==0)?groupID():groupID(pField->varGroupName);

      if (!nc_inq_varid(groupId, pField->varName.c_str(), &varId) &&
          !nc_inq_varndims(groupId, varId, &pField->varDimsN) &&
          !nc_inq_vardimid(groupId, varId, dimIds))
       {
        switch(pField->varType)
         {
       // ------------------------------------------------------------------------
          case NC_DOUBLE:
            varSize=8;
          break;
       // ------------------------------------------------------------------------
          case NC_SHORT:
            varSize=2;
          break;
       // ------------------------------------------------------------------------
          default:
            varSize=4;
          break;
       // ------------------------------------------------------------------------
         }

        nelemts=1;

        for (int idim=0;idim<pField->varDimsN;idim++)
         {
          nc_inq_dimlen(groupId,dimIds[idim], &dimlen);
          start[idim]=0;
          nelemts*=(int)dimlen;
          pField->varDimsLen[idim]=dimlen;
         }

        if ((pField->varData=(void *)malloc(nelemts*varSize))==NULL)
         rc=ERROR_ID_ALLOC;
        else if (((pField->varType==NC_DOUBLE) && nc_get_vara_double(groupId,varId,(const size_t *)start,(const size_t *)pField->varDimsLen,(double *)pField->varData)) ||
                 ((pField->varType==NC_FLOAT) && nc_get_vara_float(groupId,varId,(const size_t *)start,(const size_t *)pField->varDimsLen,(float *)pField->varData)) ||
                 ((pField->varType==NC_INT) && nc_get_vara_int(groupId,varId,(const size_t *)start,(const size_t *)pField->varDimsLen,(int *)pField->varData)) ||
                 ((pField->varType==NC_SHORT) && nc_get_vara_short(groupId,varId,(const size_t *)start,(const size_t *)pField->varDimsLen,(short *)pField->varData)))
         rc=ERROR_ID_NETCDF;
       }
     }
   }

  // Return

  return rc;
 }

void NetCDFGroup::release_data_fields(void)
 {
  struct netcdf_data_fields *pField;

  if (data_fields_list!=NULL)
   {
    for (int i=0;i<nfields;i++)
     {
      pField=&data_fields_list[i];

      memset(pField->varDimsLen,0,sizeof(size_t)*6);
      pField->varDimsN=0;

      if (pField->varData!=NULL)
       free(pField->varData);
      pField->varData=NULL;
     }
   }

  nfields=0;
 }

int NetCDFGroup::groupID(const string& groupName) const {
  int grpid;
  int rc = nc_inq_grp_full_ncid(groupid, groupName.c_str(), &grpid);
  if(rc == NC_NOERR) {
    return grpid;
  } else {
    return -1;
  }
}

bool NetCDFGroup::hasVar(const string& varName) const {
  int id;
  return (nc_inq_varid(groupid, varName.c_str(), &id) == NC_NOERR);
}

bool NetCDFGroup::hasAttr(const string& attrName, int varid) const {
  int result = nc_inq_att(groupid, varid, attrName.c_str(), nullptr, nullptr);

  if (result == NC_NOERR) {
    return true;
  } else if (result == NC_ENOTVAR) {
    stringstream ss;
    ss << "looking for attribute " << attrName << ", cannot find netCDF variable with id '" << varid << "' in group '"  << name << "'";
    throw std::runtime_error(ss.str());
  } else {
    return false;
  }
  return false;
}

int NetCDFGroup::varID(const string& varName) const {
  int id;
  int rc = nc_inq_varid(groupid, varName.c_str(), &id);

  if(rc == NC_NOERR) {
    return id;
  } else {
    throw std::runtime_error("Cannot find netCDF variable '"+name+"/"+varName+"'");
  }
}

int NetCDFGroup::numDims(int varid) const {
  int ndims;
  if (nc_inq_varndims(groupid, varid, &ndims) == NC_NOERR) {
    return ndims;
  } else {
    throw std::runtime_error("Cannot get number of dimensions for variable '" + varName(varid) + "'");
  }
}

int NetCDFGroup::numDims(const string& varName) const {
  return numDims(varID(varName));
}

vector<int> NetCDFGroup::dimIDs(const std::string &varName) const {
  return dimIDs(varID(varName));
}

vector<int> NetCDFGroup::dimIDs(int varid) const {
  vector<int> dimids(numDims(varid));
  if (nc_inq_vardimid(groupid, varid, dimids.data()) == NC_NOERR) {
    return dimids;
  } else {
    throw std::runtime_error("Cannot get dimension ids for variable '" + varName(varid) + "'");
  }
  return {};
}

string NetCDFGroup::varName(int varid) const {
  char varname[NC_MAX_NAME+1];
  if (nc_inq_varname(groupid, varid, varname) == NC_NOERR) {
    return string(varname);
  } else {
    stringstream ss;
    ss << "Cannot find netCDF variable with id '" << varid << "' in group '"  << name << "'";
    throw std::runtime_error(ss.str());
  }
  return {};
}


bool NetCDFGroup::hasDim(const string& dimName) const {
  int id;
  return (nc_inq_dimid(groupid, dimName.c_str(), &id) == NC_NOERR);
}

int NetCDFGroup::dimID(const string& dimName) const {
  int id;
  int rc = nc_inq_dimid(groupid, dimName.c_str(), &id);

  if(rc == NC_NOERR) {
    return id;
  } else {
    throw std::runtime_error("Cannot find netCDF dimension '"+dimName+"' in group '"+name+"': " + nc_strerror(rc));
  }
}


size_t NetCDFGroup::dimLen(const string& dimName) const {
  return dimLen(dimID(dimName));
}

size_t NetCDFGroup::dimLen(int dimid) const {
  size_t len;
  int rc = nc_inq_dimlen(groupid, dimid, &len);
  if (rc == NC_NOERR) {
    return len;
  } else {
    throw std::runtime_error("Cannot get length of netCDF dimension '"+dimName(dimid)+"' in group '"+name+"': " +  nc_strerror(rc));
  }
}

string NetCDFGroup::dimName(int dimid) const {
  char dimname[NC_MAX_NAME + 1];
  if (nc_inq_dimname(groupid, dimid, dimname) == NC_NOERR) {
    return(dimname);
  } else {
    stringstream ss;
    ss << "Cannot get name of netCDF dimension '" << dimid << "' in group '" << name << "'";
    throw std::runtime_error(ss.str());
  }
}

void NetCDFFile::close() {
  if (groupid) {
    int rc = nc_close(groupid);
    if(rc != NC_NOERR) {
      throw std::runtime_error("Error closing netCDF file '" + filename + "'");
    }
    groupid = 0;
    filename = "";
  }
}

NetCDFFile::NetCDFFile(NetCDFFile &&other) : NetCDFGroup(other), filename(other.filename) {
  other.groupid=0;
  other.filename="";
}

NetCDFFile& NetCDFFile::operator=(NetCDFFile &&other) {
  this->close();
  groupid = other.groupid;
  filename = other.filename;
  other.groupid=0;
  other.filename="";

  return *this;
}

namespace {
  class NCChunkSetting {
  public:
    NCChunkSetting(size_t size=0) {
      if (size == 0) { // size=0: do nothing
        do_reset = false;
        return;
      }

      int rc = nc_get_chunk_cache(&size_old, &n_elems, &preemption);
      if (rc != NC_NOERR) {
        throw std::runtime_error("Failed to get current NetCDF chunck cache settings.");
      }
      rc = nc_set_chunk_cache(size, n_elems, preemption);
      if (rc != NC_NOERR) {
        throw std::runtime_error("Failed to set NetCDF chunk cache.");
      }
      do_reset = true;
    }

    ~NCChunkSetting() {
      if (do_reset) {
        nc_set_chunk_cache(size_old, n_elems, preemption);
      }
    }

  private:
    bool do_reset;
    size_t size_old, n_elems;
    float preemption;
  };
};

int NetCDFFile::openNetCDF(const string &filename, NetCDFFile::Mode mode, size_t chunk_size) {
  NCChunkSetting set_chunks(chunk_size);
  int groupid;
  int rc = NC_NOERR;

  switch (mode) {
  case Mode::read:
    rc = nc_open(filename.c_str(), NC_NOWRITE, &groupid);
    break;
  case Mode::append:
    // Try to open an existing file:
    rc = nc_open(filename.c_str(), NC_WRITE, &groupid);
    if (rc == NC_NOERR ||
        rc == NC_EPERM || rc == NC_ENFILE || rc == NC_ENOMEM || rc == NC_EHDFERR || rc == NC_EDIMMETA) {
      // If nc_open succeeds, or for specific netCDF errors where we
      // do not want to blindly clobber the existing file, we are
      // done:
      break;
    }
    [[fallthrough]]; // All other return codes: assume file does not exist ~> fallthrough to nc_create
  case Mode::write:
    rc = nc_create(filename.c_str(), NC_NETCDF4, &groupid);
    break;
  }

  if (rc != NC_NOERR) {
    throw std::runtime_error("Error opening netCDF file '" + filename + "'");
  }
  return groupid;
}

NetCDFFile::~NetCDFFile() {
  // destructor should not throw exceptions
  try {
    this->close();
  } catch(std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

NetCDFGroup NetCDFGroup::getGroup(const string& groupname) const {
  int id = groupID(groupname);

  if (id >= 0)
    return NetCDFGroup(groupID(groupname), groupname);
  else
    throw std::runtime_error("Cannot open netCDF group '" + groupname + "'");

  return {};
}

NetCDFGroup NetCDFGroup::defGroup(const string& groupname) {
  int newid;
  int rc = nc_def_grp(groupid, groupname.c_str(), &newid);

  if (rc == NC_NOERR) {
    return NetCDFGroup(newid, name + "/" +groupname);
  } else {
    const char *err;
    switch (rc) {
    case NC_ENAMEINUSE:
      err = ", group already exists";
      break;
    case NC_EBADNAME:
      err = ", name contains illegal characters";
      break;
    default:
      err = "";
    }
    throw std::runtime_error("Error creating group '" + groupname + "'" + err);
  }
  return {};
}

int NetCDFGroup::defDim(const string& dimname, size_t len) {
  int dimid;
  int rc = nc_def_dim(groupid, dimname.c_str(), len, &dimid);

  if (rc == NC_NOERR) {
    return dimid;
  } else {
    throw std::runtime_error("Error creating dimension '" + dimname + "'");
  }
}

int NetCDFGroup::defVar(const string& varname, const vector<int>& dimids, nc_type xtype) {
  int varid;
  int rc = nc_def_var(groupid, varname.c_str(), xtype, dimids.size(), dimids.data(), &varid);
  if (rc == NC_NOERR) {
    return varid;
  } else {
    throw std::runtime_error("Error creating variable '" + varname + "' in group '" + name + "'");
  }
}

int NetCDFGroup::defVar(const string& name, const vector<string>& dimnames, nc_type xtype) {
  std::vector<int> dimids;
  dimids.reserve(dimnames.size());
  for (auto &name : dimnames) {
    dimids.push_back(dimID(name));
  }
  return defVar(name, dimids, xtype);
}

void NetCDFGroup::defVarChunking(int varid, int storage, size_t *chunksizes) {
  if (nc_def_var_chunking(groupid, varid, storage, chunksizes) != NC_NOERR) {
    throw std::runtime_error("Error setting variable chunking for '" + varName(varid) + "' in group '" + name + "'");
  }
}

void NetCDFGroup::defVarChunking(const string& name, int storage, size_t *chunksizes) {
  defVarChunking(varID(name), storage, chunksizes);
}

void NetCDFGroup::defVarDeflate(int varid, int shuffle, int deflate, int deflate_level) {
  assert(deflate_level && deflate_level <= 9);
  if (nc_def_var_deflate(groupid, varid, shuffle, deflate, deflate_level) != NC_NOERR) {
    throw std::runtime_error("Error setting variable compression for '" + varName(varid) + "' in group '" + name + "'");
  }
}

void NetCDFGroup::defVarFletcher32(const string& name, int fletcher32) {
  defVarFletcher32(varID(name), fletcher32);
}

void NetCDFGroup::defVarFletcher32(int varid, int fletcher32) {
  if (nc_def_var_fletcher32(groupid, varid, fletcher32) != NC_NOERR) {
    throw std::runtime_error("Error setting Fletcher32 filter for " + varName(varid) + " in group '" + name + "'");
  }
}

void NetCDFGroup::defVarDeflate(const string& name, int shuffle, int deflate, int deflate_level) {
  defVarDeflate(varID(name), shuffle, deflate, deflate_level);
}

string NetCDFGroup::getAttText(const string& name, int varid) {
  size_t len;
  int status = nc_inq_att(groupid, varid, name.c_str(), NULL, &len);
  if (status != NC_NOERR)
    throw std::runtime_error("Error getting data for attribute '" + name + "'");

  vector<char> charbuf(len+1); // len does not include terminating NULL string
  charbuf[len] = '\0';
  status = nc_get_att_text(groupid, varid, name.c_str(), charbuf.data());
  return string(charbuf.data() );
}

double NetCDFGroup::getAttDouble(const string& name, int varid) {
  size_t len;
  double data;
  int status = nc_inq_att(groupid, varid, name.c_str(), NULL, &len);
  if (status != NC_NOERR)
    data=0.;
  else
    status = nc_get_att_double(groupid, varid, name.c_str(), &data);

  return data;
}

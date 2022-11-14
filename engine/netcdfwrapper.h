#ifndef NETCDFWRAPPER_H
#define NETCDFWRAPPER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <memory>

#include <netcdf.h>
// #include "nc4internal.h" /* to get name of the special properties file */

extern "C" {
#include "winthrd.h"
#include "comdefs.h"
#include "stdfunc.h"
#include "engine_context.h"
#include "mediate.h"
#include "analyse.h"
#include "spline.h"
#include "vector.h"
}

typedef unsigned int uint;

struct free_nc_string {
  void operator() (char *string) {
    nc_free_string(1, &string);
  }
};

typedef std::unique_ptr<char,free_nc_string> nc_string;

template<typename T> T default_fillvalue();

// simple wrapper class around NetCDF C API calls.
class NetCDFGroup {

public:
  NetCDFGroup(int id=0, const std::string& groupName ="") :  groupid(id), name(groupName) {};

  bool hasVar(const std::string& varName) const;
//  bool hasGrp(const std::string& grpName) const;
  int varID(const std::string& varName) const;
  int numDims(const std::string& varName) const;
  int numDims(int varid) const;
  std::vector<int> dimIDs(const std::string& varName) const;
  std::vector<int> dimIDs(int varid) const;
  std::string varName(int varid) const;
  int defVar(const std::string& name, const std::vector<int>& dimids, nc_type xtype);
  int defVar(const std::string& name, const std::vector<std::string>& dimnames, nc_type xtype);
  void defVarChunking(int varid, int storage, size_t *chunksizes);
  void defVarChunking(const std::string& name, int storage, size_t *chunksizes);
  void defVarDeflate(int varid, int shuffle=1, int deflate=1, int deflate_level=7);
  void defVarDeflate(const std::string& name, int shuffle=1, int deflate=1, int deflate_level=7);
  void defVarFletcher32(int varid, int fletcher32=NC_FLETCHER32);
  void defVarFletcher32(const std::string& name, int fletcher32=NC_FLETCHER32);
  std::string getAttText(const std::string& attrname, int varid=NC_GLOBAL);

  bool hasAttr(const std::string& name, int varid=NC_GLOBAL) const;

  template<typename T>
  inline std::vector<T> getAttr(const std::string& attr_name, int varid=NC_GLOBAL) const {
    size_t len;
    int rc = nc_inq_attlen (groupid, varid, attr_name.c_str(), &len);
    if (rc != NC_NOERR) {
      std::stringstream ss;
      ss << "Cannot find netCDF attribute '"+attr_name+"' for varid " << varid << " in group " << name;
      throw std::runtime_error(ss.str() );
    }

    std::vector<T> result(len);
    rc = ncGetAttr(varid, attr_name.c_str(), result);
    if (rc != NC_NOERR) {
      std::stringstream ss;
      ss << "Cannot read netCDF attribute '"+attr_name+"' for varid " << varid << " in group " << name;
      throw std::runtime_error(ss.str() );
    } else {
      return result;
    }
  }

  template<typename T>
  inline T getFillValue(int varid) const {
    if (hasAttr("_FillValue", varid)) {
      return getAttr<T>("_FillValue", varid)[0];
    } else {
      return default_fillvalue<T>();
    }
  }

  template<typename T>
  inline T getFillValue(const std::string& var_name) const {
    return getFillValue<T>(varID(var_name));
  }

  template<typename T>
  inline void getVar(int varid, const size_t start[], const size_t count[], T *out) const {
    if (ncGetVar(varid, start, count, out) != NC_NOERR) {
       // return error !!!

      // throw std::runtime_error("Cannot read NetCDF variable '"+name+"/"+varName(varid)+"'");
    } }
  template<typename T>
  inline void getVar(const std::string& name, const size_t start[], const size_t count[], T *out) const {
    getVar(varID(name), start, count, out);
    }

  // This function allocate the vector, initialize it to the default value and if the requested variable exists, retrieves the values

  template<typename T>
  inline void getVar(const std::string& name, const size_t start[], const size_t count[], int num_dims, T fill_value,std::vector<T>& out) const
   {
    // Declarations

    int idim,                                                                     // browse dimensions
        max_dim,                                                                  // number of dimensions (should be the size of start and count vectors
        i;                                                                        // browse elements of vector var

    // Get the number of elements  to allocate

    for (max_dim=1,idim=0;idim<num_dims;idim++)
     max_dim*=count[idim];

    // Allocate the vector

    out.resize(max_dim);

    // Initialize the vector to fill value

    for (i=0;i<max_dim;i++)
     out[i]=fill_value;

    if (hasVar(name))
     getVar(name,start,count,out.data());
   }


  template<typename T>
  inline void putVar(int varid, const size_t start[], const size_t count[], T *in) {
    if (ncPutVar(varid, start, count, in) != NC_NOERR) {
      // !!! throw std::runtime_error("Cannot write NetCDF variable '"+name+"/"+varName(varid)+"'");
    } }
  template<typename T>
  inline void putVar(const std::string& name, const size_t start[], const size_t count[], T *in) {
    putVar(varID(name), start, count, in); }
  template<typename T>
  inline void putVar(int varid, T *in) {
    if (ncPutVar(varid,in) != NC_NOERR) {
     // return error !!!
     // throw std::runtime_error("Cannot write NetCDF variable '"+name+"/"+varName(varid)+"'");
    } }
  template<typename T>
  inline void putVar(const std::string& name, T *in) {
    putVar(varID(name), in); }

  template<typename T>
  inline void putAttr(const std::string& name, size_t len, T*in, int varid=NC_GLOBAL) {
    int rc = ncPutAttr(varid, name.c_str(), len, in);

    if (rc != NC_NOERR) {
      std::stringstream errstream;
      errstream << "Cannot write NetCDF attribute '" << name << "'"
                << (varid == NC_GLOBAL
                    ? ""
                    : " for variable '"+ varName(varid)) << ": ";
      switch (rc) {
      case NC_EINVAL:
        errstream << "NC_EINVAL";
        break;
      case NC_ENOTVAR:
        errstream << "NC_ENOTVAR";
        break;
      case NC_EBADTYPE:
        errstream << "NC_EBADTYPE";
        break;
      case NC_ENOMEM:
        errstream << "NC_ENOMEM";
        break;
      case NC_ELATEFILL:
        errstream << "NC_ELATEFILL";
        break;
      default:
        errstream << rc;
        break;
      }
      // !!! throw std::runtime_error(errstream.str() );
    }
  }
  template<typename T>
  inline void putAttr(const std::string& name, const std::vector<T>& in, int varid=NC_GLOBAL) {
    putAttr(name, in.size(), in.data(), varid);
  }
  template<typename T>
  inline void putAttr(const std::string& name, T in, int varid=NC_GLOBAL) {
    putAttr(name, 1, &in, varid);
  }
  inline void putAttr(const std::string& name, const std::string& value, int varid=NC_GLOBAL) {
    putAttr(name, value.length(), value.c_str(), varid);
  }
  inline void putAttr(const std::string&name, const std::vector<std::string>& strings, int varid=NC_GLOBAL) {
    std::vector<const char*> charvec;
    for (const auto &s : strings) {
      charvec.push_back(s.c_str() );
    }
    putAttr(name, charvec.size(), charvec.data(), varid);
  }
  inline void putAttr(const std::string&name, char *in, int varid=NC_GLOBAL) {
    ncPutAttr(varid, name.c_str(), strlen(in), in);
  }

  int groupID(const std::string& groupName) const;
  int groupID() const {return groupid;} ;
  NetCDFGroup getGroup(const std::string& groupName) const;
  NetCDFGroup defGroup(const std::string& groupName);

  int defDim(const std::string& dimName, size_t len);
  int dimID(const std::string& dimName) const;
  size_t dimLen(const std::string& dimName) const;
  size_t dimLen(int dimid) const;
  std::string dimName(int dimid) const;
  const std::string& getName() const { return name; };

protected:
  int groupid;

private:
  std::string name;

  inline int ncGetVar(int varid, const size_t start[], const size_t count[], float *out) const {
    return nc_get_vara_float(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], double *out) const {
    return nc_get_vara_double(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], int *out) const {
    return nc_get_vara_int(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], uint *out) const {
    return nc_get_vara_uint(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], long *out) const {
    return nc_get_vara_long(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], short *out) const {
    return nc_get_vara_short(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], unsigned short *out) const {
    return nc_get_vara_ushort(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], signed char *out) const {
    return nc_get_vara_schar(groupid, varid, start, count, out); };
  inline int ncGetVar(int varid, const size_t start[], const size_t count[], unsigned char *out) const {
    return nc_get_vara_uchar(groupid, varid, start, count, out); };

  inline int ncPutVar(int varid, const size_t start[], const size_t count[], float const *in) {
    return nc_put_vara_float(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], double const *in) {
    return nc_put_vara_double(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], short const *in) {
    return nc_put_vara_short(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], unsigned short const *in) {
    return nc_put_vara_ushort(groupid, varid, start, count, in); }
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], int const *in) {
    return nc_put_vara_int(groupid, varid, start, count, in); };
  inline int ncPutVar(int varid, const size_t start[], const size_t count[], char const **in ) {
    return nc_put_vara_string(groupid, varid, start, count, in); };

  inline int ncPutVar(int varid, float const *in) {
    return nc_put_var_float(groupid, varid, in); };
  inline int ncPutVar(int varid, double const *in) {
    return nc_put_var_double(groupid, varid, in); };
  inline int ncPutVar(int varid, short const *in) {
    return nc_put_var_short(groupid, varid, in); };
  inline int ncPutVar(int varid, unsigned short const *in) {
    return nc_put_var_ushort(groupid, varid, in); }
  inline int ncPutVar(int varid, int const *in) {
    return nc_put_var_int(groupid, varid, in); };
  inline int ncPutVar(int varid, char const **in ) {
    return nc_put_var_string(groupid, varid, in); };

  inline int ncPutAttr(int varid, char const *name, size_t len, const char *text) {
    return nc_put_att_text(groupid, varid, name, len, text);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const float *d) {
    return nc_put_att_float(groupid, varid, name, NC_FLOAT, len, d);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const double *d) {
    return nc_put_att_double(groupid, varid, name, NC_DOUBLE, len, d);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const short *d) {
    return nc_put_att_short(groupid, varid, name, NC_SHORT, len, d);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const unsigned short *d) {
    return nc_put_att_ushort(groupid, varid, name, NC_USHORT, len, d);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const int *i) {
    return nc_put_att_int(groupid, varid, name, NC_INT, len, i);
  }
  inline int ncPutAttr(int varid, char const *name, size_t len, const char **s) {
    return nc_put_att_string(groupid, varid, name, len, s);
  }

  // get string attributes as vector of std::strings (somewhat wasteful).
  inline int ncGetAttr(int varid, char const *name, std::vector<std::string>& strings) const {
    std::vector<char *> c_strings(strings.size());
    int rc = nc_get_att_string(groupid, varid, name, c_strings.data());
    if (rc == NC_NOERR) {
      for (size_t i=0; i<strings.size(); ++i) {
        strings[i] = c_strings[i];
      }
    }
    nc_free_string(c_strings.size(),c_strings.data());
    return rc;
  }

  // get string attributes as vector of nc_strings.
  inline int ncGetAttr(int varid, char const *name, std::vector<nc_string>& strings) const {
    std::vector<char *> c_strings(strings.size());
    int rc = nc_get_att_string(groupid, varid, name, c_strings.data());
    if (rc == NC_NOERR) {
      for (size_t i=0; i<strings.size(); ++i) {
        strings[i] = nc_string(c_strings[i]);
      }
    } else {
      nc_free_string(c_strings.size(),c_strings.data());
    }
    return rc;
  }

  inline int ncGetAttr(int varid, char const *name, std::vector<char>& text) const {
    return nc_get_att_text(groupid, varid, name, text.data());
  }
  inline int ncGetAttr(int varid, char const *name, std::vector<float>& d) const {
    return nc_get_att_float(groupid, varid, name, d.data());
  }
  inline int ncGetAttr(int varid, char const *name, std::vector<double>& d) const {
    return nc_get_att_double(groupid, varid, name, d.data());
  }
  inline int ncGetAttr(int varid, char const *name, std::vector<short>& d) const {
    return nc_get_att_short(groupid, varid, name, d.data());
  }
  inline int ncGetAttr(int varid, char const *name, std::vector<unsigned short>& d) const {
    return nc_get_att_ushort(groupid, varid, name, d.data());
  }
  inline int ncGetAttr(int varid, char const *name, std::vector<int>& i) const {
    return nc_get_att_int(groupid, varid, name, i.data());
  }

};

// NetCDF file with root group.
class NetCDFFile : public NetCDFGroup {
public:
  NetCDFFile(const std::string& fileName, int mode=NC_NOWRITE);
  NetCDFFile() : filename() {};
  ~NetCDFFile();

  NetCDFFile(NetCDFFile&& other);
  NetCDFFile& operator=(NetCDFFile&& other);

  const std::string& getFile() const { return filename; };

  NetCDFFile(const NetCDFFile& that) = delete; // prevent copying
  NetCDFFile& operator=(NetCDFFile& other) = delete; // prevent assignment

  void close();

private:
  std::string filename;
};

template<>
inline double default_fillvalue() {
  return NC_FILL_DOUBLE;
}

template<>
inline float default_fillvalue() {
  return NC_FILL_FLOAT;
}

template<>
inline int default_fillvalue() {
  return NC_FILL_INT;
}

template<>
inline short default_fillvalue() {
  return NC_FILL_SHORT;
}

template<>
inline unsigned short default_fillvalue() {
  return NC_FILL_USHORT;
}

template<>
inline char default_fillvalue() {
  return NC_FILL_CHAR;
}

template<>
inline const char* default_fillvalue() {
  return NC_FILL_STRING;
}

template<>
inline std::string default_fillvalue() {
  return NC_FILL_STRING;
}

#endif

// Local Variables:
// mode: c++
// End:

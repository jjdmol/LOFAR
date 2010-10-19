// Emacs, this is -*- c++ -*- code.
/* Copyright © 2000 
Michael Gradman and Corwin Joy 

Permission to use, copy, modify, distribute and sell this software and 
its documentation for any purpose is hereby granted without fee, provided 
that the above copyright notice appears in all copies and that both that 
copyright notice and this permission notice appear in supporting documentation. 
Corwin Joy and Michael Gradman make no representations about the suitability 
of this software for any purpose. 
It is provided "as is" without express or implied warranty.
*/ 
// class which abstracts the binding of columns and parameters
// Initial: ca. 9/5/2000 - MG
// Revised: 10/6/2000 - MG - added support for relative offsets
// Revised: 11/12/2000 - CJ - wrote faster GenericCmp function
// Edited: 12/19/2000 - MG - added namespaces
// Edited: 03/24/2004 - Alexander Motzkau, BoundIO remembers variant_row index

#ifndef DTL_BOUNDIO_H
#define DTL_BOUNDIO_H

#include "DB_Base.h"
#include "bind_basics.h"
#include "variant_row.h"
#include "DBException.h"
#include "string_util.h"

#include "std_warn_off.h"
#include <string>
#include <set>

#ifdef  WIN32
#ifdef WIN32
	#ifndef DTL_USE_MFC
		#include <windows.h>
	#else 
		#include <afx.h>
	#endif
#endif

#endif
#include <sqltypes.h>

#include "std_warn_on.h"

// using namespace std;

BEGIN_DTL_NAMESPACE

// extern const ETI_Map SQL_types_to_C;
class BoundIOs;
size_t BoundIOs_NumColumns(BoundIOs *pB);

char *data_ptr(variant_row &vr);

class GenerateBoundIOs;

template<class DataObj> class LocalBCA;

#if 0
// ***** copies *src to *dest by value: ******
// (should have been a member of BoundIO, but wrong form of function gets called
// when the specialization should have been called for char *'s
// generic: *dest = *src
// char *: std_strcpy(*dest, *src)
template<class DataField> 
	void MakeActualCopyOfMember(DataField *dest, const DataField *src, int typeId)
{
	*dest = *src;
}

inline void MakeActualCopyOfMember<TCHAR *>
	(TCHAR **dest, const TCHAR * const* src, int typeId)
{

		// N.B. We do *not* use *dest below.
		// DataObj address in case of binding to TCHAR[] member
		// actully gives pointer into DataObj not pointer-to-pointer
		std_tstrcpy((TCHAR *)dest, *src);
}
#endif

// workaround for the wrong version of MakeActualCopyOfMember getting called

// a specialization for TCHAR * should have made the second version above to get called
// instead, the templated version was getting invoked

inline void MakeActualCopyOfMember(void *dest, const void *src) 
{
	std_tstrcpy((TCHAR *) dest, 
				(const TCHAR *) src);
}

#define DTL_MEMBERCOPY(type) inline void MakeActualCopyOfMember(type *dest, const type *src){*dest = *src;}

// cast of thousands
DTL_MEMBERCOPY(short);
DTL_MEMBERCOPY(int);
DTL_MEMBERCOPY(unsigned int);
DTL_MEMBERCOPY(long);
DTL_MEMBERCOPY(unsigned long);
DTL_MEMBERCOPY(double);
DTL_MEMBERCOPY(float);
DTL_MEMBERCOPY(TIMESTAMP_STRUCT);
DTL_MEMBERCOPY(blob);
DTL_MEMBERCOPY(jtime_c);
DTL_MEMBERCOPY(bool);
DTL_MEMBERCOPY(TCHAR);
DTL_MEMBERCOPY(tstring);
DTL_MEMBERCOPY(long long);
//DTL_MEMBERCOPY(ODBCINT64);

// proxies needed so that char arrays get passed properly to MakeActualCopyOfMember() 

// needed so char literals may be passed to IndexedDBView::find() and friends

template<class DataField> class CharStarProxyNonConst
{
private:
	DataField *ptr;
public:
	CharStarProxyNonConst(DataField *p) : ptr(p) { }

	DataField *ptr_from_proxy() const
	{
		return ptr;
	}
};

template<> class CharStarProxyNonConst<const TCHAR *>
{
private:
	TCHAR **ptr;
public:
	CharStarProxyNonConst(const TCHAR **p) : ptr(const_cast<TCHAR **>(p)) { }

	TCHAR **ptr_from_proxy() const
	{
		return ptr;
	}
};

#if !defined (__BORLANDC__) && !defined (__SUNPRO_CC)
template<> class CharStarProxyNonConst<TCHAR *>
{
private:
	TCHAR **ptr;
public:
	CharStarProxyNonConst(TCHAR **p) : ptr(p) { }

	TCHAR **ptr_from_proxy() const
	{
		return ptr;
	}
};

template<> class CharStarProxyNonConst<TCHAR * const>
{
private:
	TCHAR **ptr;
public:
	CharStarProxyNonConst(TCHAR * const *p) : ptr(const_cast<TCHAR **>(p)) { }

	TCHAR **ptr_from_proxy() const
	{
		return ptr;
	}
};

template<> class CharStarProxyNonConst<const TCHAR * const>
{
private:
	TCHAR **ptr;
public:
	CharStarProxyNonConst(const TCHAR * const *p) : ptr(const_cast<TCHAR **>(p)) { }

	TCHAR **ptr_from_proxy() const
	{
		return ptr;
	}
};
#endif

template<class DataField> class CharStarProxyConst
{
private:
	const DataField & ref;
public:
	CharStarProxyConst(const DataField &r) : ref(r) { }

	const DataField * ptr_from_proxy() const
	{
		return &ref;
	}
};

template<> class CharStarProxyConst<const TCHAR *>
{
private:
	const TCHAR * const ptr;
public:
	CharStarProxyConst(const TCHAR * const p) : ptr(p) { }

	const TCHAR * ptr_from_proxy() const
	{
		return ptr;
	}
};

template<> class CharStarProxyConst<TCHAR *>
{
private:
	const TCHAR * const ptr;
public:
	CharStarProxyConst(const TCHAR * const p) : ptr(p) { }

	const TCHAR * ptr_from_proxy() const
	{
		return ptr;
	}
};

class BoundIOs_base
{
public:

	virtual void* GetWorkingAddr() { return NULL; };

    virtual size_t GetWorkingObjSize () = 0;

	virtual STD_::string GetWorkingObjType () { return ""; };

	virtual ~BoundIOs_base ( ) { }
};

// class which abstracts the binding of columns and parameters
class BoundIO
{
  public:
    // what does the BoundIO structure map data to?
    enum BoundType { UNKNOWN, COLUMN, PARAM };

    // bind as columns or parameters?
    enum ColumnsOrParams { BIND_AS_COL, BIND_AS_PARAM };

	static const size_t MINSTRBUFLEN;

  private:
	void *addr;				// absolute address of data
	ptrdiff_t offset;		// relative offset of this field from base address of its DataObj
							// or ParamObj where applicable

	SDWORD sqlType;
	SDWORD cType;
	SDWORD paramType;
	tstring name;
	int    typeId;
	bool   IsPrimitive;
	SDWORD size;
	SDWORD bufferLength;

	CountedPtr<SDWORD> bytesFetched;

	static const SDWORD lenAtExec; // needed for PutData()

	int VariantRowIdx;		// -1: It isn't a variant row
					// Otherwise: Index into the variant row
	BoundIOs_base *pBoundIOs;	// refers to BoundIOs that this object belongs to
	
	BoundType bindingType;  // column or param???

	int colNo;				// column number
	

	MemPtr strbuf;		   // needed for use with STL strings
						   // points to a buffer allocated
						   // for string or wstring
						   // NULL for all other types

	// workaround for compiler member specialization issue instead of
	// being able to specialize BoundIO::CopyMember(), we cheat
	// we wrote a specialization for variant_row and do nothing otherwise
	// the function below should never get called!
	template <class T> void CopyVariantRowMember(T &key, const dtl_variant_t & df) {
	}

	// form of CopyMember() for variant
	void CopyVariantRowMember(variant_row &vr, const dtl_variant_t &m);

	// only used by GenerateBoundIOs in LocalBCA.h
	BoundIO(const tstring &nm, BoundType bt);

  public:

	BoundIO(); 

//	BoundIO(const tstring &nm, BoundColMode bcMode, BoundType bt, BoundIOs &parentCollection);

	BoundIO(const tstring &nm, BoundType bt, BoundIOs &parentCollection);

	BoundIO(BoundIOs &parentCollection, const tstring &nm, const TypeTranslation &tt, void *field, void *base_addr, size_t base_size);

	BoundIO(const BoundIO &boundIO);

	// only used by GenerateBoundIOs in LocalBCA.h
	// works like copy constructor, but overrides parent collection ptr.
	// with the reference passed in
	BoundIO(const BoundIO &boundIO, BoundIOs &parentCollection);

	// nothrow swap
	void swap(BoundIO &other);

	// exception safe assignment
	BoundIO &operator=(const BoundIO &boundIO);

	// BoundColMode GetBoundColMode() const;

	BoundType GetBindingType() const;

	void SetBindingType(BoundType bt);

	int GetColNo() const;
	
	void SetColNo(int iCol);

	bool IsParam()  const;

	bool IsColumn() const;
	
	bool IsKnown() const;

	// returns whether the column fetched is null
	bool IsNull() const;

	// set the given column to represent a NULL value for writing parameters
	void SetNull();

	// clear the NULL status given above
	// indicator must be SQL_NTS for strings and 0 otherwise
	// except for strings, all SQL_C types have fixed lengths
	void ClearNull();

	// does this BoundIO represent a tstring?
	bool IsString() const; 

	// does this BoundIO represent a variable length string?
	bool IsVariString() const;

	// is this an array_string
	bool IsCString() const;
	
	bool IsPrimitiveColumn() {return IsPrimitive;}

	bool IsJtime() const;

	template<class DataObj> int Compare(DataObj *key1, DataObj *key2) const
	{
		// offset into the objects should be valid
		// as we checked the offset back in BoundIOs::operator==()
		// so no check needs to be done here
					
		// get pointer to the actual member in each DataObj
		// now using our BoundIO offset
		void *pMember1 = (void *) (data_ptr(key1) + offset);
		void *pMember2 = (void *) (data_ptr(key2) + offset);

		// now we must cast to the appropriate type based on the
		// type name of the BoundIO and then perform the comparison
	    return GenericCmp(pMember1, pMember2, typeId);
	}

	template<class DataObj> size_t Hash(DataObj *key1) const
	{
		// offset into the objects should be valid
		// as we checked the offset back in BoundIOs::operator==()
		// so no check needs to be done here
					
		// get pointer to the actual member in each DataObj
		// now using our BoundIO offset	
		void *pMember1 = (void *) (data_ptr(key1) + offset);
	
		// now we must cast to the appropriate type based on the
		// type name of the BoundIO and then perform the hash
	    if (IsNull())
			return 0;  // null fields hash to the identity element
		else
			return GenericHash(pMember1, typeId);
	}

	// overloads needed for variants to correctly handle null fields
	int Compare(variant_row *key1, variant_row *key2) const;

	size_t Hash(variant_row *key1) const;

	// needed for proper STL tstring handling
	// MoveRead() and MoveWrite()
	// the functions that propagate the data back to the STL tstring
	// upon reading or writing respectively from/to the database
	void MoveRead(DBStmt &stmt);
	void MoveWrite(SQLQueryType sqlQryType, DBStmt &stmt);
	void MoveWriteAfterExec(SQLQueryType sqlQryType, DBStmt &stmt);

	// comparison operator for BoundIO objects ...
	// needed by BoundIOs::operator[]()
    friend bool operator==(const BoundIO &bound1, const BoundIO &bound2);

	// get proper address for use by actual BindCol() and BindParam()
	void *GetRawPtr() const;

	// get size of raw buffer
	SDWORD GetRawSize() const;

	// get precision of value
	SDWORD GetPrecision() const;

	// get SQL column size for SQLBindParam purposes of value
	SDWORD GetColumnSize(const DBConnection &conn) const;

	// assuming buffer length and raw size are the same
	SDWORD GetBufferLength() const;

	// get C Type
	SDWORD GetCType() const;

	// get Input/Ouput parameter type
	SDWORD GetParamType() const;

	void SetParamType(SDWORD ptype);

	// get SQL Type
	SDWORD GetSQLType() const;

	// set SQL Type ... use this method to override the default SQL type used
	// vs. your C type
	void SetSQLType(SDWORD newSqlType);

	// set C Type ... use this method to override the default ODBC C type used
	// when binding a field .e.g.the default SQL_C_type for a char data field
	// is SQL_C_CHAR, but you might want to use SQL_C_TINYINT to hold an int in
	// a char field instead
	void SetCType(SDWORD newCType);

	BoundIO & SetColumnSize(size_t sz);

	// get type ID
	int GetTypeID() const;

	// get pointer to bytesFetched
	SDWORD *GetBytesFetchedPtr() const;

	// set pointer to bytesFetched
	void SetBytesFetchedPtr(SDWORD *ptr);

	// get pointer to length at exec ptr. for PutData()
	SDWORD *GetLenAtExecPtr() const;

	// returns name of the bound IO object (column name or parameter #)
	tstring GetName() const;

	BoundIOs *GetBoundIOsPtr() const;

	void SetBoundIOsPtr(BoundIOs *ptr);

	void *GetAddr() const;

	void SetAddr(void *address);

	void SetStrbufAddr(BYTE *address, size_t sz);

	void SetStrbufAddrOnly(BYTE *address);

	void SetBufferLength(SDWORD sz);

	// return relative offset from bound DataObj
	ptrdiff_t GetOffset() const;
	void SetOffset(ptrdiff_t diff);

	void SetAsParam(int numParam);

	// set the field for this BoundIO to NULL if a column and primitive
	void InitNullField();

	virtual ~BoundIO();
   
    // binding operator ... maps the memory structure (object member) to a database column
    // or SQL parameter
    // will use RTTI to determine the information to fill the BoundIO with
    // and use a lookup map to determine the appropriate SQL and C types for the appropriate // run-time type
    // may need a specialization for strings

    // must check bc.GetBoundColMode()  first ... BIND_ADDRESSES -  bind just the
    // names, do not bind addresses
    // BIND_ADDRESSES - bind both names and addresses
	// give binding operator access to the innards of this class

private:

	template<class memberType> void GenericBindPrimitive(memberType &memberToBind)
	{
//	  std::cerr << "Bind " << typeid (memberToBind).name ( ) << std::endl;
		// common tasks for BIND_ADDRESSES and BIND_AS_INDICES

	    // use RTTI to get the type of the object
		const STD_::string RTTI = DTL_TYPEID_NAME (memberToBind);

		ETI_Map &SQL_types_to_C = GetSQL_types_to_C();
		TypeTranslation &tt = SQL_types_to_C[RTTI];
		tt.size = sizeof(memberToBind);  // we need to reset size here to handle char[]

		void*   base_addr;
		size_t  base_size;

		if (pBoundIOs)
		{
		  base_addr = pBoundIOs->GetWorkingAddr();
		  base_size = pBoundIOs->GetWorkingObjSize();
		}
		else
		{
		  base_addr = addr;
		  base_size = tt.size;
		}

		// exception safety handled by InitFromField
		InitFromField(tt, &memberToBind, base_addr, base_size);
	}

	template<typename T> void GenericBindString(T &memberToBind)
	{
//	  std::cerr << "Bind " << typeid (memberToBind).name ( ) << std::endl;
		// common tasks for BIND_ADDRESSES and BIND_AS_INDICES

	    // use RTTI to get the type of the object
		const STD_::string RTTI = DTL_TYPEID_NAME (memberToBind);

		ETI_Map &SQL_types_to_C = GetSQL_types_to_C();
		TypeTranslation tt = SQL_types_to_C[RTTI];
		tt.size = memberToBind.capacity();

		void *base_addr;
		size_t base_size;


		if (pBoundIOs)
		{
			base_addr = pBoundIOs->GetWorkingAddr();
			base_size = pBoundIOs->GetWorkingObjSize();
		}
		else
		{
			base_addr = addr;
			base_size = tt.size;
		}

		// exception safe binding handled by InitFromField
		InitFromField(tt, &memberToBind, base_addr, base_size);
	}

	void GenericBind(STD_::string &memberToBind)
	{
		GenericBindString(memberToBind);
	}

	void GenericBind(blob &memberToBind)
	{
		GenericBindString(memberToBind);
	}

#ifndef DTL_NO_UNICODE
	void GenericBind(STD_::wstring &memberToBind)
	{
		GenericBindString(memberToBind);
	}
#endif

  // Specialize for tcstring since that is all we
  // understand at this point.
 
  template<size_t N>
  void GenericBindArrayString (tcstring<N>& memberToBind)
  {
    const STD_::type_info& RTTI = typeid (fake_tcstring);
		
    ETI_Map& SQL_types_to_C = GetSQL_types_to_C ( );
    TypeTranslation tt = SQL_types_to_C[RTTI.name ( )];
    tt.size = (N + 1) * sizeof(TCHAR);

    void* base_addr;
    size_t base_size;

    if (pBoundIOs) {
      base_addr = pBoundIOs->GetWorkingAddr();
      base_size = pBoundIOs->GetWorkingObjSize();

    } else {
      base_addr = addr;
      base_size = (N + 1) * sizeof(TCHAR);
    }

    // exception safe binding handled by InitFromField
    // InitFromField (tt, &memberToBind, base_addr, base_size);
    InitFromField (tt, &*(memberToBind.begin()), base_addr, base_size);
  }

  template<size_t N>
	  void GenericBind(tcstring<N> &memberToBind)
  { GenericBindArrayString(memberToBind); }


// macro used to generate overloads which call GenericBindPrimitive()
#define DTL_GENERIC_BIND_PRIMITIVE(type) \
  inline void GenericBind(type &memberToBind) \
  { GenericBindPrimitive(memberToBind); }
  
    DTL_GENERIC_BIND_PRIMITIVE(short);
//	DTL_GENERIC_BIND_PRIMITIVE(unsigned short); // not supported as unsigned short is the
												// type used for wchar_t on some compilers
	DTL_GENERIC_BIND_PRIMITIVE(bool);
	DTL_GENERIC_BIND_PRIMITIVE(int);
	DTL_GENERIC_BIND_PRIMITIVE(unsigned int);
	DTL_GENERIC_BIND_PRIMITIVE(long);
	DTL_GENERIC_BIND_PRIMITIVE(unsigned long);
	DTL_GENERIC_BIND_PRIMITIVE(double);
	DTL_GENERIC_BIND_PRIMITIVE(float);

	DTL_GENERIC_BIND_PRIMITIVE(struct tagTIMESTAMP_STRUCT);
	DTL_GENERIC_BIND_PRIMITIVE(jtime_c);

	DTL_GENERIC_BIND_PRIMITIVE(long long);
	DTL_GENERIC_BIND_PRIMITIVE(unsigned long long);
	//DTL_GENERIC_BIND_PRIMITIVE(ODBCINT64);

	void TypeTranslationFieldBind(TypeTranslationField &ttf);

public:

	template<class memberType> BoundIO operator==(memberType &memberToBind) {
		paramType = SQL_PARAM_INPUT_OUTPUT;
		GenericBind(memberToBind);
		return *this;
	}

	template<class memberType> BoundIO operator<<(memberType &memberToBind) {
		paramType = SQL_PARAM_INPUT;
		GenericBind(memberToBind);
		return *this;
	}

	template<class memberType> BoundIO operator>>(memberType &memberToBind) {
		paramType = SQL_PARAM_OUTPUT;
		GenericBind(memberToBind);
		return *this;
	}

        // if you are binding character arrays in Borland you will
        // need to manually set the size by calling SetColumnSize
        // e.g.
        // struct Rowbuf {char strarray[50];}
        // rowbuf["STRING_FIELD"] == rowbuf.strarray;
        // rowbuf.SetColumnSize(sizeof(rowbuf.strarray));
private:
    BoundIO CharBind(tchar_struct memberToBind);
public:
	size_t GetColumnSize() const;

    BoundIO operator==(tchar_struct memberToBind) ;
    BoundIO operator<<(tchar_struct memberToBind) ;
    BoundIO operator>>(tchar_struct memberToBind) ;

	BoundIO operator==(TypeTranslationField ttf);

	BoundIO operator<<(TypeTranslationField ttf);

	BoundIO operator>>(TypeTranslationField ttf);

	// Construct binding definition based on type & address
	void InitFromField(const TypeTranslation &tt, void *field, void *base_addr, size_t base_size);

#if 0
	template<class DataObj> 
		void CopyMember(DataObj &key, const TCHAR df[])
	{
		CopyMember(key, (const TCHAR *const) df);
	}
#endif

	template<class DataObj, class DataField>
			  void CopyMember(DataObj &key, const DataField &df)
	{  

#if 0
		  if (!pBoundIOs)
		  {
			DTL_THROW RootException(_TEXT("BoundIO::CopyMember()"),
				_TEXT("This BoundIO is not bound to its parent!"));
		  }
#endif

		  if (VariantRowIdx>=0)
		  {
			  CopyVariantRowMember(key, dtl_variant_t(df));
			  return;
		  }

		  STD_::string dataObjType = DTL_TYPEID_NAME (key);
		  STD_::string baseType = pBoundIOs->GetWorkingObjType();

		  // check types of DataObj and of BoundIO's container base object
		  if (dataObjType != baseType)
		  {
			  tstring errmsg;
			  errmsg.reserve(512);
			  errmsg += _TEXT("Type mismatch for base object!  ");
			  errmsg += _TEXT("Expected type ");
			  errmsg += tstring_cast((tstring *)NULL, baseType);
			  errmsg += _TEXT("!  Instead got ");
			  errmsg += tstring_cast((tstring *)NULL, dataObjType);
			  errmsg += _TEXT("!");

			  DTL_THROW DBException(tstring(_TEXT("BoundIO::CopyMember()")),
					errmsg, NULL, NULL);
		  }

		  // if type names don't match, we've mismatched fields for sure
		  // throw an exception in this case

		  // other than this check, we must trust the programmer!
		  STD_::string dfTypeNm = DTL_TYPEID_NAME (df);
		
// skip datafield type check in GCC because of different convention
// for typeid().name()'s that we can't handle for constness
#ifndef __GNUC__
		  ETI_Map &SQL_types_to_C = GetSQL_types_to_C();

		  TypeTranslation &tt = SQL_types_to_C[StripConstFromTypename(dfTypeNm)];


		  if (tt.typeId != typeId && (typeId == C_TCSTRING && !is_tcstring(df)))
			  DTL_THROW DBException(_TEXT("BoundIO::CopyMember()"),
				_TEXT("Type mismatch in member!  Type of target value: ") + 
				tstring_cast((tstring *)NULL, dfTypeNm) + 
				_TEXT(" does not match bound column type!"),
				NULL, NULL);


#endif

		  // this cast should be safe
		  // (watch out for offset problems???  We should be OK as we're using the same
		  // data types that were used to compute the offset)
		  DataField *rawAddr = (DataField *) ((BYTE *) data_ptr(&key) + offset); 

		  // makes the actual copy of the member
		  // generic case: *rawAddr = df
		  // TCHAR *: std_tstrcpy(*rawAddr, df)

		  MakeActualCopyOfMember(
			  CharStarProxyNonConst<DataField>(rawAddr).ptr_from_proxy(),
			  CharStarProxyConst<DataField>(df).ptr_from_proxy()
		  );
	}

	// needed so char literals can be coerced to pointers implicitly
	// void CopyMember(DataObj &key, const DataField &df);

	void SetAsVariantRow(int idx);

	bool GetIsVariantRow() const;
	
	int GetVariantRowIdx() const;

	SDWORD GetActualBufferLength() const;

	friend class BoundIOs;

	friend class GenerateBoundIOs;

	// facilities used by LocalBCA
	// operator that generates a BoundIOs structure from two raw BoundIO's
	BoundIOs operator&&(const BoundIO &addMe);

	TypeTranslation GetTypeTranslation() const;
};

tostream &operator<<(tostream &o, const BoundIO &b);

// a mapping of columns and parameters to memory structures
// (usually for a specific view)
class BoundIOs : public STD_::map<tstring, BoundIO, NonCaseSensitiveLt>,
		 public BoundIOs_base
{
  private:
	// BoundIO::BoundColMode mode;
	void *working_addr;			// base address of the DataObj we're bound to
								// needed so we can calculate offsets of the members
								// only needed in the case of indexes
								// but might be able to be used elsewhere

	STD_::string working_type;			// base object's type (DataObj)
	size_t working_size;			// size of the DataObj we're bound to
								// think of it as sizeof(*working_addr)

 public:
	int cColumns;

	MemPtr pBytesFetchedArray;
	MemPtr pRowStatusArray;
	CountedPtr<SQLUINTEGER> pNumRowsFetched;
	MemPtr pDatesFetchedArray;


	// BoundIOs(BoundIO::BoundColMode bcMode = BoundIO::BIND_ADDRESSES);
	BoundIOs();

	// Should rebind bases for proper behavior after call to copy constructor!
	BoundIOs(const BoundIOs &boundIOs);

	// no-throw swap
	void swap(BoundIOs &other);

	BoundIOs &operator=(const BoundIOs &boundIOs);

	// bind base object so we can compute offsets
    // currently only implemented for indices
	template<class DataObj> void BindAsBase(DataObj &rowbuf)
	{
        // attempting to assign working_type first guarantees exception safety
		working_type = DTL_TYPEID_NAME (rowbuf);
		working_addr = &rowbuf;
		working_size = sizeof(rowbuf);
	}

	// fix column ordering ... needed if column names lose their zero-indexing
	void FixColNos();
 
	void BindAsBase(void *addr, size_t size, STD_::string name);

	// returns address of current DataObj / ParamObj we are working with in bca / bpa
	void *GetWorkingAddr();

	// Resets the address of the DataObj we are working with & updates all BoundIOs to
	// this new address
	void SetWorkingAddr(BYTE *addr, TIMESTAMP_STRUCT *pts);
	
	void SetComplexAddr(BYTE *addr, TIMESTAMP_STRUCT *pts);

	bool HasStrings();

	bool HasCStrings();

	// returns size of current DataObj / ParamObj we are working with in bca / bpa
	size_t GetWorkingObjSize();

	// returns stringified type of current DataObj / ParamObj we are working with in bca / bpa
	STD_::string GetWorkingObjType();

	// you must call this function to bind the base address of the DataObj
	// just prior to calling the 
    // return the column with the given name
    BoundIO &operator[](const tstring &colName);

	// return the parameter with the given index
	// (uses similar logic to BoundIO::operator[](tstring)
	BoundIO &operator[](unsigned int paramNum);
	

	// accessors for BoundColMode ... BIND_ADDRESSES or 
	// BIND_ADDRESSES
	// BoundIO::BoundColMode GetBoundColMode();
	
	// return the names of the columns bound to
	STD_::vector<tstring> GetColumnNames() const;

	// return the # of parameters bound
	int NumParams() const;

	// return the # of columns bound
	int NumColumns() const;

	// # of jtime columns
	int NumJtimes();

	// invalidate all existing columns
	void EraseColumns() ;

	// erase a particular column
	void EraseColumn(tstring &col);

	// return true if any of the parameters are output or input_output parameters
	bool HasOutput() const;

	// propagate all bound STL strings to their strbufs
	// We make this non exception-safe, but that should be O.K. since BoundIOs merely
	// hold temporary buffers anyway. Also, making a temporary copy for exception
	// safety here will kill us performance wise.
	void PropagateToSQL(SQLQueryType sqlQryType, DBStmt &stmt);

	// propagation stuff needed after Execute() for PutData()
	void PropagateToSQLAfterExec(SQLQueryType sqlQryType, DBStmt &stmt);

	// find the BoundIO with the param number passed in
	BoundIO &GetBoundIOforColumn(SQLPOINTER paramNum);

		// propagate results back to the bound STL strings ... done on a SELECT
	// We make this non exception-safe, but that should be O.K. since BoundIOs merely
	// hold temporary buffers anyway. Also, making a temporary copy for exception
	// safety here will kill us performance wise.
	void PropagateFromResults(DBStmt &stmt);

	// returns a BoundIOs object which is the set of BoundIO objects that represent
	// the fields that differ from dataObj1 to dataObj2
	template<class DataObj>
		BoundIOs ChangedFields(const DataObj &dataObj1, const DataObj &dataObj2)
	{
	   // accessor ... exception safe
	   BoundIOs changed;
	   
	   for (iterator it = begin(); it != end(); it++)
	   {
		 BoundIO &boundIO = (*it).second;
		 
		 DataObj *dataObjPtr1 = const_cast<DataObj *>(&dataObj1);
		 DataObj *dataObjPtr2 = const_cast<DataObj *>(&dataObj2);

		 if (boundIO.Compare(dataObjPtr1, dataObjPtr2) != 0)
			changed.insert(STD_::pair<const tstring, BoundIO>((*it).first, boundIO));
	   }
	  
	   return changed;
	}


	// set the address & initialize bound variant_row fields
	void BindVariantRow(variant_row &vr);

	void ClearNull();

	// set the field for all primitive columns to NULL
	void InitNullFields();

	// returns true if field name passed in null or doesn't exist
	bool IsNullOrNotExists(const tstring &name);

	// facilities used by LocalBCA
	// operator that generates a BoundIOs structure from an
	// already existing BoundIOs and a raw BoundIO
	BoundIOs operator&&(const BoundIO &addMe);

};


size_t BoundIOs_NumColumns(BoundIOs *pB);

tostream &operator<<(tostream &o, const BoundIOs &bs);

END_DTL_NAMESPACE

#endif




// example illustrating the use of an IndexedDBView for Example objects

#ifndef _INDEXEDVIEWEXAMPLE_H
#define _INDEXEDVIEWEXAMPLE_H

#include "example_core.h"


inline bool AlwaysBadSelValidate(BoundIOs &boundIOs, Example &rowbuf)
{
	return false;
}


typedef DBView<Example, ParamObjExample> ViewType;

// typedef's for non-hashed index specialization
typedef CBFunctor2wRet<const Example *, const Example *, bool> IVCompare;

typedef multiset<Example *, IVCompare> MultisetType; 
typedef DBIndex<ViewType, MultisetType, NO_HASH> IdxType; // default arguments are of proper type here


BEGIN_DTL_NAMESPACE
// "specialized" ContainerFactory() tells IndexedDBView to use a custom
// container for indexing records
// in this case, the container is a multiset which sorts in reverse order
// for the Primary Index and normal for all other indices
template<> class ContainerFactory<IdxType>
{
public:
	MultisetType operator()(IdxType *pDBIndex, NO_HASH h) 
	{
		 // for STRING_VALUE's, compare true if first exampleStr > second exampleStr
	
		 if (pDBIndex->GetName() == "PrimaryIndex")
			return MultisetType(cb_ptr_fun_w_ret(reverse_compare_strings));
		 else // for all other indices use generic comparison
			return MultisetType(cb_ptr_fun_w_ret(*pDBIndex, &IdxType::lt));
	}
};
END_DTL_NAMESPACE

#ifdef __SGI_STL_PORT
// typedefs for hashed index specialization
typedef CBFunctor2wRet<const Example *, const Example *, bool> IVEqual;

typedef CBFunctor1wRet<const Example *, size_t> IVHash;

typedef hash_multiset<Example *, IVHash, IVEqual> HashMultisetType; 
typedef DBIndex<ViewType, HashMultisetType, HASH> HashIdxType;

// same as VerySimpleReadFromIndexedView(), but using hashed index containers instead
void VerySimpleReadFromHashedIndexedView();

// same example, but one that should call our specialized version
// of hashed ContainerFactory, and uses postfix clause
void SimpleReadFromHashedIndexedView();

BEGIN_DTL_NAMESPACE
// "specialized" ContainerFactory() tells IndexedDBView to use a custom
// container for indexing records
// in this case, the container is a hash_multiset which uses an alternative hash
// function for the Primary Index and normal for all other indices
template<> class ContainerFactory<HashIdxType>
{
public:
	HashMultisetType operator()(HashIdxType *pDBIndex, HASH h) 
	{
		 // for STRING_VALUE's, hash on exampleStr using alternative hash function
	
		 if (pDBIndex->GetName() == "PrimaryIndex")
			return HashMultisetType(MEDIUM_HASH_TABLE,
									cb_ptr_fun_w_ret(my_hash_strings),
									cb_ptr_fun_w_ret(*pDBIndex, &HashIdxType::eq));
		 else // for all other indices use generic hash and comparison
			return HashMultisetType(MEDIUM_HASH_TABLE,
									cb_ptr_fun_w_ret(*pDBIndex, &HashIdxType::hash),
									cb_ptr_fun_w_ret(*pDBIndex, &HashIdxType::eq));
	}
};
END_DTL_NAMESPACE

#endif

// Example of using an IndexedDBView to read, insert and update records in a container / database
void IndexedViewExample();

// Example of reading from a IndexedDBView which uses all default comparisons
void SimpleReadFromIndexedView();

// Even simpler example ... query with no postfix clause ...
void VerySimpleReadFromIndexedView();

// same example yet again, but used the custom associative container
// that is a sorted vector that emulates a multiset
void SimpleReadFromCustomIndexedView();

// Example of using an IndexDBView to read, insert and update records in a container / database
void CustomIndexedViewExample();

#endif

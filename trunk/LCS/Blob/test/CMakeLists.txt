
foreach (LCS_Blob_test
    tBlobString
    tBlobStream
    tBlobArray
    tBlobSTL
    tBlobCasa
    tBlobField
    tBlobAipsIO
    tKeyValueMap
    )
  ## compiler instructions
  add_executable (${LCS_Blob_test} ${LCS_Blob_test}.cc)
  ## linker instructions
  if (HAVE_CASACORE)
  target_link_libraries (${LCS_Blob_test} blob common ${CASA_LIBRARIES})
  else (HAVE_CASACORE)
  target_link_libraries (${LCS_Blob_test} blob common)
  endif (HAVE_CASACORE)
  ## testing instructions
  add_test (${LCS_Blob_test} ${LCS_Blob_test})
endforeach (LCS_Blob_test)

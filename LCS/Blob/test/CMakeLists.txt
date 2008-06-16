
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
  target_link_libraries (${LCS_Blob_test} blob common)
  ## testing instructions
  add_test (${LCS_Blob_test} ${LCS_Blob_test})
endforeach (LCS_Blob_test)
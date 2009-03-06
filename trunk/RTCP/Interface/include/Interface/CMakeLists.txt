## ------------------------------------------------------------------------------
## Include files

file (GLOB libinterface_HEADERS Align.h
				AlignedStdAllocator.h
				Allocator.h
				CN_Command.h
				CN_Configuration.h
				CN_Mapping.h
				CN_Mode.h
				CorrelatedData.h
				Config.h
				FilteredData.h
				MultiDimArray.h
				Parset.h
				SubbandMetaData.h
				PrintVector.h
				Queue.h
				RSPTimeStamp.h
				SparseSet.h
				Exceptions.h
				StreamableData.h
				PencilBeamData.h
                		StokesData.h
                		PipelineOutput.h)
   
## ------------------------------------------------------------------------------
## Installation

install (FILES ${libinterface_HEADERS}
  DESTINATION include/Interface
  )

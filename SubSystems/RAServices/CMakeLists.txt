# $Id: CMakeLists.txt 20934 2012-05-15 09:26:48Z schoenmakers $

lofar_package(RAServices
                DEPENDS MoMQueryService
                        RATaskSpecifiedService
                        RAtoOTDBTaskSpecificationPropagator
                        ResourceAssigner
                        ResourceAssignmentDatabase
                        ResourceAssignmentEditor
                        ResourceAssignmentEstimator
                        ResourceAssignmentService
                        SystemStatusDatabase
                        SystemStatusService)

# supervisord config files
install(FILES
  RAServices.ini
  DESTINATION etc/supervisord.d)

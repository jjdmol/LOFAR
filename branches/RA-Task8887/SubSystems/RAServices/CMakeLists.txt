# $Id: CMakeLists.txt 20934 2012-05-15 09:26:48Z schoenmakers $

lofar_package(RAServices
                DEPENDS MoMQueryService
                        ResourceAssignmentDatabase
                        ResourceAssignmentServices
                        ResourceAssignmentEditor
                        ResourceAssignmentEstimator
                        ResourceAssignmentService
                        ResourceAssigner
                        RAtoOTDBTaskSpecificationPropagator)

# supervisord config files
install(FILES
  RAServices.ini
  DESTINATION etc/supervisord.d)

# $Id: CMakeLists.txt 20934 2012-05-15 09:26:48Z schoenmakers $

lofar_package(RAServices
                DEPENDS OTDB_Services
                        MoMQueryService
                        ResourceAssignmentDatabase
                        RATaskSpecifiedService
                        ResourceAssignmentEditor
                        ResourceAssignmentService)

# supervisord config files
install(FILES
  RAServices.ini
  DESTINATION etc/supervisord.d)

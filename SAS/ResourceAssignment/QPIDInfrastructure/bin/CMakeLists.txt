# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

lofar_add_bin_scripts(addtoQPIDDB.py
                      cep4_config.sh
                      compareQPIDwithDB.py
                      configQPIDfromDB.py
                      gatherfrombrokers.sh
                      route_to_struct.py)

install_files(/share/qpidinfrastructure/sql qpidinfra_dump.sql)

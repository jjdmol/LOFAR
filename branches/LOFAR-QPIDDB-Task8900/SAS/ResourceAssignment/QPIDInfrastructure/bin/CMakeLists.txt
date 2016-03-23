# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

lofar_add_bin_scripts(addtoQPIDDB.py
                      cep4_config.sh
                      CMakeLists.txt
                      compareQPIDwithDB.py
                      configQPIDfromDB.py
                      gatherfrombrokers.sh
                      # qpidinfra_dump.sql
                      route_to_struct.py)


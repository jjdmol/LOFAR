# HBA frontend en RCU test met led 
# M.J. Norden, 27-09-2010
python verify.py --brd rsp0 --fp blp0 --te tc/hba_client.py --client_acces w --client_reg led --data 01
echo "de led op de RCU 1 brand 2 seconden"
sleep 2
python verify.py --brd rsp0 --fp blp0 --te tc/hba_client.py --client_acces w --client_reg led --data 00
echo "de led op de RCU 1 is nu uit"
python verify.py --brd rsp0 --fp blp0 --te tc/hba_server.py --server 1 --server_acces uc --server_function sw --data 01,01
echo "de led op de HBA-FE brand 2 seconden"
sleep 2
python verify.py --brd rsp0 --fp blp0 --te tc/hba_server.py --server 1 --server_acces uc --server_function sw --data 00,00
echo "de led op de HBA-FE is nu uit"





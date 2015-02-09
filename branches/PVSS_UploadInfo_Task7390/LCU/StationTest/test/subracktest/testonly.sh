# !/bin/bash
# version 2.0, date 18-08-2009,  M.J.Norden


#export PYTHONPATH=/opt/stationtest/modules

######## vragen subrack, batchnr en serienr ####################

vraag1="welk subrack ga je testen [0, 1, 2, 3, 4, of 5] "
echo -n "$vraag1"
read subracknr

vraag2="welk batchnummer staat er op het subrack "
echo -n "$vraag2"
read batchnr

vraag3="welk serienummer staat er op het subrack "
echo -n "$vraag3"
read serienr

####### het starten van de subrack test ####################
cd /opt/stationtest

rm -f *.log
rm -f *.diff

rspctl --rcuprsg=1

python subrack_production.py -b $batchnr -s $serienr





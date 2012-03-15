#! /bin/bash
source /software/users/lofareor/eor-init.sh
outdir="/data1/users/lofareor/${USER}/stats/out/`hostname`"
strategy="/home/users/offringa/distributed/statistics.rfis"
declare -i index=0
rm -fr ${outdir}
mkdir -p ${outdir}
echo single.sh $* > ${outdir}/command.txt
for f in $* ; do
    echo ${index} > ${outdir}/progress.txt
    cdir=${outdir}/${index}
    rm -fr "${cdir}"
    mkdir -p ${cdir}
    cd ${cdir}
    echo \# "$f" > ${cdir}/command.txt 2>> ${cdir}/stderr.txt
    echo makeFLAGwritable "$f" >> ${cdir}/command.txt
    makeFLAGwritable "$f" > ${cdir}/stdout.txt 2>> ${cdir}/stderr.txt
    echo rficonsole -indirect-read -v -nolog -strategy ${strategy} -j 9 "$f" >> ${cdir}/command.txt
    rficonsole -indirect-read -v -nolog -strategy ${strategy} -j 9 "$f" >> ${cdir}/stdout.txt 2>> ${cdir}/stderr.txt
    index=${index}+1
    rm -fr "${localms}" 2>> ${cdir}/stderr.txt
done

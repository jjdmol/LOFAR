#! /bin/bash
source /opt/login/loadpackage.bash LofIm Mon
#source /opt/login/loadpackage.bash LofIm
outdir="/data/scratch/${USER}/stats/out/`hostname`"
strategy="/home/offringa/stats/resolution-test.rfis"
#strategy="~/stats/countflag.rfis"
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
    echo ~/LOFAR/build/gnu_opt/CEP/DP3/AOFlagger/src/rficonsole -strategy ${strategy} -j 7 "$f" >> ${cdir}/command.txt
    localms="/data/scratch/${USER}/SCRIPTED.MS"
    rm -fr "${localms}" 2>> ${cdir}/stderr.txt
    scp -rq "$f" "${localms}" 2>> ${cdir}/stderr.txt
    ~/LOFAR/build/gnu_opt/CEP/DP3/AOFlagger/src/rficonsole -strategy ${strategy} -j 7 "${localms}" > ${cdir}/stdout.txt 2>> ${cdir}/stderr.txt

    index=${index}+1
    rm -fr "${localms}" 2>> ${cdir}/stderr.txt
done

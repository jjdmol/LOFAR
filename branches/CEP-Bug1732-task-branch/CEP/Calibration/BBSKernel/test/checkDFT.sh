# This script checks if the output of the

glish -l tpredict.g >& pred.log
sed -e 's/[][]//g' pred.log > aa
run2.3C343 >& run2.log
sed -e 's/[][,()]/ /g' run2.log >& bb
echo '-- checking pred --'
grep 'pred:' aa | sed -e 's/.*pred://' > aa1
grep 'corr=0' bb | sed -e 's/corr=0//' > bb1
~/sim/LOFAR/autoconf_share/checkfloat aa1 bb1

echo '-- checking data --'
grep 'data:' aa | sed -e 's/.*data://' > aa2
grep 'cor=0' bb | sed -e 's/cor=0//' > bb2
~/sim/LOFAR/autoconf_share/checkfloat aa2 bb2


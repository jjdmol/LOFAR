glish -l ../../../test/tMeqPolc.g | sed -e 's/[][]//g' > aa
tMeqPolc | fgrep 'result' | sed -e 's/[][,]//g' -e 's/result: //' > bb
diff -b aa bb

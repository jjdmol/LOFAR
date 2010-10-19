#
# DROPS the current database and make a whole new test database
#
dropdb -h dop50 -U postgres otdbtest
createdb -h dop50 -U postgres otdbtest

echo "creating new tables"
psql -f create_OTDB.sql -h dop50 -U postgres otdbtest 2>&1 | grep ERROR | grep -v "does not exist"

#echo "adding testset"
#psql -f fill_VIC_components.sql -h dop50 -U postgres otdbtest 2>&1 | grep ERROR
#psql -f fill_VIC_template.sql -h dop50 -U postgres otdbtest 2>&1 | grep ERROR

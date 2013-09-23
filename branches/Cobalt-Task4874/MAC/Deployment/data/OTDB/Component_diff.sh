#!/bin/bash
#
# Script to show the diffs in the component files between two versions of SVN
#
# Usage:
#
# > Component_diff.sh -o <old_version> -p <old_branch> -n <new_version> 
#                      -m <new branch>
#
# If new version is omitted, we assume the latest (HEAD)
# If no branch names are given, we assume trunk
#


# SyntaxError msg
#
SyntaxError()
{
        Msg=$1

        [ -z "${Msg}" ] || echo "ERROR: ${Msg}"
        echo ""
        echo "Syntax: $(basename $0) -o <old_revision> -n <new_revision>"
        echo ""
	echo "  - If new version is omitted, we assume the latest (HEAD)"
        echo "  - Script will find out which branch revision number belongs to"
        echo ""
}

function find_branch_name {
  line=$1
  branch=`echo $line | awk -F/ '{print $2}'`
  if [ "$branch" != "trunk" ]; then
    branch=`echo $line | awk -F/ '{print $3}' | awk '{print $1}'`
    
  fi
#  echo $branch
}

function find_branch {
  rev=$1
  svn log -v --incremental -c ${rev} https://svn.astron.nl/LOFAR > /tmp/svn_log_$$

  branch=""
  nextline=0
  while read line 
  do
    if [ $nextline -eq 1 ]; then
      find_branch_name "$line"
      nextline=2
      break
    fi

    if [ "$line" == "Changed paths:" ]; then
      nextline=1
    fi
  done < /tmp/svn_log_$$
  rm -f /tmp/svn_log_$$
  
}


#
# MAIN
#

if [ -z $1 ]; then
    SyntaxError
    exit 1
fi

curdir=`pwd -P`
old_version=0
new_version=HEAD
old_branch=trunk
new_branch=trunk

while getopts "ho:n:" OPTION
do
     case $OPTION in

         h)
             SyntaxError
             exit 1
             ;;
 	 o)
	     old_version=$OPTARG
             ;;
         n)  
	     new_version=$OPTARG
             ;;
         ?)
             SyntaxError
             exit 1
             ;;
       esac
done

find_branch $old_version
old_branch=${branch}
find_branch $new_version
new_branch=${branch}

echo "Comparing rev. $new_version of branch $new_branch with rev. $old_version of branch $old_branch"
            
mkdir -p /tmp/comp_$new_version
cd /tmp/comp_$new_version
if [ "$new_branch" == "trunk" ]; then 
  co_newdir="trunk/MAC/Deployment/data"
else
  co_newdir="branches/${new_branch}/MAC/Deployment/data"
fi
svn co -r ${new_version} https://svn.astron.nl/LOFAR/${co_newdir} data >& /dev/null
cd data/OTDB 
./create_OTDB_comps -l 
ls -1 *comp > all_new_comps.lst
rm -f all_new_comps.txt
#cat all_new_comps.lst | awk '{print "grep -v ^# "$1" >> all_new_comps.txt"}' | sh


mkdir -p /tmp/comp_$old_version
cd /tmp/comp_$old_version
if [ "$old_branch" == "trunk" ]; then 
   co_olddir="trunk/MAC/Deployment/data"
else
  co_olddir="branches/${old_branch}/MAC/Deployment/data"
fi
svn co -r ${old_version} https://svn.astron.nl/LOFAR/${co_olddir} data >& /dev/null
cd data/OTDB
./create_OTDB_comps -l
ls -1 *comp > all_old_comps.lst
rm -f all_old_comps.txt
#cat all_old_comps.lst | awk '{print "grep -v ^# "$1" >> all_old_comps.txt"}' | sh

cd $curdir
diff -bB -u /tmp/comp_$old_version/data/OTDB/all_old_comps.lst /tmp/comp_$new_version/data/OTDB/all_new_comps.lst > diff_compfiles_$old_version_$new_version
echo ""
echo "Differences in component files:"
echo "=============="
echo "New files in rev. $new_version ($new_branch) compared to rev. $old_version ($old_branch):"
grep ^+[A-Z] diff_compfiles_$old_version_$new_version
echo "=============="
echo "Removed files in rev. $new_version ($new_branch) since rev. $old_version ($old_branch):"
grep ^-[A-Z] diff_compfiles_$old_version_$new_version
echo "=============="
echo ""
if [ -s diff_compfiles_$old_version_$new_version ]; then 
  cat diff_compfiles_$old_version_$new_version | grep ^\ [A-Z] | awk '{print $1}' > all_present.lst
else
  cat /tmp/comp_$new_version/data/OTDB/all_new_comps.lst > all_present.lst
fi
#cd /tmp/comp_$old_version/data/OTDB
#cat ${curdir}/all_present.lst | awk '{print "grep -v ^# "$1" >> all_old_comps.txt"}' | sh
#cd /tmp/comp_$new_version/data/OTDB
#cat ${curdir}/all_present.lst | awk '{print "grep -v ^# "$1" >> all_new_comps.txt"}' | sh

cd $curdir
#diff -bB -y --suppress-common-lines /tmp/comp_$old_version/data/OTDB/all_old_comps.txt /tmp/comp_$new_version/data/OTDB/all_new_comps.txt > diff_${old_version}_${new_version}

while read line
do 
  grep -v -e 'Id:\|create_OTDB_comps' /tmp/comp_$old_version/data/OTDB/${line} > /tmp/comp_$old_version/data/OTDB/tmp
  grep -v -e 'Id:\|create_OTDB_comps' /tmp/comp_$new_version/data/OTDB/${line} > /tmp/comp_$new_version/data/OTDB/tmp
  they_diff=`diff -b -B -q /tmp/comp_$old_version/data/OTDB/tmp /tmp/comp_$new_version/data/OTDB/tmp 2>&1 1>/dev/null ; echo $?`
  if [ $they_diff -eq 1 ]; then 
    echo "Differences in file ${line} between rev. $new_version ($new_branch) and rev. $old_version ($old_branch):" > tmp
    echo "" >> tmp
    diff -bB -u -U 100 /tmp/comp_$old_version/data/OTDB/tmp /tmp/comp_$new_version/data/OTDB/tmp >> tmp
    echo "====================" >> tmp
    cat tmp | grep -v "\-\-\-" | grep -v "+++" | grep -v "\@\@"
    rm -f /tmp/comp_$old_version/data/OTDB/tmp
    rm -f /tmp/comp_$new_version/data/OTDB/tmp
    rm -f tmp

  fi
done < ${curdir}/all_present.lst

rm -fr /tmp/comp_$old_version
rm -fr /tmp/comp_$new_version


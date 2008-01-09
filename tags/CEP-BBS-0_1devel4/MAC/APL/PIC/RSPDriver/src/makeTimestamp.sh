ProgName=`pwd | rev | cut -d '/' -f3 | rev`
Date=`date +"%a %Y/%m/%d %T %Z"`
User=`whoami`
Host=`hostname -s`

echo static char* globalGeneratedTimestamp=\"VERSION=$ProgName\;$Date\;$User@$Host\"\; >makeTimestamp.c

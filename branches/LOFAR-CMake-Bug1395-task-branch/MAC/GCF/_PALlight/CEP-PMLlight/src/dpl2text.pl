#!/usr/bin/perl
#
# Program to open the dpl file, read it in,
# convert type numbers to their textual representation,
# remove #<struct.seq.num>
# write the lines to the outputfile
# close the files
use strict;

my @pvssValueTypes = 
(
  #
  # MAC value types      PVSS type name               PVSS type id
  #
  "notype",           #// <not specified by pvss>      0
  "structure",        #// structure                    1
  "notype",           #// <not specified by pvss>      0
  "char_array",       #// dyn. character array         3
  "unsigned_array",   #// dyn. unsigned array          4
  "int_array",        #// dyn. integer array           5
  "double_array",     #// dyn. float array             6
  "bool_array",       #// dyn. bit array               7
  "notype",           #// dyn. bit pattern-array       8
  "string_array",     #// dyn. text array              9
  "notype",           #// dyn. time array              10
  "notype",           #// character structure          11
  "notype",           #// integer structure            12
  "notype",           #// unsigned structure           13
  "notype",           #// float structure              14
  "notype",           #// bit32                        15
  "notype",           #// bit32 structure              16
  "notype",           #// text structure               17
  "notype",           #// time structure               18
  "char",             #// character                    19
  "unsigned",         #// unsigned                     20
  "int",              #// integer                      21
  "double",           #// float                        22
  "bool",             #// bit                          23
  "notype",           #// bit pattern                  24
  "string",           #// text                         25
  "notype",           #// time                         26
  "notype",           #// identifier                   27
  "notype",           #// dyn. identifier              29
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// <not specified by pvss>      0
  "notype",           #// identifier array             39
  "notype",           #// <not specified by pvss>      0
  "notype",           #// type reference               41
  "notype",           #// multilingual text            42
  "notype",           #// multilingual text structure  43
  "notype",           #// dyn. description array       44
  "notype",           #// <not specified by pvss>      0
  "blob",             #// blob                         46
  "notype",           #// blob structure               47
  "blob_array"        #// dyn. blob array              48
);


my $numArgs = $#ARGV + 1;
if($numArgs != 2)
{
    print "Usage: dpl2text.pl <infile> <outfile>\n\n";
    exit;
}

my $infile = $ARGV[0];
my $outfile = $ARGV[1];
if(!open(INFILE, $infile))
{
    print "Unable to open file $infile for reading: $!\n\n";
    exit;
}
if(!open(OUTFILE, ">$outfile"))
{
    print "Unable to open file $outfile for writing: $!\n\n";
    exit;
}

my @lines = <INFILE>;

print OUTFILE "##\n";
print OUTFILE "## WARNING: DO NOT MODIFY!\n";
print OUTFILE "## This file was automatically generated from $infile by dpl2text.pl\n";
print OUTFILE "##\n";

foreach my $line (@lines)
{
    if($line =~ /(.*)(\s)(\d+)(\#\d+)/ )
    {
	print OUTFILE "$1$2@pvssValueTypes[$3]\n";
    }
    else
    {
	print OUTFILE $line;
    }
}
close(INFILE);
close(OUTFILE);

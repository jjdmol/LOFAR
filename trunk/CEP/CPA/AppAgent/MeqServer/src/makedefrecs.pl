#!/usr/bin/perl

@ARGV>1 or die "Usage: $0 <defrec file> <list of headers>";

my ($outfile,@headers) = @ARGV;

# loop over all headers
for( @headers )
{
  open INFILE,"<$_" or die "Can't open input file $infile: $!";
  # nodeclass becomes non-empty inside a defrec declaration
  my $nodeclass = "";  
  my $comments = "";
  while( <INFILE> )
  {
    my $line = $_;
    next unless $line =~ s/^\/\/ ?//; # strip leading //, skip if none
    if( $line =~ /^\s*defrec\s+begin\s+(\w+)\b/ )
    {
      not $nodeclass or die "'defrec begin' inside of a defrec block";
      $nodeclass = $1;
    }
    elsif( $line =~ /^\s*defrec\s+end/ )
    {
      $nodeclass or die "'defrec end' outside of a defrec block";
    }
    elsif( $line =~ /^\s*field:\s+(\w+)\s+(\S+)\b/ )
    {
      next unless $nodeclass; # ignore outside of begin/end block
    }
    else  # else free-form comment to be added to next field
    {
      next unless $nodeclass; # ignore outside of begin/end block
      $comments .= $line;
    }
  }
}

open OUTFILE,">$outfile" or die "Can't open output file $outfile: $!";

close OUTFILE;

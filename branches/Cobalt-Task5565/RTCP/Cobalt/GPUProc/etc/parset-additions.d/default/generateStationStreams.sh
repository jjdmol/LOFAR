#!/bin/bash
#
# Usage: ./generateStationStreams.sh > StationStreams.parset
#
# Requires:
#   RSPConnections_Cobalt.dat
#   MAC+IP.dat
#   RSP+IP.dat
#
# These files can be found in
#   MAC/Deployment/data/StaticMetaData
#

cat RSPConnections_Cobalt.dat | perl -ne '

/^(\w+) RSP_([01]) ([^ \t\n]+)/ || next;

$station = $1;
$board = $2;
$host = $3;

$station =~ /^[A-Z][A-Z]([0-9]+)/;
$nr = $1;

# only parse cobalt nodes
$host =~ /^cbt/ || next;

if (not $cached) {
  %lookup = {};
  %rlookup = {};
  %ilookup = {};

  # MAC+IP.dat resolves hostnames to IPs and MACs
  open $fh, "MAC+IP.dat"
    or die "Cannot open MAC+IP.dat";

  while($line = <$fh>) {
    next if $line =~ /^#/;
    ($name, $ip, $mac) = split(/\s+/, $line);

    $lookup{$name} = $ip;
    $rlookup{$ip}  = $rlookup{$ip} || $name;
  }

  close $fh;

  # RSP+IP.dat lists the international station IP addresses
  open $fh, "RSP+IP.dat"
    or die "Cannot open RSP+IP.dat";

  while($line = <$fh>) {
    next if $line =~ /^#/;
    ($name, $ip) = split(/\s+/, $line);

    $ilookup{$name} = $ip;
  }

  close $fh;

  $cached = 1;
}

$dest = $lookup{$host};
$iface = $rlookup{$dest};
$baseport = 10000 + $nr * 10;

$ips = {};

foreach $rspNr (0 .. 3) {
  $brdname = sprintf "%s_%02d", $station, $rspNr;

  if (exists $ilookup{$brdname}) {
    # international station: IP = src IP, ending in .50
    $ips{$rspNr} = $ilookup{$brdname} =~ s/\.[0-9]+$/.50/r;
  } else {
    $ips{$rspNr} = $iface;
  }
}

$portstr = sprintf "[udp:%s:%d, udp:%s:%d, udp:%s:%d, udp:%s:%d]",
  $ips{0}, $baseport + ($board * 6) + 0,
  $ips{1}, $baseport + ($board * 6) + 1,
  $ips{2}, $baseport + ($board * 6) + 2,
  $ips{3}, $baseport + ($board * 6) + 3;

$iface =~ /(cbt[0-9]+)-10GB0([1234])/;
$host = $1;
$ifnr = $2;
$receiver = sprintf "%s_%u", $host, ($ifnr - 1)/2;

if ($board == 0) {
  printf "PIC.Core.%sLBA.RSP.receiver  = %s\n",$station,$receiver;
  printf "PIC.Core.%sLBA.RSP.ports     = %s\n",$station,$portstr;

  printf "PIC.Core.%sHBA.RSP.receiver  = %s\n",$station,$receiver;
  printf "PIC.Core.%sHBA.RSP.ports     = %s\n",$station,$portstr;

  if ($station =~ /^CS/) {
    printf "PIC.Core.%sHBA0.RSP.receiver = %s\n",$station,$receiver;
    printf "PIC.Core.%sHBA0.RSP.ports    = %s\n",$station,$portstr;
  } else {
    print "\n";
  }
}

if ($board == 1) {
  printf "PIC.Core.%sHBA1.RSP.receiver = %s\n",$station,$receiver;
  printf "PIC.Core.%sHBA1.RSP.ports    = %s\n",$station,$portstr;
  print "\n";
}

'



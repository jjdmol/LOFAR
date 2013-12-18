#!/usr/bin/perl
$VERSION = q$Revision: 3.00$;

#
# rsuctl # Remote System Update - program firmware in FPGA flash
#
# Usage: rsuctl [options] command
#
#   options
#     [-i ifname]    # Ethernet interface name on which to communicate (default is eth1)
#     -m mac-address # MAC-address of the target board (Mandatory)
#
#   command (specify at least one of the following)
#     [-l] # List the images currently stored in all pages of the flash
#
#     [-e -p page [-F]]                                 # Erase flash page,  0 <= page < 16 (-F forces erase of page 0)
#     [-x -p page                                       # Start (load & reset) new firmware from the specified page
#     [-w -p page -b bp_img[.hex] -a ap_img[.hex] [-F]] # Write bp and ap image into specified page (-F forces write to page 0)
#     [-v -p page -b bp_img[.hex] -a ap_img[.hex]]      # Compare flash page with bp and ap image
#     [-d -p page -f dumpfile.bin]                      # Dump flash page to dmp.bin
#
#     [-r] # Reset all FPGA's (load factory image)
#     [-c] # Clear all FPGA's (restart current image)
#     [-s] # Send SYNC pulse to all FPGA's
#
#     [-q] # Prevent checking for running RSPDriver (prevents pause at start)
#     [-P] # Ping the board (find board when using broadcast address)
#     [-V] # Read RSU status and RSR versions
#     [-h] # This help text
#
#

#use Socket;
use Net::RawIP;
use Getopt::Std;
use File::stat;
use Switch;
use POSIX; # for geteuid()

$g_debug  = 0;  # set to 1 to enable debug output
$g_packet = ''; # global packet string
$g_error  = 0;  # global error status

#
# Definitions
#
$PID_RSR      = 0x01;
$PID_RSU      = 0x02;

$REG_STATUS       = 0x00;
$REG_STATUS_SIZE  = 200;
$REG_VERSION      = 0x01;
$REG_VERSION_SIZE = 2;

$REG_RW       = 0x01;
$REG_ERASE    = 0x02;
$REG_RECONFIG = 0x03;
$REG_CLEAR    = 0x04;

$CMD_ERASE = 0x01;

$CMD_SYNC  = 0x01;
$CMD_CLEAR = 0x02;
$CMD_RESET = 0x04;

$MEPHDRSIZE = 16;

$TYPE_READ     = 0x01;
$TYPE_WRITE    = 0x02;
$TYPE_READACK  = 0x03;
$TYPE_WRITEACK = 0x04;

$ADDR_BLP_NONE = 0x00;
$ADDR_RSP      = 0x01;

$FLASHSIZE         = 64 * 1024 * 1024; # 64MB
$BLOCKSIZE         = 1024; # block  = 1024 bytes
$BLOCKS_PER_SECTOR = 128;  # sector = 128 blocks
$SECTORS_PER_PAGE  = 32;   # page   = 32 sectors
$BLOCKS_PER_PAGE   = $BLOCKS_PER_SECTOR * $SECTORS_PER_PAGE; # 4096 blocks
$PAGESIZE          = $SECTORS_PER_PAGE * $BLOCKS_PER_SECTOR * $BLOCKSIZE; # 4MB  
$PAGES             = 16;

$PROGRESS_SIZE     = 32;

$RESPONSETIMEOUT = 2;
$TO = 5;

#
# pack format for the MEP message
#
# C 0x10 Ethernet Type byte 0
# C 0xFA Ethernet Type byte 1
# C TYPE
# C STATUS
# S FRAMELENGTH
# C BLPID
# C RSP
# C PID
# C REGID
# S OFFSET
# S PAYLOAD_LENGTH
# S SEQ_NR
# S RESERVED
#
$MEPSENDFORMAT="CCCCSCCCCSSSS";
$MEPRECVFORMAT="H28CCSCCCCSSSS";

# ETHLEN = max length of Ethernet payload
$ETHLEN = 1500;

#
# Error code
#
%STATUS = ( 0 => "=== OK",
	1 => "=== ERR unknown MEP message type",
	2 => "=== ERR illegal BLP address",
	3 => "=== ERR invalid PID",
	4 => "=== ERR register does not exist",
	5 => "=== ERR offset too large",
	6 => "=== ERR message is too large",
	7 => "=== ERR message corruption during RSP processing",
	8 => "=== ERR message lost during RSP processing"
);

#
# usage
#
sub usage
{
	print STDERR <<END;

	# Remote System Update - program firmware in FPGA flash
		
	Usage: rsuctl [options] command

	options
	[-i ifname]    # Ethernet interface name on which to communicate (default is eth1)
	-m mac-address # MAC-address of the target board (Mandatory)

	command (specify at least one of the following, need -m option at least)
	[-l] # List the images currently stored in all pages of the flash

	[-e -p page [-F]]                                 # Erase flash page, 0 <= page < 16 (-F forces erase of page 0)
	[-x -p page                                       # Start (load & reset) new firmware from the specified page
	[-w -p page -b bp_image.hex -a ap_image.hex [-F]] # Write bp and ap image into specified page (-F forces write to page 0)
	[-v -p page -b bp_image.hex -a ap_image.hex]      # Compare flash page with bp and ap image
	[-d -p page -f dumpfile.bin]                      # Dump flash page to dmp.bin

	[-r] # Reset all FPGA's (load factory image)
	[-c] # Clear all FPGA's (restart current image)
	[-s] # Send SYNC pulse to all FPGA's

	[-q] # Prevent checking for running RSPDriver (prevents pause at start)
	[-P] # Ping the board (find board when using broadcast address)
	[-V] # Read RSU status and RSR versions
	[-h] # This help text

END

	exit;
}

#
# pack_rsu($type, $regid, $size, $offset)
#
# Pack an RSU message
#
sub pack_rsu
{
	my ($type, $regid, $size, $offset) = @_;
	
	return pack($MEPSENDFORMAT,
		0x10, 0xFA, $type, 0x00, $MEPHDRSIZE + $size,
		$ADDR_BLP_NONE, $ADDR_RSP, $PID_RSU, $regid, $offset, $size, 0, 0);;
}

#
# pcapcallback, save packet in $g_packet
#
# savepacket($handle, $hdr, $packet)
#
sub savepacket
{
	my ($handle, $hdr, $packet) = @_;
	$g_packet = $packet;
}

#
# ($payload,$error) = readresponse($pcap)
#
sub readresponse
{
	my ($pcap) = @_;
	my ($retval) = 0;

	do { # keep going when we receive invalid messages
		$retval = 0; # assume success

		# wait for message to arrive
		eval {
			use POSIX qw(SIGALRM);
			POSIX::sigaction(SIGALRM,
			POSIX::SigAction->new(sub { die }))
			|| die "=== ERR error setting SIGALRM handler: $!\n";

			alarm $RESPONSETIMEOUT;
			(0 == loop ($pcap, 1, \&savepacket, ''))
			|| die "=== ERR Failed to receive response";
			alarm 0;
		};
		if ($@) {
			$retval = 1; # fatal (timeout)
			return ('', $retval, '');
		}

		($ethdr, $type, $status, $framesize, $blp, $rsp, $pid, $regid, $offset, $size, $seqnr, $reserved) =
		unpack($MEPRECVFORMAT, $g_packet);

		if ($TYPE_WRITEACK != $type && $TYPE_READACK != $type) {
			$retval = 2; # ignore invalid message
			print STDERR "ignoring message with invalid TYPE\n" if ($g_debug);
		}
		if ($PID_RSU != $pid && $PID_RSR != $pid) {
			$retval = 2; # ignore invalid message
			print STDERR "ignoring message with invalid PID\n" if ($g_debug);
		}

		printf STDERR "0x%s 0x%02x 0x%02x %5d 0x%02x 0x%02x 0x%02x 0x%02x %5d %5d %5d %5d\n",
		$ethdr, $type, $status, $framesize, $blp, $rsp, $pid, $regid, $offset, $size, $seqnr, $reserved if ($g_debug);

		if (0 == $retval && 0 != $status) {
			$retval = 1; # fatal
			print STDERR "=== FATAL protocol error: status=", $STATUS{$status}, "\n";
			print STDERR "=== Are you using the correct Ethernet interface? Use -i argument to change, e.g. -i eth1\n";
			exit;
		}
	} until ($retval != 2);

	return ('', $retval, '') if ($retval);

	return (substr($g_packet, 14 + $MEPHDRSIZE), $retval, $ethdr);
}

#
# flash($sock, $pcap, $page, $blockinc, $msg, $cmd, $size, $writecb, $readcb, $image)
#
sub flash
{
	my ($sock, $pcap, $page, $blockinc, $msg, $cmd, $size, $writecb, $readcb, $image) = @_;

	printf STDERR "=== %s page %02d\n", $msg, $page;
	print  STDERR "=== |" . ("-" x $SECTORS_PER_PAGE) . "|\n===  ";
	#print STDERR "Image: $image \n"; 
	
	my ($success) = 1;
	# leave last block for directory entry
	my ($block);
	for ($block = 0; $block < $BLOCKS_PER_PAGE - 1; $block += $blockinc) {
		if ($writecb) {
			$packet  = pack_rsu($TYPE_WRITE, $cmd, $size, ($page * $BLOCKS_PER_PAGE) + $block);

			$packet .= &$writecb($page, $block, $image);

			$sock->send_eth_frame($packet);
			($payload, $error) = readresponse($pcap);

			return $error if ($error);
		}

		if ($readcb) {
			$packet  = pack_rsu($TYPE_READ, $cmd, $size,
			($page * $BLOCKS_PER_PAGE) + $block);

			$sock->send_eth_frame($packet);
			($payload, $error) = readresponse($pcap);
			return $error if ($error);

			$success = &$readcb($page, $block, $payload, $image) && $success;
		}

		if (0 == $block % $BLOCKS_PER_SECTOR) {
			if ($success) { print STDERR "."; }
			else          { print STDERR "X"; }
			$success = 1;
		}
	}
	print STDERR "\n";

	return 0;
}

#
# writedir($sock, $pcap, $page, $entry)
# Write the entry to the last block of page $page
#
sub writedir
{
	my ($sock, $pcap, $page, $entry) = @_;

	die "=== ERR internal error: size of direntry > $BLOCKSIZE" if (length($entry) > $BLOCKSIZE);

	print STDERR "writing dir @", (($page + 1) * $BLOCKS_PER_PAGE) - 1, "\n" if ($g_debug);

	# add zero padding if needed
	$entry .= pack("C", 0) x ($BLOCKSIZE - length($entry)) if (length($entry) < $BLOCKSIZE);

	# write last block with dir entry
	$packet  = pack_rsu($TYPE_WRITE, $REG_RW, $BLOCKSIZE,
	(($page + 1) * $BLOCKS_PER_PAGE) - 1);
	$packet .= $entry;

	my $error = 2;
	$sock->send_eth_frame($packet);

	($result, $error) = readresponse($pcap);

	return $error;
}

#
# readdir($sock, $pcap, $page)
# Read the directory entry from the specified page
#
sub readdirentry
{
	my ($sock, $pcap, $page) = @_;

	my ($packet)  = pack_rsu($TYPE_READ, $REG_RW, $BLOCKSIZE,
	(($page + 1) * $BLOCKS_PER_PAGE) -1);

	$sock->send_eth_frame($packet);

	my $error = 0;
	($entry, $error) = readresponse($pcap);
	return ('', $error) if $error;

	$entry = '' if (substr($entry,0,1) eq pack("C", 0xff));

	return ($entry, $error);
}

#
# erasecb($page, $block, $image)
#
sub erasecb
{
	my ($page, $block, $image) = @_;

	print STDERR "erasecb: $page, $block\n" if ($g_debug);

	return pack("C", $CMD_ERASE);
}

#
# erase($sock, $pcap, $page)
# page = -1 means all pages
#
sub erase
{
	my ($sock, $pcap, $page) = @_;

	return flash($sock, $pcap, $page, $BLOCKS_PER_SECTOR,
	"erasing", $REG_ERASE, 1,
	\&erasecb, 0, '');
}

#
# Verify the data
#
sub verifycb
{
	my ($page, $block, $payload, $image) = @_;

	print STDERR "verifycb: page=$page, block=$block, length(payload)=", length($payload), "\n" if ($g_debug);

	my ($refblock) = substr($image, $block * $BLOCKSIZE, $BLOCKSIZE);

	# append zeroes to fill buffer to $BLOCKSIZE
	$refblock .= pack("C", 0) x ($BLOCKSIZE - length($refblock));

	if ($payload ne $refblock) {
		print STDERR "=== ERR failed to verify block $block in page $page\n" if ($g_debug);
		$g_error = 1;
		return 0;
	}

	return 1;
}

#
# verify($sock, $pcap, $page, $image)
#
sub verify
{
	my ($sock, $pcap, $page, $image) = @_;

	return flash($sock, $pcap, $page, 1,
	"verifying", $REG_RW, $BLOCKSIZE,
	0, \&verifycb, $image);
}

#
# Dump the data to file
#
sub dumpcb
{
	my ($page, $block, $payload, $image) = @_;

	print STDERR "dumpcb: page=$page, block=$block, length(payload)=", length($payload), "\n" if ($g_debug);

	print DUMPFILE $payload;

	return 1;
}

#
# dump($sock, $pcap, $page)
#
sub dump($$$)
{
	my ($sock, $pcap, $page) = @_;

	return flash($sock, $pcap, $page, 1,
	"dumping", $REG_RW, $BLOCKSIZE,
	0, \&dumpcb, '');
}

#
# Write the data
#
sub writepagecb
{
	my ($page, $block, $image) = @_;

	print STDERR "writepagecb: page=$page, block=$block\n" if ($g_debug);

	my ($data) = substr($image, $block * $BLOCKSIZE, $BLOCKSIZE);

	# append zeroes to fill buffer to $BLOCKSIZE
	$data .= pack("C", 0) x ($BLOCKSIZE - length($data));

	return $data;
}

#
# Read and verify the data
#
sub readpagecb
{
	my ($page, $block, $payload, $image) = @_;

	print STDERR "readpagecb: page=$page, block=$block, length(payload)=", length($payload), "\n" if ($g_debug);

	my ($refblock) = substr($image, $block * $BLOCKSIZE, $BLOCKSIZE);

	# append zeroes to fill buffer to $BLOCKSIZE
	$refblock .= pack("C", 0) x ($BLOCKSIZE - length($refblock));

	if ($payload ne $refblock) {
		print STDERR "=== ERR failed to verify block $block in page $page\n" if ($g_debug);
		$g_error = 1;
		return 0;
	}

	return 1;
}

#
# Create a direntry string based on filename
# direntry($filename)
#
sub formatentry
{
	my ($bp_filename, $ap_filename) = @_;

	$stat = stat($bp_filename) || die "=== ERR failed to stat file '$bp_filename'";
	$bp_datetime = ctime($stat->mtime);
	chop($bp_datetime);

	$stat = stat($ap_filename) || die "=== ERR failed to stat file '$ap_filename'";
	$ap_datetime = ctime($stat->mtime);
	chop($ap_datetime);

	# return 'size YYYY mm dd hh:mm bp_filename ; size YYYY mm dd hh:mm ap_filename'
	return sprintf("%s( %s ) ; %s( %s )", $bp_filename, $bp_datetime, $ap_filename, $ap_datetime);
}

#
# writepage($sock, $pcap, $page, $bp_filename, $ap_filename, $image)
#
sub writepage
{
	my ($sock, $pcap, $page, $bp_filename, $ap_filename, $image) = @_;

	$error = flash($sock, $pcap, $page, 1,
	"writing and verifying", $REG_RW, $BLOCKSIZE,
	\&writepagecb, \&readpagecb, $image);

	if (!$error) {
		$error = writedir($sock, $pcap, $page, formatentry($bp_filename, $ap_filename));
	} else {
		# clear entry, an error has occured
		writedir($sock, $pcap, $page, '');
	}
	return $error;
}


#
# Read images from the hex files. 
# then the files are converted from hex to bin format.
#
# readhexfiles($filename_bp, &filename_ap )
#
sub readhexfiles
{
	my ($filename_bp, $filename_ap) = @_;
	local $/ = undef; # read entire file

	# open input and output files
	die "=== ERR no image file specified" if ((!defined($filename_bp)) or (!defined($filename_ap)));
	
	open(IMAGEFILE, $filename_bp) || die "=== ERR failed to open file '$filename_bp'";
	print STDERR "=== Reading '$filename_bp'\n";
	$data = <IMAGEFILE>; # read entire file
	close(IMAGEFILE);
	
	# remove \r\n from data file
	$data =~ s/\r//;
	$data =~ s/\n//;
	
	open(IMAGEFILE, $filename_ap) || die "=== ERR failed to open file '$filename_ap'";
	print STDERR "=== Reading '$filename_ap'\n";
	$data .= <IMAGEFILE>; # read entire file
	close(IMAGEFILE);
	
	# remove \r\n from data file
	$data =~ s/\r//;
	$data =~ s/\n//;
	
	print STDERR "=== Converting '$filename_bp' and '$filename_ap' to binary\n";
	print STDERR "=== |" . ("-" x $PROGRESS_SIZE) . "|\n===  ";

	$i = 0;
	$interval = length($data) / ($PROGRESS_SIZE * 2);
	$interval = 1 if ($interval < .5);

	$bindata = '';
	
	for ($byte = 0; $byte < length($data); $byte += 2) {
		$bytestr = substr($data, $byte, 2);
		
		$bindata .= pack("C", hex($bytestr));
		
		# print progress bar
		$i++;
		print STDERR "." if (0 == $i % $interval);
	}
	
	# append FF's to fill buffer to $IMAGESIZE - 1
	$bindata .= pack("C", 255) x (($PAGESIZE - $BLOCKSIZE) - length($bindata));
	
	print STDERR "\n";
	
	return $bindata;
}

#
# Read RSU status from RSR status register
# readrsustatus($sock, $pcap)
#
sub readrsustatus
{
	my ($sock, $pcap) = @_;

	$size = $REG_STATUS_SIZE;
	$packet = pack($MEPSENDFORMAT,
	0x10, 0xFA, $TYPE_READ, 0x00, $MEPHDRSIZE + $size,
	$ADDR_BLP_NONE, $ADDR_RSP, $PID_RSR, $REG_STATUS, 0, $size, 0, 0);
	
	$sock->send_eth_frame($packet);
	my $error = 0;
	($result, $error) = readresponse($pcap);
	return $error if $error;

	$rsustatus = unpack("C", substr($result, 164, 1));

	print STDERR sprintf("\nRSU Status:\n	Rdy      : %s\n	FpgaType : %s\n	ImageType: %s\n	Trig     : %s\n\n",
	('Configuration ongoing', 'Configuration done')[$rsustatus & 1],
	('loaded from flash', 'loaded via JTAG')[($rsustatus >> 2) & 1],
	('Factory image is running', 'User image is running')[($rsustatus >> 3) & 1],
	('Reconfiguration due to button reset'
	, 'Unknown'
	, 'Reconfiguration due to over temperature'
	, 'Unknown'
	, 'Reconfiguration due to user reset'
	, 'Unknown'
	, 'Reconfiguration due to watchdog reset' )[($rsustatus >> 4) & 7]);

	return $error;
}

#
# Read versions of all FPGA's (handy after reconfig, reset, clear, etc)
# readversions($sock, $pcap)
#
sub readversions
{
	my ($sock, $pcap) = @_;

	# read BP version
	$size = $REG_VERSION_SIZE;
	$packet = pack($MEPSENDFORMAT,
	0x10, 0xFA, $TYPE_READ, 0x00, $MEPHDRSIZE + $size,
	$ADDR_BLP_NONE, $ADDR_RSP, $PID_RSR, $REG_VERSION, 0, $size, 0, 0);
	$sock->send_eth_frame($packet);
	my $error = 0;
	($result, $error) = readresponse($pcap);
	return $error if $error;

	($id, $version) = unpack("CC", $result);
	$maj = $version >> 4;
	$min = $version & 15;
	print STDERR "RSP   version: $id\nBP    version: $maj.$min\n";

	# read AP versions
	for ($ap = 0; $ap < 4; $ap++) {
		$packet = pack($MEPSENDFORMAT,
		0x10, 0xFA, $TYPE_READ, 0x00, $MEPHDRSIZE + $size,
		1 << $ap, 0, $PID_RSR, $REG_VERSION, 0, $size, 0, 0);
		$sock->send_eth_frame($packet);
		($result, $error) = readresponse($pcap);
		return $error if $error;

		($id, $version) = unpack("CC", $result);
		$maj = $version >> 4;
		$min = $version & 15;
		print STDERR "AP[$id] version: $maj.$min\n";
	}

	return 0;
}

#
# Find MAC addresses of active boards
# ping($sock, $pcap)
#
sub ping
{
	my ($sock, $pcap) = @_;

	# read BP version
	$size = $REG_VERSION_SIZE;
	$packet = pack($MEPSENDFORMAT,
	0x10, 0xFA, $TYPE_READ, 0x00, $MEPHDRSIZE + $size,
	$ADDR_BLP_NONE, $ADDR_RSP, $PID_RSR, $REG_VERSION, 0, $size, 0, 0);
	$sock->send_eth_frame($packet);
	my $error = 0;

	while (!$error) {
		($result, $error, $ethdr) = readresponse($pcap);
		$mac = substr($ethdr, 12, 12);
		$mac =~ s/(..)(..)(..)(..)(..)(..)/$1:$2:$3:$4:$5:$6/;
		print STDERR "Found RSP board on MAC: ", $mac, "\n" if (!$error);
	}

	return 0;
}

#
# Reconfigure. Load new firmware from flash into FPGA's. After loading
# has finished, the FPGA will be reset and restart with new firmware.
#
# reconfig($sock, $pcap, $page)
#
sub reconfig
{
	my ($sock, $pcap, $page) = @_;

	$cmd = $page & 15; # use bit 0..3
	$cmd = $cmd | 128;
	$packet  = pack_rsu($TYPE_WRITE, $REG_RECONFIG, 1, 0);
	$packet .= pack("C", $cmd);

	$sock->send_eth_frame($packet);

	# there will be no response
	# ($response, $error) = readresponse($pcap);

	return 0;
}

#
# Reset control for all FPGA's
#
# clearctl($sock, $pcap, $cmd)
# valid values for $cmd:  $CMD_SYNC, $CMD_CLEAR, $CMD_RESET
#
sub clearctl
{
	my ($sock, $pcap, $cmd) = @_;

	$packet  = pack_rsu($TYPE_WRITE, $REG_CLEAR, 1, 0);
	$packet .= pack("C", $cmd);

	$sock->send_eth_frame($packet);
	($response, $error) = readresponse($pcap);

	return $error;
}

#
# Report result
# report($error, $report_ok = false)
#
sub report
{
	my ($error, $report_ok) = @_;

	if ($error || $g_error) {
		print STDERR "=== FAILED\n";
		exit 1;
	}
	else {
		print STDERR "=== OK\n" if ($report_ok);
	}
}

#
# Change range of the page parameter
# chkpage($page)
#
sub chkpage
{
	my ($page) = @_;

	die "=== ERR no flash page specified, use -p" if (!defined($page));
	if ($page < 0 || $page >= $PAGES) {
		die "=== ERR invalid page index $page, should be >= 0 and < $PAGES";
	}
}

#
# Get rsu status and versions
# version_cmd($sock, $pcap)
#
sub version_cmd
{
	my ($sock, $pcap) = @_;

	$error = readrsustatus($sock, $pcap);
	$error = readversions($sock, $pcap) && $error;
	return $error;
}

sub main
{
	my $sock = 0;

	print STDERR "=== rsuctl ($VERSION), for RSP3 boards only\n";
	
	Getopt::Std::getopts("i:m:lexwvb:a:dp:f:FrcsPVhq");

	# check that at least one argument is present
	usage() if ($opt_h);

	if (!defined($opt_m)) {
		print STDERR "=== ERR Must specify MAC address using -m option";
		usage();
	}

	usage() unless ($opt_l || $opt_e || $opt_x || $opt_w || $opt_v ||
	$opt_d || $opt_r || $opt_c || $opt_s || $opt_P || $opt_V || $opt_h);

	# assign default values
	$opt_i = 'eth1'              if !defined($opt_i);

	# Create socket to the boards
	die "=== ERR this program needs to run as root to send/recv raw Ethernet frames" if (0 != geteuid());

	$sock = new Net::RawIP;
	$sock->ethnew($opt_i);
	$sock->ethset(dest => $opt_m);

	$TIMEOUT=5;
	$pcap = $sock->pcapinit($opt_i, "not ether dst $opt_m", $ETHLEN, $TIMEOUT);

	die "=== ERR could not open raw socket to RSP board(s) on $opt_i:$opt_m: $!\n" unless $sock;

	print STDERR "=== connected to RSP board(s) on $opt_i:$opt_m\n";

	if (!defined($opt_q)) {
		print STDERR "=== checking for running RSPDriver\n";
		($payload, $error) = readresponse($pcap);
		die "=== RSPDriver appears to be running, stop it first\n" if ($payload ne '');
	}

	if (defined($opt_l)) {

		# LIST IMAGES IN FLASH
		#
		# readdirentry for all pages

		print STDERR "=== listing flash contents\n\n";
		for ($page = 0; $page < $PAGES; $page++) {
			($entry, $error) = readdirentry($sock, $pcap, $page);
			break if $error;
			print STDERR sprintf("%s %2d: %s\n", 
				(0 == $page ? 'Factory':'Image  '),	$page, $entry) if ((0 == $page) || $entry ne '');
		}
		print STDERR "\n";
		report($error, true);

		return;
	}

	if (defined($opt_e)) {

		# ERASE
		#
		# erase flash page

		die "=== ERR erasing factory image (page 0) prohibited, use -F to force erase of factory image" if (!$opt_F && (0 == $opt_p));
		chkpage($opt_p);

		report(erase($sock, $pcap, $opt_p), true);

		return;
	}

	if (defined($opt_x)) {

		# RECONFIG
		#
		# reconfig BP and AP
		# wait $TO seconds
		# clear
		# wait $TO seconds
		# read rsustatus & version
		
		chkpage($opt_p);
		report(reconfig($sock, $pcap, $opt_p));
		report(clearctl($sock, $pcap, $CMD_RESET));
		report($error) if $error;
		print STDERR "=== waiting $TO seconds after reset\n"; sleep $TO;
		$error = readrsustatus($sock, $pcap);
		$error = readversions($sock, $pcap) && $error;
		report($error, true);

		return;
	}

	if (defined($opt_w)) {

		# WRITE
		#
		# readimage
		# erase page
		# write & verify page

		die "=== ERR writing to factory image (page 0) prohibited, use -F to force write to factory image" if (!$opt_F && (0 == $opt_p));
		chkpage($opt_p);

		$image = readhexfiles($opt_b, $opt_a);
		report(erase($sock, $pcap, $opt_p));
		report(writepage($sock, $pcap, $opt_p, $opt_b, $opt_a, $image), true);

		return;
	}

	if (defined($opt_v)) {

		# VERIFY
		#
		# readimage reference
		# verify flash page against reference

		chkpage($opt_p);
		$image = readhexfiles($opt_b, $opt_a);
		report(verify($sock, $pcap, $opt_p, $image), true);

		return;
	}

	if (defined($opt_d)) {

		# DUMP
		#
		# dump flash page to file

		chkpage($opt_p);
		die "=== ERR no image file specified" if (!defined($opt_f));

		open(DUMPFILE, ">$opt_f") || die "=== ERR failed to open file '$opt_f' for writing\n";
		print STDERR "=== Dumping to file '$opt_f'\n";
		$error = dump($sock, $pcap, $opt_p);
		close(DUMPFILE);
		report($error, true);

		return;
	}

	if (defined($opt_r)) {

		# RESET
		#
		# send reset (factory image (page 0) will start on BP)
		# wait $DTO seconds
		# reconfigure AP's with page 8 (factory default for AP's)
		# wait $TO second
		# read rsustatus and versions

		report(reconfig($sock, $pcap, 0)); # set image 0 (factory image)
		report(clearctl($sock, $pcap, $CMD_RESET));
		print STDERR "=== waiting $TO seconds after reset\n"; sleep $TO;
		print STDERR "=== reconfiguring BP and AP's with factory image\n";

		# don't check the error return code because we know
		# it indicates failure while the reconfig is actually in progress
		
		report(version_cmd($sock, $pcap) && $error, true);

		return;
	}

	if (defined($opt_c)) {

		# CLEAR
		#
		# send clear
		# wait $TO seconds
		# read rsustatus and versions

		report(clearctl($sock, $pcap, $CMD_CLEAR));
		print STDERR "=== waiting $TO seconds after clear\n"; sleep $TO;
		report(version_cmd($sock, $pcap), true);

		return;
	}

	if (defined($opt_s)) {

		# SYNC
		#
		# send sync

		report(clearctl($sock, $pcap, $CMD_SYNC), true);

		return;
	}

	if (defined($opt_P)) {
		report(ping($sock, $pcap), true);
		return;
	}

	if (defined($opt_V)) {
		# versions
		#
		# read rsustatus and versions

		report(version_cmd($sock, $pcap), true);

		return;
	}
}

main();
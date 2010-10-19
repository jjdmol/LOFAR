// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "cd-player.h"

/* default to device /dev/cdrom. If no device is set then use default
 * otherwise use the currentDeviceName set bij setDevice */

const char * CDplayer::defaultDeviceName = "/dev/cdrom";

/* Invalidate cdrom_fd handle and open the device */
CDplayer::CDplayer()
{
	currentDeviceName = (char *) defaultDeviceName;
	cdrom_fd = 0;
	track0 = track1 = 0;
	discId = 0;
	open();
}

/* Close the cdrom_fd handle by closing cdrom */
CDplayer::~CDplayer()
{
	close();
}

/* If cdrom is opened then funtional */
const bool CDplayer::ready()
{
	return cdrom_fd > 0;
}

/* Open the handle explicitly with the currentDeviceName */
const int CDplayer::open()
{
	return open(currentDeviceName);
}

/* Open the handle explicitly with a given devicename */
const int CDplayer::open(const char * deviceName)
{
	if(cdrom_fd <= 0){
		if((cdrom_fd = ::open(deviceName, O_RDONLY)) == -1){
			cerr << "Error on " << deviceName << " detected.\n";
			ERROR("Program error");
		} else {
			struct cdrom_tochdr tochdr;
			if(ioctl(cdrom_fd, CDROMREADTOCHDR, & tochdr) == -1){
				close();
				ERROR("ioctl cdromreadtochdr");
			} else {
				track0 = tochdr.cdth_trk0;
				track1 = tochdr.cdth_trk1;

				readTimes();
			}
		}
	}

	return cdrom_fd;
}

/* Write all times from toc in memory-table */
void CDplayer::readTimes()
{
	struct cdrom_tocentry tocentry;

	total_time = (0 - CD_MSF_OFFSET) / CD_FRAMES;
	for(cdtrck_t i = track0; i <= track1; i++){
		readTocEntry(i, CDROM_MSF, tocentry);
		startTimes[i] = tocentry.cdte_addr.msf.minute * 60 + tocentry.cdte_addr.msf.second;

		readTocEntry(i, CDROM_LBA, tocentry);
		startFrames[i] = tocentry.cdte_addr.lba + CD_MSF_OFFSET;
	}

	readTocEntry(CDROM_LEADOUT, CDROM_MSF, tocentry);
	startTimes[track1 + 1] = tocentry.cdte_addr.msf.minute * 60 + tocentry.cdte_addr.msf.second;
	total_time += startTimes[track1 + 1];

	calculateDiscId();
}

/* Close the device if the handle is valid (i.e. > 0) */
void CDplayer::close(){

	if(cdrom_fd > 0)
		::close(cdrom_fd);

	cdrom_fd = 0;
}

/* Check the channel status and return the standard CDROM_AUDIO flags
 * used in linux/cdrom.h */
const int CDplayer::audio_status() {

	struct cdrom_subchnl subchnl;
	readSubChannel(subchnl);

	return subchnl.cdsc_audiostatus;
}

/* Read the cdrom subchannel information */
void CDplayer::readSubChannel(struct cdrom_subchnl & subchnl){

	subchnl.cdsc_format = CDROM_MSF;
	if(ioctl(cdrom_fd, CDROMSUBCHNL, &subchnl) == -1)
		ERROR("ioctl cdromsubchnl");
}

/* Read cd cdrom table of contents entry of a specified track */
void CDplayer::readTocEntry(const cdtrck_t track, const cdfmt_t format, struct cdrom_tocentry & tocentry)
{
	tocentry.cdte_track  = track;
	tocentry.cdte_format = format;
	if(ioctl(cdrom_fd, CDROMREADTOCENTRY, &tocentry) == -1)
		ERROR("ioctl cdromtocentry");
}

/* Get additional information on current track */
void CDplayer::getPlayingTrack(CDplayer::CDtrack & track)
{
	struct cdrom_subchnl subchnl;
	readSubChannel(subchnl);

	track.setTrack(subchnl.cdsc_trk % 99, subchnl.cdsc_ind % 99);
	track.setCurrenttime(subchnl.cdsc_absaddr.msf.minute % 99, subchnl.cdsc_absaddr.msf.second % 60);

	u_int32_t c_track = track.track();
	track.setTrackTimes(startTimes[c_track], startTimes[c_track + 1]);
}

/* Return the tracknumber */
const cdtrck_t CDplayer::CDtrack::track() const
{
	return (const cdtrck_t) tracknr;
}

/* Return the indexnumber */
const cdindx_t CDplayer::CDtrack::index() const
{
	return (const cdindx_t) indexnr;
}

/* Set the object with te necessary information */
void CDplayer::CDtrack::setTrack(const cdtrck_t track, const cdindx_t index)
{
	tracknr = track;
	indexnr = index;
}

/* Set the object current time (in track) */
void CDplayer::CDtrack::setCurrenttime(const int min, const int sec)
{
	currenttime = min * 60 + sec;
}

/* Set the object time fields */
void CDplayer::CDtrack::setTrackTimes(const int start, const int stop)
{
	starttime = start;
	stoptime  = stop;
}

/* Calculate the elapsed time */
const int CDplayer::CDtrack::timeElapsed() const
{
	return currenttime;
}

/* Calculate the remaining time */
const int CDplayer::CDtrack::timeRemaining() const
{
	return stoptime - currenttime;
}

/* Test with the DATA_CD bit if cd in drive is a data cdrom or audio */
const bool CDplayer::isAudioTrack(const cdtrck_t c_track){

	struct cdrom_tocentry tocentry;
	readTocEntry(c_track, CDROM_MSF, tocentry);

	return (tocentry.cdte_ctrl & CDROM_DATA_TRACK) != CDROM_DATA_TRACK;
}

/* Start playing the track with no offset and index 0 */
void CDplayer::play(const cdtrck_t play_track0, const cdtrck_t play_track1,
		    const int start_time_offset){

	play(play_track0, 1, play_track1, 1, start_time_offset);
}

/* Play a track from track0 to track1. If track1 = 0 then play cd from track0. Test
 * the tracks on valid values and correct with valid ones if not. Index = 1
 * (always) */
void CDplayer::play(const cdtrck_t play_track0, const cdindx_t play_index0,
		    const cdtrck_t play_track1, const cdindx_t play_index1,
		    const int start_time_offset){

	if(isAudioTrack(play_track0)){
		int audio_stat = audio_status();
		if (audio_stat == CDROM_AUDIO_PAUSED){
			if(ioctl(cdrom_fd, CDROMRESUME) == -1)
				ERROR("ioctl cdromresume");
	    	} else {
			struct cdrom_ti ti;
			struct cdrom_msf msf;

			ti.cdti_ind0 = play_index0;
			ti.cdti_ind1 = play_index1;
			ti.cdti_trk0 = (play_track0 >= track0 && play_track0 <= track1) ? play_track0 : track0;
			ti.cdti_trk1 = (play_track1 >= track0 && play_track1 <= track1) ? play_track1 : track1;

			if(ti.cdti_trk0 == ti.cdti_trk1 && start_time_offset > 0){
				u_int32_t stime = start_time_offset + startTimes[ti.cdti_trk0];
				msf.cdmsf_min0 = stime / 60;
				msf.cdmsf_sec0 = stime % 60;
				msf.cdmsf_min1 = startTimes[ti.cdti_trk0 + 1] / 60;
				msf.cdmsf_sec1 = startTimes[ti.cdti_trk0 + 1] % 60;
			} else {
				msf.cdmsf_min0 = 0;
				msf.cdmsf_sec0 = 0;
				msf.cdmsf_min1 = 0;
				msf.cdmsf_sec1 = 0;
			}

			msf.cdmsf_frame0 = CDROM_MSF;
			msf.cdmsf_frame1 = CDROM_MSF;
			if(ioctl(cdrom_fd, CDROMPLAYTRKIND, &ti) == -1)
				ERROR("ioctl cdromplaytrkind");

			if(start_time_offset && ioctl(cdrom_fd, CDROMPLAYMSF, &msf) == -1)
			   	ERROR("ioctl cdromplaymsf");
		}
	}
}

/* Stop the current track */
void CDplayer::stop(){

	if(ioctl(cdrom_fd, CDROMSTOP) == -1)
		ERROR("ioctl cdromstop");
}

/* Pause the current track */
void CDplayer::pause(){

	if(ioctl(cdrom_fd, CDROMPAUSE) == -1)
		ERROR("ioctl cdrompause");
}

/* Eject the current cdrom */
void CDplayer::eject(){

	if(ioctl(cdrom_fd, CDROMEJECT) == -1)
		ERROR("ioctl cdromeject");
}

/* Set the new devicename if it can be opened */
void CDplayer::setDevice(const char * deviceName){

	int current_fd = cdrom_fd;
	cdrom_fd = -1;					// invalidate handle
	if(open(deviceName) == -1){
		cdrom_fd = current_fd;			// revalidate handle
		ERROR("Illegal deviceName used");
	} else {
		if(current_fd > 0)
			::close(current_fd);
	}
}

/* Display the error with extra code guidance */
void CDplayer::error(const char * indication, char * file, int line, char * func){

	perror("Error class CDplayer -");
	cerr << "Error detected in " << file << " (" << line << ")\n\t" <<
				     func << "\n\t(" << indication << ")\n\n";
}

int CDplayer::cddb_sum(int trackTime)
{
	int result = 0;
	while(trackTime > 0) {
		result += (trackTime % 10);
		trackTime /= 10;
	}

	return result;
}

/* Calculate an cddb-discId for an opened cdrom */
void CDplayer::calculateDiscId()
{
	discid_t discTime = 0;
	for(cdtrck_t i = firstTrack(); i <= lastTrack(); i++){
		discTime += cddb_sum(startTimes[i]);
	}

	discId = ((discTime % 0xff) << 24 | total_time << 8 | lastTrack());
}

/* Return the cdda disc-id for the current disc */
const discid_t CDplayer::getDiscId() const
{
	return discId;
}

/* Get the trackinfo on request */
void CDplayer::getTrack(CDplayer::CDtrack & track, const cdtrck_t tracknr)
{
	track.setTrack(tracknr, 1);
	track.setCurrenttime(0, 0);
	if(tracknr >= track0 && tracknr <= track1)
		track.setTrackTimes(startTimes[tracknr], startTimes[tracknr+1]);
	else
		track.setTrackTimes(0, 0);
}

/* Use currentDeviceName(/dev/cdrom as default) to follow linkname and see if it's hd? or sg?/sd? */
const eDeviceType CDplayer::getCurrentDeviceType()
{
	eDeviceType devType = UnknownDevice;

	char linkName[1024];
	memset(linkName, 0, sizeof(linkName));
	if(readlink(currentDeviceName, linkName, sizeof(linkName)) != -1 || errno == EINVAL){

		int fd = ::open(currentDeviceName, O_RDONLY);
		if(fd != -1){
			::close(fd);

			char * devName = rindex(linkName, '/');
			if(devName == 0){
				devName = linkName;
			}

			switch(devName[0]){
				case 's': // scsi device
					devType = ScsiDevice;
					break;
				case 'h': // ide  device
					devType = IdeDevice;
					break;
			}
		}
	}

	return devType;
}

/* Determine type of current device. Because the device gets opened 
 * there is a disc inside. */
const bool CDplayer::hasCDrom()
{
	return getCurrentDeviceType() != UnknownDevice;
}

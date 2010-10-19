// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "record.h"

int stop;				// Arggg... Shitty global.

/* Print the available commandline options */
void printOptions(bool useCDrom)
{
	cerr << "Options:"
		"\n\t-c\tNumber of channels [1/2]"
		"\n\t-s\tSamplerate of the recording"
		"\n\t-b\tBits per sample [8/16]"
		"\n\t-k\tKeep going if destination file already exists (overwrite)"
		"\n\t-P\tUse a higher priority for recording thread"
		"\n\t-A\tAudio device (default /dev/dsp)"
		"\n\t-e\tExecute this statement after recording (eg 'rm $file')"
		"\n\t-S\tThe recording time (mm:ss)"
		"\n\t-f\tOutput format [wav/pcm/cdr/ima3/ima4/ima5]" << ends;
	if(useCDrom == true){
		cerr << "\n\t-t\tTrack to start recording"
			"\n\t-p\tTrack to stop recording"
			"\n\t-l\tTracklist to record; comma separated list (no space)"
			"\n\t-C\tFull cdrom recording (seperate files, ??_file)"
			"\n\t-O\tStarttime of a sample (mm:ss)"
			"\n\t-q\tOutput no track information to screen"
			"\n\t-D\tCDROM device (default /dev/cdrom)"
			"\n\t-M\tMixer device (default /dev/mixer)"
			"\n\t-Q\tQuit on exec-error" << ends;
	}

	cerr << "\n\t-h\tThis information"
		"\n" << endl;
		       
}

/* Test if the file exists and can be created (exists) */
const bool fileCanBeCreated(struct recordSettings * recSettings)
{
	struct stat fstat;
	bool status = recSettings->getForceFileCreate();

	if(status == false && stat(recSettings->getIoFile(), & fstat) == -1){
		if(errno == ENOENT)
			status = true;
		else
			perror("Stat file");
	}
	return status;
}

/* Read from audio device to an intermediat, rotating buffer to reduce
   the numbers of DMA buffer overrun errors.

   Patch submitted by:  Frank Heckenbach <frank@tim.gerwinski.de> */
void * recordRead (volatile struct audio_buffers *buffers)
{
	time_t start_time = time(0);
	bool countSeconds = buffers->numberOfSeconds > 0;
	buffers->recordedBytes = 0L;
	while((buffers->len [buffers->last] = buffers->dsp->read(buffers->buf [buffers->last], buffers->bufferSize)) > 0){
		if(countSeconds && (unsigned long)(time(0) - start_time) > buffers->numberOfSeconds) {
			buffers->len[buffers->last] =
				(buffers->len[buffers->last] * buffers->numberOfSeconds) / (time(0) - start_time);
		}

		buffers->recordedBytes += buffers->len [buffers->last];
		if ((buffers->last = (buffers->last + 1) % NUM_BUFFERS) == buffers->first) {
			fputs("Recording overrun\n", stderr);
			while (buffers->last == buffers->first)
		      		sched_yield();
		}
		write(buffers->pipe[1], "\1", 1);

		if(countSeconds && buffers->numberOfSeconds < (unsigned long)(time(0) - start_time)) stop = 1;
		if(stop) break;
	}
	write(buffers->pipe[1], "\0", 1);
	return NULL;
}

/* Write audio to output using the intermediate buffer

   Patch submitted by:  Frank Heckenbach <frank@tim.gerwinski.de> */
void * recordWrite (volatile struct audio_buffers *buffers)
{
	char c;
	while (1) {
		read(buffers->pipe[0], &c, 1);
		if (!c)
			break;
		buffers->oFile->write(buffers->buf [buffers->first], buffers->len [buffers->first]);
		buffers->first = (buffers->first + 1) % NUM_BUFFERS;
		if (buffers->first != buffers->last)
			printf("Writing behind by %i buffers\n", (buffers->last - buffers->first + NUM_BUFFERS) % NUM_BUFFERS);
	}
	return NULL;
}

/* Read from /dev/dsp the audio and record it in a file

   Partly patched function.
   Patch submitted by:  Frank Heckenbach <frank@tim.gerwinski.de> */
void recordIt(struct recordSettings * recSettings)
{
	struct audio_buffers buffers;
	buffers.oFile = 0;

	if(recSettings->getHighPrioOnAudioThr() == true){
		int policy = SCHED_FIFO;
		pthread_attr_t attr;
		struct sched_param sparam;
		
		sparam.sched_priority = sched_get_priority_max(policy);
		pthread_attr_init(&attr);
		pthread_attr_setschedparam(&attr, &sparam);
	}

	DspSetting dspSetting(recSettings->getSoundDevice(), recSettings->getSampleRate(), O_RDONLY, 
				recSettings->getRecordFormat(), recSettings->getChannels());

	buffers.numberOfSeconds = recSettings->getNumberOfSeconds();
	buffers.dsp = new Dsp(dspSetting);

	switch(recSettings->getFileFormat()){
		case WAV:
		case IMA2:
		case IMA3:
		case IMA4:
		case IMA5:
		case MSADPCM:
			buffers.oFile = new TWave;
			break;
		case PCM:
			buffers.oFile = new TPcm;
			break;
		case CDR:
			buffers.oFile = new TCdr;
	}

	if(buffers.dsp->init()){
		if(fileCanBeCreated(recSettings) == true){
			const int bps = recSettings->getBitsPerSample() / 8;
			buffers.bufferSize = buffers.dsp->getOptimalBufferSize();
			cerr << "Creating file: " << recSettings->getIoFile() << "\n" << endl;

			for (int i = 0; i < NUM_BUFFERS; i++)
				buffers.buf [i] = (char *) malloc(buffers.bufferSize);
			buffers.first = buffers.last = 0;

			if(buffers.oFile->create(recSettings->getIoFile(), 
					recSettings->getFileFormatId(), 
					recSettings->getChannels(),
					recSettings->getOutputBitsPerSample(), 
					recSettings->getSampleRate()) == false){
				perror("Outputfile");
			} else { 
				if(pipe(buffers.pipe)) {
					perror("Pipe");
				} else {
					pthread_t pa, pb;
					pthread_create(&pa,0,(void *(*)(void *)) recordRead,  (void *) &buffers);
					pthread_create(&pb,0,(void *(*)(void *)) recordWrite, (void *) &buffers);
					pthread_join(pa, (void **) NULL);
					pthread_join(pb, (void **) NULL);
					close (buffers.pipe[0]);
					close (buffers.pipe[1]);
				}
			}

			for (int i = 0; i < NUM_BUFFERS; i++)
				free(buffers.buf[i]);

			cerr << "\nEnd of recording: " << recSettings->getIoFile() <<
				" with " << buffers.recordedBytes / bps <<
				" samples" << endl;
		} else {
			cerr << "File can't be created (use -k to override "
				"overwrite protection)." << endl;
			stop = 1;
		}
	} else {
		stop = 2;
		cerr << "Error during initialisation of soundcard." << endl;
	}

	buffers.oFile->close();
	delete buffers.oFile;
	delete buffers.dsp;
}

/* Show info from cdrom-recording  */
void cdromShow(struct cdRecordData * cdRecData)
{
	CDplayer::CDtrack track;
	int time = 0;
	cdtrck_t savedCurrentTrack;
	cdtrck_t trk  = 0;
	cdindx_t indx = 0;
	bool quiet = cdRecData->recSettings->getQuiet();

	if(cdRecData->recSettings->getHighPrioOnAudioThr() == true){
		int policy = SCHED_FIFO;
		pthread_attr_t attr;
		struct sched_param sparam;
		
		sparam.sched_priority = sched_get_priority_min(policy);
		pthread_attr_init(&attr);
		pthread_attr_setschedparam(&attr, &sparam);
	}

	sleep(1);
	do {
		savedCurrentTrack = time ? track.track() : 0;
		cdRecData->cdrom->getPlayingTrack(track);	
		if(time % 10 == 0){
			time = track.timeRemaining();
			if(quiet == false){
				trk = track.track();
				indx = track.index();
			}
		} else {
			time--;
		}
		if(quiet == false){
			cerr << "Number " << trk << ":" << 
				setw(2) << setfill('0') << indx << 
				" " << time / 60 << ":" << 
				setw(2) << setfill('0') <<time % 60 <<
				"\r" << flush;
		}
		sleep(1);
	} while(!stop && (!savedCurrentTrack || (time > 0 && savedCurrentTrack == track.track())) && 
		cdRecData->cdrom->audio_status() == CDROM_AUDIO_PLAY);

	if(!stop)				// normal termination (no signals)
		stop = 1;
}

/* Show info from line-recording, stops on signal */
void lineShow(struct cdRecordData * cdRecData)
{
	time_t start_time = time(0);
	int current_time = 1;
	bool quiet = cdRecData->recSettings->getQuiet();

	if(cdRecData->recSettings->getHighPrioOnAudioThr() == true){
		int policy = SCHED_FIFO;
		pthread_attr_t attr;
		struct sched_param sparam;
		
		sparam.sched_priority = sched_get_priority_min(policy);
		pthread_attr_init(&attr);
		pthread_attr_setschedparam(&attr, &sparam);
	}
	sleep(1);
	do {
		if(quiet == false){
			cerr << "Time " <<
				setw(2) << setfill('0') <<
				current_time / 60 << ":" << 
				setw(2) << setfill('0') <<
				current_time % 60 << "\r" << flush;
		}
		sleep(1);
		if(current_time % 10 == 0) {
			current_time = time(0) - start_time;
		} else {
			current_time++;
		}
	} while(!stop);
}

/* Break from recording loop and exit program */
void shutdown_signal(int signal)
{
	void connect_signal(int the_signal);

	connect_signal(signal);
	stop = 2;
}

/* Connect to a new signal handler */
void connect_signal(int the_signal)
{
	if(signal(the_signal, shutdown_signal) == SIG_ERR){
		perror("Signal error");
		exit(1);
	}
}

/* Get the next track from the track-record list */
int getNextTrack(cdtrck_t & trackIdx, struct recordSettings & recSettings)
{
	if(trackIdx >= 1 && trackIdx <= 99){
		for(; trackIdx <= 99; trackIdx++){
			if(recSettings.track_list[trackIdx]){
				return recSettings.track_list[trackIdx++];
			}
		}
	}

	return -1;
}

/* Test if there are more than one song listed in te 'playlist' */
const bool isMultiTrack(struct recordSettings * recSettings)
{
	return recSettings->track_list[2] != 0;
}

void checkMixerSettingsForCd(struct recordSettings * recSettings)
{
	Mixer mixer;

	mixer.bindMixerToDevice(recSettings->getMixerDevice());

	Channel & channel = mixer.getChannel(SOUND_MIXER_CD);
	if(channel.canChannelRecord() == false){
		cerr << "CDROM device can't record!!" << endl;
		stop = 2;
	} else {
		if(channel.isChannelInRecordState() == false){
			channel.setChannelRecordState(true);
			cerr << "Warning: Settings of mixer were adjusted to record from CD!" << endl;
		}
	}
}

/* Control cdplayer to record a track from dsp */
void cdrom_record(struct recordSettings * recSettings)
{
	void * retval;
	cdtrck_t current_track;
	cdindx_t trackIdx;
	pthread_t pa, pb;
	char file[256];
	const char * iofile = recSettings->getIoFile();
	struct cdRecordData cdRecData;

	if(iofile != 0){
		CDplayer cdrom;

		cdrom.setDevice(recSettings->getCdromDevice());
		if(cdrom.ready()){
			checkMixerSettingsForCd(recSettings);

			connect_signal(SIGINT);		// for now
			connect_signal(SIGTERM);
			connect_signal(SIGQUIT);

			trackIdx = 1;
			recSettings->setIoFile(file);	// Set pointer to file
			cdRecData.cdrom = & cdrom;
			cdRecData.recSettings = recSettings;
			while((current_track = getNextTrack(trackIdx, (*recSettings))) &&
					current_track <= cdrom.lastTrack() &&
					current_track > 0 && stop != 2){
				if(cdrom.isAudioTrack(current_track)){

					if(isMultiTrack(recSettings))
						sprintf(file, "%02d_%s", current_track, iofile);
					else
						sprintf(file, "%s", iofile);
					cdrom.play(current_track, current_track, recSettings->getStartTimeOfSamples());
					stop = 0;

					pthread_create(&pa,0,(void *(*)(void *)) recordIt, recSettings);
					pthread_create(&pb,0,(void *(*)(void *)) cdromShow, & cdRecData);
					pthread_join(pa,(void **)&retval);
					pthread_join(pb,(void **)&retval);

					if(stop == 1 && recSettings->getExecStmt() != 0){
						int pid, status;
						if(!(pid = fork())){
							/* Patch setenv by
							 * Michael Fowler
							 * <michael@shoebox.net>
							 * */
							setenv("file", recSettings->getIoFile(), 1);
							execl("/bin/sh", "sh", "-c", recSettings->getExecStmt(), 0);
							exit(0);
						}

						waitpid(pid, &status, 0);

                                                if(status != 0 && recSettings->getQuitOnError() == true){
                                                        cerr << "Child exits with " << status << endl;
                                                        stop = 2;
                                                }
					}
					if(stop == 1) stop = 0;
				} else {
					cerr << "Cannot read CDROM data-tracks." << endl;
				}
			}
		}

		cdrom.stop();

		if(stop == 2){
			cerr << "Unlinking file due to abnormal termination." << endl;
			unlink(recSettings->getIoFile());
		}
	}
}

/* record the source on /dev/dsp and quit on signals */
void line_record(struct recordSettings * recSettings)
{
	void * retval;
	pthread_t pa, pb;
	struct cdRecordData cdRecData;

	stop = 0;
	if(recSettings->getIoFile()){
		connect_signal(SIGINT);		// for now
		connect_signal(SIGTERM);

		cerr << "Record from dsp (no cdrom support).\nTo end the recording press CTRL-C" << endl;

		cdRecData.cdrom = 0;
		cdRecData.recSettings = recSettings;

		pthread_create(&pa,0,(void *(*)(void *)) recordIt, recSettings);
		pthread_create(&pb,0,(void *(*)(void *)) lineShow, &cdRecData);
		pthread_join(pa,(void **)&retval);
		pthread_join(pb,(void **)&retval);

		if(recSettings->getExecStmt() != 0){
			int pid, status;
			if(!(pid = fork())){
				/* Patch setenv by Michael Fowler
				 * <michael@shoebox.net> */
				setenv("file", recSettings->getIoFile(), 1);
				execl("/bin/sh", "sh", "-c", recSettings->getExecStmt(), 0);
				exit(0);
			}

			waitpid(pid, &status, 0);

			if(status != 0 && recSettings->getQuitOnError() == true){
				cerr << "Child exits with " << status << endl;
				stop = 2;
			}
		}

	}
}

// Process info from a rc-file to recSettings object
void readSettings(RCfile & rc, recordSettings  & recSettings)
{
	if(rc.isOpen() == true){
		char buffer[LINEBUFFERSIZE];

		if(rc.getEntry("channels", buffer, sizeof(buffer)) == true){
			if(!strcmp(buffer, "mono")){
				recSettings.setChannels(1);
			} else {
				if(strcmp(buffer, "stereo")) {
					cerr << "Illegal channels setting: " <<
						buffer << " defaulting to stereo." << endl;
				}
				recSettings.setChannels(2);
			}
		}

		if(rc.getEntry("samplerate", buffer, sizeof(buffer)) == true){

			u_int32_t sampleRate;
			sscanf(buffer, "%ul", &sampleRate);

			recSettings.setSampleRate(sampleRate);
		}

		if(rc.getEntry("fileformat", buffer, sizeof(buffer)) == true){
			OUTPUT_FORMATS fmt = NONE;
			if(!strcmp(buffer, "wave")) fmt = WAV;
			if(!strcmp(buffer, "pcm")) fmt = PCM;
			if(!strcmp(buffer, "cdr")) fmt = CDR;

			if(fmt == NONE){
				fmt = WAV;
				cerr << "Format " << buffer << " unknown." <<
					"Using default format WAV." << endl;
			}
			recSettings.setFileFormat(fmt);
		}

		if(rc.getEntry("format", buffer, sizeof(buffer)) == true){
			struct sSoundType {char * name; int format;} r[] =
				{ {"MU LAW",    AFMT_MU_LAW},    {"A LAW", AFMT_A_LAW},
				  {"IMA ADPCM", AFMT_IMA_ADPCM}, {"U8",    AFMT_U8},
				  {"U16le",     AFMT_U16_LE},    {"U16be", AFMT_U16_BE},
				  {"S8",        AFMT_S8},        {"S16le", AFMT_S16_LE},
				  {"S16be",     AFMT_S16_BE}
				};

			for(int i = 0; i < 9; i++){
				if(!strcmp(buffer, r[i].name)){
					recSettings.setRecordFormat(r[i].format);
					break;
				}
			}
		}

		if(rc.getEntry("QuitOnExec", buffer, sizeof(buffer)) == true){
			if(!strcmp(buffer, "true")) recSettings.setQuitOnError(true);
			if(!strcmp(buffer, "false")) recSettings.setQuitOnError(false);
		}

		if(rc.getEntry("sounddevice", buffer, sizeof(buffer)) == true){
			int len = strlen(buffer);
			while(len > 0 && buffer[len-1] == ' ')
				buffer[--len] = '\0';
			recSettings.setSoundDevice(buffer);
		}

		if(rc.getEntry("mixerdevice", buffer, sizeof(buffer)) == true){
			int len = strlen(buffer);
			while(len > 0 && buffer[len-1] == ' ')
				buffer[--len] = '\0';
			recSettings.setMixerDevice(buffer);
		}

		if(rc.getEntry("cdromdevice", buffer, sizeof(buffer)) == true){
			int len = strlen(buffer);
			while(len > 0 && buffer[len-1] == ' ')
				buffer[--len] = '\0';
			recSettings.setCdromDevice(buffer);
		}
	}
}

/* Set the default settings for program */
void defaultSettings(recordSettings & recSettings)
{
	char * home, * home_env = getenv("HOME");
	home = new char[strlen(home_env) + 50];

	strcpy(home, "/etc/soundrecorder.conf");
	RCfile rc(home, false);

	strcpy(home, home_env);
	strcat(home, "/.soundrecorder");

	rc.open(home, false, true);
	readSettings(rc, recSettings);
	rc.close();

	delete [] home;
}

/* Create a list with the tracknumbers from -tlst argument */
void expandList(struct recordSettings & recSettings, char * track_list)
{
	u_int32_t i = 0, from, to = 1, dst = 1;
	while(i < strlen(track_list)){
		from = 0;
		if(track_list[i] < '0' || track_list[i] > '9'){
			if(track_list[i] == '-')
				from = to;
			to = 0;
			i++;
		}

		sscanf(track_list + i,"%d", & to);
		while(track_list[i] >= '0' && track_list[i] <= '9') i++;
		if(from){
			for(u_int32_t j = from + 1; j <= to; j++)
				recSettings.track_list[dst++] = j;
		} else {
			recSettings.track_list[dst++] = to;
		}
	}
}

const bool threadScheduleAvailable()
{
	printf("Using POSIX Priority Threading.\n");
	if(sysconf(_SC_THREAD_PRIORITY_SCHEDULING) == 1)
		return true;
	else
		return false;
}

/* Extract extra parameters for recordsettings */
bool parseArgs(int argc, char ** argv, recordSettings & recSettings, bool useCDrom)
{
	bool status = true;
	long lng;
	int opt, i, min, sec, tstt = 0, tstp = 0;
	u_int16_t forceBpsTo = 0;
	defaultSettings(recSettings);

	while((opt = getopt(argc, argv, useCDrom ? "c:s:f:t:p:l:e:S:O:D:M:A:b:ChkPqQ" : "c:s:f:e:S:b:A:hkPq")) != -1){
		switch(opt){
			case '?':
			case 'h':
				printOptions(useCDrom);
				return false;
			case 'c':
				sscanf(optarg, "%lu", &lng);
				status &= recSettings.setChannels(lng);
				break;
			case 's':
				sscanf(optarg, "%lu", &lng);
				status &= recSettings.setSampleRate(lng);
				break;
			case 'f':
				if(!strcasecmp("wav", optarg)){
					status &= recSettings.setFileFormat(WAV);
					recSettings.setFileFormatId(0x0001);
				}

				if(!strcasecmp("pcm", optarg)){
					status &= recSettings.setFileFormat(PCM);
					recSettings.setFileFormatId(0x0001);
				}
				
				if(!strcasecmp("cdr", optarg)){
					status &= recSettings.setFileFormat(CDR);
					recSettings.setFileFormatId(0x0001);
				}
				
				if(!strcasecmp("ima2", optarg)){ 
					status &= recSettings.setFileFormat(IMA2);
					forceBpsTo = 16;
					recSettings.setFileFormatId(0x0011);
					recSettings.setOutputBitsPerSample(2);
				}
				
				if(!strcasecmp("ima3", optarg)){
					status &= recSettings.setFileFormat(IMA3);
					forceBpsTo = 16;
					recSettings.setFileFormatId(0x0011);
					recSettings.setOutputBitsPerSample(3);
				}
				
				if(!strcasecmp("ima4", optarg)){
					status &= recSettings.setFileFormat(IMA4);
					forceBpsTo = 16;
					recSettings.setFileFormatId(0x0011);
					recSettings.setOutputBitsPerSample(4);
				}
				
				if(!strcasecmp("ima5", optarg)){
					status &= recSettings.setFileFormat(IMA5);
					forceBpsTo = 16;
					recSettings.setFileFormatId(0x0011);
					recSettings.setOutputBitsPerSample(5);
				}
				
				if(!strcasecmp("msadpcm", optarg)){
					status &= recSettings.setFileFormat(MSADPCM);
					forceBpsTo = 16;
					recSettings.setFileFormatId(0x0002);
					recSettings.setOutputBitsPerSample(4);
				}
				
				break;
			case 'b':
				sscanf(optarg, "%d", &i);
				recSettings.setBitsPerSample(i == 8 ? 8 : 16);
				recSettings.setRecordFormat(i == 8 ? AFMT_U8 : AFMT_S16_LE);
				break;
			case 't':
				sscanf(optarg, "%d", &tstt);
				recSettings.track_list[1] = tstt;
				break;
			case 'p':
				if(tstt > 0){
					sscanf(optarg, "%d", &tstp);
					for(i = 1; tstt <= tstp; i++){
						recSettings.track_list[i] = tstt++;
					}
				} else {
					cerr << "No start given with -t" << endl;
				}
				break;
			case 'l':
				expandList(recSettings, optarg);
				break;
			case 'e':
				status &= recSettings.setExecStmt(optarg);
				break;
			case 'C':
				for(i = 1; i < 99; i++)
					recSettings.track_list[i] = i;
				break;
			case 'S':
				if(sscanf(optarg, "%d:%d", &min, &sec) == 2) {
					cerr << "Setting recording limit to "
						<< (60 * min + sec) << " seconds\n";
					status &= recSettings.setNumberOfSeconds(60 * min + sec);
				} else {
					cerr << "Error in timedefinition -S\n";
					status = -1;
				}
				break;
			case 'O':
				sscanf(optarg, "%d:%d", &min, &sec);
				status &= recSettings.setStartTimeOfSamples(min * 60 + sec);
				break;
			case 'q':
				recSettings.setQuiet(true);
				break;
			case 'k':
				recSettings.setForceFileCreate(true);
				break;
			case 'P':
				recSettings.setHighPrioOnAudioThr(true);
				break;
			case 'Q':
				recSettings.setQuitOnError(true);
				break;
			case 'D':
				recSettings.setCdromDevice(optarg);
				break;
			case 'M':
				recSettings.setMixerDevice(optarg);
				break;
			case 'A':
				recSettings.setSoundDevice(optarg);
				break;
		}
	}

	status &= recSettings.setIoFile(argv[optind]);

	if(forceBpsTo){
		if(recSettings.getBitsPerSample() != forceBpsTo){
			cerr << "Output format forces change of inputformat."
			     << endl << "Using " << forceBpsTo << " bit p/s"
			     << endl;
			recSettings.setBitsPerSample(forceBpsTo);
		}
	}

	if(recSettings.getHighPrioOnAudioThr() == true && threadScheduleAvailable() == false){
		cerr << "Priority scheduling not supported on this system." << endl;
		recSettings.setHighPrioOnAudioThr(false);
	}
	
	if(status == false){
		printOptions(useCDrom);
	}

	return status;
}

/* Commandline coordinated use of binary */
int main(int argc, char **argv)
{
	cerr << "Sound Recorder version " VERSION "\n"
		"Copyright (C) 1997-2000 by B. Warmerdam under GPL.\n"
		"This program is free software and comes with ABSOLUTELY NO WARRANTY.\n" << endl;

	int len = strlen(argv[0]);
	bool useCDrom = (len >= 16 && ! strcmp(argv[0] + len - 16, "cdsound-recorder"));

	try{
		struct recordSettings recSettings;
		if(parseArgs(argc, argv, recSettings, useCDrom) == true){
			if(useCDrom == true){
				cdrom_record(& recSettings);
			} else {
				line_record(& recSettings);
			}
		}
	}
	catch(char * err){
		cerr << "Error: " << err << endl;
	}
	return 0;
}


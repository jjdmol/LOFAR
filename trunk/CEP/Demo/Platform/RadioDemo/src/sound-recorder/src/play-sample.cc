// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "play-sample.h"

/* Print the available commandline options */
void printOptions()
{
	cerr << "Options:"
		"\n\t-c\tNumber of channels [1/2]"
		"\n\t-s\tSamplerate of the recording"
		"\n\t-b\tBits per sample [8/16]"
		"\n\t-f\tOutput format [wav/pcm/cdr/ima3/ima4/ima5]"
		"\n\t-A\tAudio device (default /dev/dsp)"
		"\n\t-q\tQuiet mode (no additional info to screen)"
		"\n\t-h\tThis information"
		"\n" << endl;
}

/* Read audio from a file and play it on /dev/dsp */
void playIt(struct recordSettings * recSettings)
{
	int  len;
	bool status = true;
	char * audio_buffer;
	int nr_samples = 0;
	int bufferSize;

	TFileFormat * iFile = 0;

	int fFormat = recSettings->getFileFormat();
	switch(fFormat){
		case WAV:
			chnl_t chnl;
			bps_t bps;
			smpl_t smpl;
			int bs;

			iFile = new TWave;

			if((status = ((TWave *) iFile)->open((const char *) recSettings->getIoFile())) == true){
				((TWave *) iFile)->getWaveInfo(chnl, bps, smpl, bs);
				recSettings->setBitsPerSample(bps);
				recSettings->setRecordFormat((bps == 16) ? AFMT_S16_LE : AFMT_U8);
				recSettings->setSampleRate(smpl);
				recSettings->setChannels(chnl);
				nr_samples = ((TWave *) iFile)->sampleCount();
			}
			break;
		case PCM:
		case CDR:
			u_int32_t sampleSize;
			struct stat filestatus;

			if(fFormat == PCM) {
				iFile = new TPcm;
			} else {
				if(fFormat == CDR)
					iFile = new TCdr;
				else
					throw "Error assigning format";
			}

			if(stat(recSettings->getIoFile(), & filestatus) == -1){
				perror("Play-error");
				status = false;
			} else {
				sampleSize = (recSettings->getChannels() * recSettings->getBitsPerSample() *
					recSettings->getSampleRate()) / 8;

				nr_samples = filestatus.st_size / sampleSize;
			}
			break;
	}

	if(status == true){
		DspSetting dspSetting(recSettings->getSoundDevice(), recSettings->getSampleRate(), O_WRONLY,
			recSettings->getRecordFormat(), recSettings->getChannels());
		Dsp dsp(dspSetting);

		if(dsp.init()){
			bufferSize = dsp.getOptimalBufferSize();

			cerr << "Reading file: " << recSettings->getIoFile() <<
				" (ETA: " << nr_samples / 60 << ":" <<
				setw(2) << setfill('0') << nr_samples % 60 <<
				")" << endl;

			audio_buffer = new char[bufferSize];
			iFile->open(recSettings->getIoFile());

			while((len = iFile->read(audio_buffer, bufferSize)) > 0){
				dsp.write(audio_buffer, len);
			}
			
			delete [] audio_buffer;
			cerr << "End of file: " << recSettings->getIoFile() << endl;
		} else {
			cerr << "Error during initialisation of soundcard." << endl;
		}
	}

	iFile->close();
	delete iFile;
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
			if(!strcmp(buffer, "wave")) recSettings.setFileFormat(WAV);
			if(!strcmp(buffer, "pcm")) recSettings.setFileFormat(PCM);
			if(!strcmp(buffer, "cdr")) recSettings.setFileFormat(CDR);

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

/* Extract extra parameters for recordsettings */
bool parseArgs(int argc, char ** argv, recordSettings & recSettings)
{
	bool status = true;
	long lng;
	int  opt, i;

	defaultSettings(recSettings);

	while((opt = getopt(argc, argv, "c:s:f:A:b:hq")) != -1){
		switch(opt){
			case '?':
			case 'h':
				printOptions();
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
				if(!strcasecmp("wav", optarg)) status &= recSettings.setFileFormat(WAV);
				if(!strcasecmp("pcm", optarg)) status &= recSettings.setFileFormat(PCM);
				if(!strcasecmp("cdr", optarg)) status &= recSettings.setFileFormat(CDR);
				break;
			case 'b':
				sscanf(optarg, "%d", &i);
				recSettings.setBitsPerSample(i == 8 ? 8 : 16);
				recSettings.setRecordFormat(i == 8 ? AFMT_U8 : AFMT_S16_LE);
				break;
			case 'M':
				recSettings.setMixerDevice(optarg);
				break;
			case 'A':
				recSettings.setSoundDevice(optarg);
				break;
			case 'q':
				recSettings.setQuiet(true);
				break;
		}
	}

	status &= recSettings.setIoFile(argv[optind]);

	if(status == false){
		printOptions();
	}

	return status;
}

int main(int argc, char **argv)
{
	cerr << "Play Sample version " VERSION "\n"
		"Copyright (C) 1997-2000 by B. Warmerdam under GPL.\n"
		"This program is free software and comes with ABSOLUTELY NO WARRANTY.\n" << endl;
 
	struct recordSettings recSettings;
	if(parseArgs(argc, argv, recSettings) && recSettings.getIoFile()){
		try{
			int i;

			// ReFind the first commandline option that is a filename
			for(i = 1; i < argc; i++)
				if(argv[i] == recSettings.getIoFile())
					break;

			// Play from this argument on, the rest
			for(; i < argc; i++){
				recSettings.setIoFile(argv[i]);
				playIt(&recSettings);
			}
		}
		catch(const char * msg){
			cerr << "Error: " << msg << endl;
		}
	}

	return 0;
}

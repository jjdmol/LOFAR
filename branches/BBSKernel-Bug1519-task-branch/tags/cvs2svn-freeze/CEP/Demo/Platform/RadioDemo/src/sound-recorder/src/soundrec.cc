// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "soundrec.h"

/* Set all variables on a default value */
recordSettings::recordSettings()
{
	iofile = 0;
	memset(track_list, 0, sizeof(track_list));
	bitsPerSample = 16;
	outputBitsPerSample = 0;
	recordFormat  = AFMT_S16_LE;
	sampleRate = 44100;
	channels   = 2;
	fileFormat = WAV;
	exec_stmt  = 0;
	numberOfSeconds = 0;
	startTimeOfSamples = 0;
	stopTimeOfSamples  = 0;
	show = false;
	quiet = false;
	quitOnError = false;
	forceFileCreate = false;
	highPrioOnAudioThr = false;
	fileFormatId = 1;
	strcpy(soundDevice, "/dev/dsp");
	strcpy(mixerDevice, "/dev/mixer");
	strcpy(cdromDevice, "/dev/cdrom");
}

/* Deconstruct settings */
recordSettings::~recordSettings()
{
}

/* Set channels of recording (1 or 2) */
const bool recordSettings::setChannels(const u_int32_t _channels)
{
	bool status = true;

	if(_channels < 3 && _channels >= 1){
		channels = _channels;
	} else {
		cerr << "Channels can be 1 or 2, not " << _channels << endl;
		status = false;
		channels = 2;
	}

	return status;
}

/* Set samplerate; only a fixed set of usual settings allowed */
const bool recordSettings::setSampleRate(const u_int32_t _sampleRate)
{
	bool status = true;
	
	switch(_sampleRate){
		case 8000:  case 11025: case 12000: case 16000:
		case 22050: case 32000: case 44100: case 48000:
			    sampleRate = _sampleRate;
			    break;
		default:
			cerr << "Illegal samplingrate " << _sampleRate <<
				" defaulting to 44100." << endl;
			sampleRate = 44100;
			status = false;
	}

	return status;
}

/* Set filetype of in/output file */
const bool recordSettings::setFileFormat(const u_int32_t _fileFormat)
{
	bool status = true;
	
	if(_fileFormat >= WAV && _fileFormat <= MSADPCM){
		fileFormat = _fileFormat;
	} else {
		cerr << "Illegal fileformat " << _fileFormat << endl;
		fileFormat = WAV;
		status = false;
	}

	return status;
}

/* Set the type of audio format the soundcard produces/uses */
const bool recordSettings::setRecordFormat(const u_int32_t _recordFormat)
{
	bool status = false;
	u_int32_t vals[] = {AFMT_MU_LAW, AFMT_A_LAW, AFMT_IMA_ADPCM, AFMT_U8,
				AFMT_U16_LE, AFMT_U16_BE, AFMT_S8, AFMT_S16_LE, AFMT_S16_BE};

	for(u_int32_t i = 0; i < sizeof(vals)/sizeof(vals[1]); i++)
		if(vals[i] == _recordFormat){
			recordFormat = _recordFormat;
			status = true;
			break;
		}

	return status;
}

/* Set the number of bits per sample (8 or 16) */
const bool recordSettings::setBitsPerSample(const u_int32_t _bitsPerSample)
{
	bool status = false;

	switch(_bitsPerSample){
		case 8:
		case 16:
			status = true;
			bitsPerSample = _bitsPerSample;
			break;
		default:
			cerr << "Illegal bits-per-sample value: " <<
				_bitsPerSample << endl;
		}

	return status;
}

/* Set the number of bits per sample (8 or 16) */
const bool recordSettings::setOutputBitsPerSample(const u_int32_t _outputBitsPerSample)
{
	bool status = false;

	switch(_outputBitsPerSample){
		case 2:
		case 3:
		case 4:
		case 5:
		case 8:
		case 16:
			status = true;
			outputBitsPerSample = _outputBitsPerSample;
			break;
		default:
			cerr << "Illegal bits-per-sample value: " <<
				_outputBitsPerSample << endl;
		}

	return status;
}

/* Set the execute statement used in recording */
const bool recordSettings::setExecStmt(const char * _statement)
{
	if(_statement != 0){
		exec_stmt = (char *) _statement;
	}

	return (_statement != 0);
}

/* Store the filename used in recording and playback */
const bool recordSettings::setIoFile(const char * _iofile)
{
	if(_iofile != 0){
		iofile = (char *) _iofile;
	}

	return (_iofile != 0);
}

/* Set the number of samples that are recorded */
const bool recordSettings::setNumberOfSeconds(const long _numberOfSeconds)
{
	numberOfSeconds = _numberOfSeconds;
	return true;
}

/* Set the sounddevice used in recording and playback */
const bool recordSettings::setSoundDevice(const char * _soundDevice)
{
	soundDevice[0] = '\0';
	strncpy(soundDevice, _soundDevice, (strlen(_soundDevice) + 1) % sizeof(soundDevice));
	return true;
}

/* Set the format id of the output file (wav format id) */
const bool recordSettings::setFileFormatId(const u_int16_t _fileFormatId)
{
	fileFormatId = _fileFormatId;
	return true;
}

/* Get the number of channels used in recording and playback */
const u_int32_t recordSettings::getChannels() const
{
	return channels;
}

/* Get the number of bits per sample */
const u_int32_t recordSettings::getBitsPerSample() const
{
	return bitsPerSample;
}

/* Get the number of bits per sample used in outputfile */
const u_int32_t recordSettings::getOutputBitsPerSample() const
{
	return outputBitsPerSample > 0 ? outputBitsPerSample : bitsPerSample;
}

/* Set the qoe flag so recording ends when exec errors occur */
const bool recordSettings::setQuitOnError(const bool _quitOnError)
{
	quitOnError = _quitOnError;
	return true;
}

/* Get the samplerate used for recording/playback */
const bool recordSettings::setForceFileCreate(const bool _forceFileCreate)
{
	forceFileCreate = _forceFileCreate;
	return true;
}

/* Are priority threads used on audio threads */
const bool recordSettings::setHighPrioOnAudioThr(const bool _highPrioOnAudioThr)
{
	highPrioOnAudioThr = _highPrioOnAudioThr;
	return true;
}

/* Get the flag if dspsettings are to be shown */
const u_int32_t recordSettings::getSampleRate() const
{
	return sampleRate;
}

/* Get the flag if output to tty's are minimized */
const bool recordSettings::getQuiet() const
{
	return quiet;
}

/* Set the starttime of a recording in samples */
const bool recordSettings::setStartTimeOfSamples(const long _startTimeOfSamples)
{
	startTimeOfSamples = _startTimeOfSamples;
	return true;
}

/* Set the stoptime of a recording in samples */
const bool recordSettings::setStopTimeOfSamples(const long _stopTimeOfSamples)
{
	stopTimeOfSamples = _stopTimeOfSamples;
	return true;
}

/* Set flag is dspsettings are to be shown */
const bool recordSettings::setShowSettings(const bool _show)
{
	show = _show;
	return true;
}

/* Set the quiet flag */
const bool recordSettings::setQuiet(const bool _quiet)
{
	quiet = _quiet;
	return true;
}

/* Set the mixer device */
const bool recordSettings::setMixerDevice(const char * _mixerDevice)
{
	mixerDevice[0] = '\0';
	strncpy(mixerDevice, _mixerDevice, (strlen(_mixerDevice) + 1) % sizeof(mixerDevice));
	return true;
}

/* Set the cdrom device */
const bool recordSettings::setCdromDevice(const char * _cdromDevice)
{
	cdromDevice[0] = '\0';
	strncpy(cdromDevice, _cdromDevice, (strlen(_cdromDevice) + 1) % sizeof(cdromDevice));
	return true;
}

/* Get the name of record/playback file */
const char * recordSettings::getIoFile() const
{
	return iofile;
}

/* Get the statement to be executed after every recorded track/sample */
const char * recordSettings::getExecStmt() const
{
	return exec_stmt;
}

/* Get the starttime of recording in samples */
const long recordSettings::getStartTimeOfSamples() const
{
	return startTimeOfSamples;
}

/* Get the stoptime of recording in samples */
const long recordSettings::getStopTimeOfSamples() const
{
	return stopTimeOfSamples;
}

/* Get the qoe flag */
const bool recordSettings::getQuitOnError() const
{
	return quitOnError;
}

/* Get the name of the device used for recording/playback */
const bool recordSettings::getForceFileCreate() const
{
	return forceFileCreate;
}

/* Get the fileformat used for recording/playback */
const bool recordSettings::getHighPrioOnAudioThr() const
{
	return highPrioOnAudioThr;
}

/* Get the name of the device used for recording/playback */
const char * recordSettings::getSoundDevice() const
{
	return soundDevice;
}

/* Get the fileformat used for recording/playback */
const u_int32_t recordSettings::getFileFormat() const
{
	return fileFormat;
}

/* Get the format or recording/playback used by soundcard */
const u_int32_t recordSettings::getRecordFormat() const
{
	return recordFormat;
}

/* Get number of samples to record */
const u_int32_t long recordSettings::getNumberOfSeconds() const
{
	return numberOfSeconds;
}

/* Get mixer device name (default /dev/mixer) */
const char * recordSettings::getMixerDevice() const
{
	return mixerDevice;
}

/* Get cdrom device name (default /dev/cdrom) */
const char * recordSettings::getCdromDevice() const
{
	return cdromDevice;
}

/* Get format id for type of file (wav format id) */
const u_int16_t recordSettings::getFileFormatId() const
{
	return fileFormatId;
}

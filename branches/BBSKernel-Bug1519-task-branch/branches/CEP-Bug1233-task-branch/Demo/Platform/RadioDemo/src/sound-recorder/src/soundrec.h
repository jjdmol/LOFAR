// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _soundrec_h
#define _soundrec_h

#include <sys/types.h>
#include <sys/soundcard.h>
#include <string.h>
#include <iostream.h>

enum OUTPUT_FORMATS { NONE = 0, WAV = 1, PCM, CDR, IMA2, IMA3, IMA4, IMA5, MSADPCM };

class recordSettings {

	public:
		recordSettings();
		~recordSettings();

		const bool		setChannels(const u_int32_t _channels);
		const bool		setSampleRate(const u_int32_t _sampleRate);
		const bool		setFileFormat(const u_int32_t _fileFormat);
		const bool		setRecordFormat(const u_int32_t _recordFormat);
		const bool		setBitsPerSample(const u_int32_t _bitsPerSample);
		const bool		setOutputBitsPerSample(const u_int32_t _outputBitsPerSample);
		const bool		setExecStmt(const char * _statement);
		const bool		setIoFile(const char * _iofile);
		const bool		setNumberOfSeconds(const long _numberOfSeconds);
		const bool		setStartTimeOfSamples(const long _startTimeOfSamples);
		const bool		setStopTimeOfSamples(const long _stopTimeOfSamples);
		const bool		setShowSettings(const bool _show);
		const bool		setQuiet(const bool _quiet);
		const bool		setQuitOnError(const bool _quitOnError);
		const bool		setForceFileCreate(const bool _forceFileCreate);
		const bool		setHighPrioOnAudioThr(const bool _highPrioOnAudioThr);
		const bool		setSoundDevice(const char * _soundDevice);
		const bool		setMixerDevice(const char * _mixerDevice);
		const bool		setCdromDevice(const char * _cdromDevice);
		const bool		setFileFormatId(const u_int16_t _fileFormatId);

		const u_int32_t		getChannels() const;
		const u_int32_t		getBitsPerSample() const;
		const u_int32_t		getOutputBitsPerSample() const;
		const u_int32_t		getSampleRate() const;
		const u_int32_t		getFileFormat() const;
		const u_int32_t		getRecordFormat() const;
		const bool		getQuiet() const;
		const char		* getIoFile() const;
		const char		* getExecStmt() const;
		const long		getStartTimeOfSamples() const;
		const long		getStopTimeOfSamples() const;
		const bool		getQuitOnError() const;
		const bool		getForceFileCreate() const;
		const bool		getHighPrioOnAudioThr() const;
		const char *		getSoundDevice() const;
		const u_int32_t long	getNumberOfSeconds() const;
		const char *		getMixerDevice() const;
		const char *		getCdromDevice() const;
		const u_int16_t		getFileFormatId() const;

		char			track_list[100];		// track 00-99
	private:
		char			* iofile;
		u_int32_t		bitsPerSample, outputBitsPerSample;
		u_int32_t		recordFormat;
		u_int32_t		sampleRate;
		int			channels;
		int			fileFormat;
		char			* exec_stmt;
		u_int32_t long		numberOfSeconds;
		u_int32_t		startTimeOfSamples;
		u_int32_t		stopTimeOfSamples;
		bool			show;
		bool			quiet;
		bool			quitOnError;
		bool			forceFileCreate;
		bool			highPrioOnAudioThr;
		u_int16_t		fileFormatId;

		char			soundDevice[1024];
		char			mixerDevice[1024];
		char			cdromDevice[1024];
};

#endif

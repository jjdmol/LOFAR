// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

/* class definition of the cd-player */

/* Created on: December 9 1997       */

#ifndef _cd_player_h
#define _cd_player_h

#include <unistd.h>
#include <stdio.h>
#include <iostream.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <exception>
#include "types.h"

#define ERROR perror

enum eDeviceType { UnknownDevice, IdeDevice, ScsiDevice };

class CDplayer{

	public:

		class	CDtrack{
			friend CDplayer;

			private:
					cdtrck_t	tracknr;
					cdindx_t	indexnr;
					int		starttime, stoptime, currenttime;
			public:
					const cdtrck_t	track() const;
					const cdindx_t	index() const;
					const int	timeElapsed() const;
					const int	timeRemaining() const;
			protected:
					void		setTrack(const cdtrck_t track, const cdindx_t index);
					void		setCurrenttime(const int min, const int sec);
					void		setTrackTimes(const int start, const int stop);
		};

					CDplayer();
					~CDplayer();

		const bool		ready();
		const int		open();
		void			close();

		void			getPlayingTrack(CDplayer::CDtrack & track);
		void			play(const cdtrck_t track0, const cdindx_t play_index0,
					     const cdtrck_t track1, const cdindx_t play_index1,
					     const int start_time_offset);
		void			play(const cdtrck_t track0 = 1, const cdtrck_t track1 = 0);
		void			play(const cdtrck_t track0 = 1, const cdtrck_t track1 = 0,
					     const int start_time_offset = 0);
		void			stop();
		void			pause();
		void			eject();

		const bool		isAudioTrack(const cdtrck_t c_track);
		const bool		hasCDrom();
		const int		audio_status();
		void			setDevice(const char * deviceName);
		const cdtrck_t		firstTrack(){ return track0;}
		const cdtrck_t		lastTrack(){ return track1;}

		const discid_t		getDiscId() const;
		void			getTrack(CDplayer::CDtrack & track, const cdtrck_t tracknr);
		const eDeviceType	getCurrentDeviceType();

	private:
		int			defaultDeviceUseage, cdrom_fd;
		cdtrck_t		track0, track1;
		cdindx_t		index0, index1;
		int			total_time;

		int			startTimes[100];
		int			startFrames[100];

		discid_t		discId;

		static const char *	defaultDeviceName;
		char *			currentDeviceName;

		const int		open(const char * deviceName);
		void			error(const char * indication, 
					      char * file, int line, char * func);
		void			readSubChannel(struct cdrom_subchnl & subchnl);
		void			readTocEntry(const cdtrck_t track, const cdfmt_t format, 
						     struct cdrom_tocentry & tocentry);
		void			readTimes();
		int			cddb_sum(int trackTime);
		void			calculateDiscId();
};

#endif

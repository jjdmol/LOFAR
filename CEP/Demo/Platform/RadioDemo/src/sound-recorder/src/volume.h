// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _volume_h
#define _volume_h

typedef int volume_t;

class Volume {

	public:
				Volume();
				~Volume();

				Volume(const Volume & vol);
		Volume &	operator= (const Volume & vol);

		const int	getVolumeLeft() const;
		const int	getVolumeRight() const;
		const volume_t	getVolume() const;

		void		setVolumeLeft(const int vol);
		void		setVolumeRight(const int vol);
		void		setVolume(const volume_t vol);
	private:
		volume_t	volume;
};

#endif

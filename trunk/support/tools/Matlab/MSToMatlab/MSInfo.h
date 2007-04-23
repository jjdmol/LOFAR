#ifndef MSInfo_H
#define MSInfo_H

#include <string>

class MSInfo
{
	public:
		MSInfo();
		~MSInfo();
		
		std::string sourceMS;
		int nAntennae;
		int nSamples;
		int nChannels;
		int nFrequencies; 
		int nPolarizations;

		int nNumTimeSlots;
		
		bool validInfo;
};

#endif

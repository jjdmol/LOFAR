// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "rcfile.h"

RCfile::RCfile(const char * fileName, const bool askNew)
{
	file = 0;
	fName = 0;
	isNewCreated = false;
	open(fileName, askNew, true);
}

const bool RCfile::open(const char * fileName, const bool askNew, const bool quiet)
{
	char choice[10];

	file = 0;
	isNewCreated = false;

	if(fName != 0){
		free(fName);
		fName = 0;
	}

	if(fileName){
		fName = strdup(fileName);
		file = new fstream(fileName, ios::in | ios::out | ios::nocreate | ios::skipws);
		if(!file || !file->is_open()){
			if(quiet == false)
				cerr << "File " << fileName << " not found" << endl;

			if(askNew){
				cout << "Create a new file? (y/n): ";
				cin >> setw(2) >> choice;

				if(choice[0] == 'y' || choice[0] == 'Y'){
					file->open(fileName, ios::in | ios::out | ios::trunc | ios::skipws);
				}
			}

			if(file->is_open()){
				isNewCreated = askNew;
			} else {
				if(quiet == false)
					cerr << "Error creating " << fileName << endl;
				file->close();
				delete file;

				file = 0;
			}
		}
	}
	return isOpen();
}

RCfile::~RCfile()
{
	close();
}

void RCfile::close()
{
	if(fName != 0){
		free(fName);
		fName = 0;
	}

	if(file != 0){
		file->close();
		delete file;
		file = 0;
	}
}

const bool RCfile::getEntry(const char label[], char buffer[], const int bufferSize)
{
	bool   success = false;
	char*  ptr = 0;
	char   value[LINEBUFFERSIZE];

	if(file && file->is_open()){
		file->seekp(0);
		while(!file->eof() && !ptr){
			file->getline(value, LINEBUFFERSIZE);

			if(value[0] && value[0] != '#' && (ptr = strstr(value, " "))){
				ptr[0] = '\0';
				if(!strcmp(label, value)){
					for(++ptr; ptr[0] && ptr[0] == ' '; ptr++);
					if((int) strlen(ptr) <= bufferSize){
						strcpy(buffer, ptr);
					} else {
						memcpy(buffer, ptr, bufferSize);
					}
					success = true;
					break;
				}
				ptr = 0;
			}
		}
	}

	return success;
}

const bool RCfile::putEntry(const char label[], const char buffer[])
{
	bool   success = false;
	bool   hasLabel = false;
	char   value[LINEBUFFERSIZE];
	char** lines = 0;
	int    lineCount = 0;
	int    labelLength = strlen(label);

	if(file && file->is_open()){
		file->seekp(0);

		while(!file->eof()){
			file->getline(value, sizeof(value));

			lines = (char **) realloc(lines, ++lineCount * sizeof(char *));
			lines[lineCount-1] = strdup(value);
		}

		file->close();
		file->open(fName, ios::in | ios::out | ios::trunc | ios::skipws);

		for(int i = 0; i < lineCount - 1; i++){
			if(!strncmp(lines[i], label, labelLength) &&
					strlen(lines[i]) > (u_int32_t) labelLength &&
					lines[i][labelLength] == ' ' ){

				*file << label << " " << buffer << endl;
				hasLabel = true;
			} else {
				*file << lines[i] << endl;
			}

			free(lines[i]);
		}

		if(lineCount)
			free(lines);

		if(!hasLabel)
			*file << label << " " << buffer << endl;
	}

	return success;
}

const bool RCfile::isNewlyCreated() const
{
	return isNewCreated;
}

const bool RCfile::isOpen() const
{
	return (file != 0 && file->is_open()) ? true : false;
}

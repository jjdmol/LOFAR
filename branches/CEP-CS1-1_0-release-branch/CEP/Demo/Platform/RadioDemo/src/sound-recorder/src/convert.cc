#include "waveriff0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)

int main(int argc, char ** argv)
{
	const bufferSize = 64000;
	char  buffer[bufferSize];
	int   len;

	TWave out, in;
	in.open("/home/bartw/etc/wav/guru.wav");

	out.create("/tmp/test1.wav", 17, 2, 5, 44100);
	while((len = in.read(buffer, bufferSize)) > 0){
		out.write(buffer, len);
	}

	return 0;
}

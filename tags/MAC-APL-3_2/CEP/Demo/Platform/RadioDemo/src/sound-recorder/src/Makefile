
# $Id$

include ../Makefile.Rules

RECORD_OBJ = record.o cd-player.o waveriff.o pcm.o dsp.o cdr.o \
	dspsetting.o rcfile.o soundrec.o mixer.o errorcodes.o \
	channel.o volume.o codec_pcm.o codec_msadpcm.o codec_imaadpcm.o \
	audio_spec.o
PLAY_S_OBJ = play-sample.o waveriff.o pcm.o dsp.o dspsetting.o rcfile.o \
	soundrec.o codec_pcm.o codec_msadpcm.o codec_imaadpcm.o cdr.o \
	audio_spec.o
CONVERT = convert.o waveriff.o codec_imaadpcm.o codec_msadpcm.o codec_pcm.o

all:	sound-recorder play-sample

sound-recorder: $(RECORD_OBJ)
	$(GPP) $(FLAGS) $(RECORD_OBJ) -o $@ $(LIBS)
	test -f cdsound-recorder || ln -s sound-recorder cdsound-recorder

play-sample: $(PLAY_S_OBJ)
	$(GPP) $(FLAGS) $(PLAY_S_OBJ) -o $@ $(LIBS)

convert: $(CONVERT)
	$(GPP) $(FLAGS) $(CONVERT) -o $@ $(LIBS)

%.o: %.cc %.h
	$(GPP) $(FLAGS) -c $<

clean:
	rm -f sound-recorder play-sample *.o
	test ! -L cdsound-recorder || rm cdsound-recorder

install: sound-recorder play-sample
	$(INSTALL) -s -m 755 -g root -o root sound-recorder $(BIN)
	$(INSTALL) -s -m 755 -g root -o root play-sample $(BIN)
	ln -s sound-recorder $(BIN)/cdsound-recorder

.depends dep:
	rm -f .depends
	$(GPP) -E -MM *.cc > .depends
	$(MAKE) $*

dist:
	@echo "Nothing to be done for dist"

include .depends

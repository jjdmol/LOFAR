//# protconvert.c: Fills in header bytes in framework protocols
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#

    /**
    * @code
    * Signal format 
    *
    * 2 most significant bits indicate direction of signal:
    *   F_IN    = 0b01
    *   F_OUT   = 0b10
    *   F_INOUT = 0b11 (F_IN_SIGNAL | F_OUT_SIGNAL)
    *
    * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    * | O | I | P | P | P | P | P | S | S | S | S | S | S | S | S | S |
    * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    *  15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
    * <- I/O-><--- protocol ---------><--------- signal -------------->
    * @endcode
    */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GCF/TM/GCF_Protocols.h>

#define VERSION "00.2.3"
#define OUTFILE_EXT ".tmp"

using namespace LOFAR::GCF::TM;

FILE    *pcReadLofarProtFile;   /* filepointer voor inputfile */
FILE    *pcOutputFile;          /* filepointer voor outputfile */

/* prototypes */

FILE *OpenFile(char *pcFileName, char ucRW);
void  CloseFile(FILE *pcFile);
void  ReadInputFile(FILE *pcFile);
void  DoConversion(unsigned int uiProtId);
char  TestForDirection(char *pcStartOfLine);
unsigned int SearchIdAndCopyLines(unsigned int uiProtId);
void  PrintMessageHeader(char* cInternalLine, unsigned int uiSignalDirection, unsigned int uiProtocolId, int iEventsCounted);

int main( int argc, char *argv[] )
{
  char pcFileName[500];
  char *pcSearch = NULL;
  unsigned int uiProtId = 0x00;

  if(argc == 3)
  {
    fprintf(stderr, "\nLofar test script engine protocol converter, version %s.\n", VERSION );
    strcpy (pcFileName, argv[1]);
    pcSearch = strstr(pcFileName, ".prot");
    if (pcSearch != NULL)
    {
      *pcSearch = '\0';
      strcat(pcFileName, OUTFILE_EXT); /* Change the file extension */ 
      
      pcReadLofarProtFile = OpenFile(argv[1], 0x10);
      if ( pcReadLofarProtFile != NULL )
      {
        pcOutputFile = OpenFile(pcFileName, 0x01); /* Outputfilename */
        uiProtId = SearchIdAndCopyLines(atoi(argv[2])); /* Complile the protocol base Id */
        DoConversion(uiProtId);
        CloseFile(pcReadLofarProtFile);
        CloseFile(pcOutputFile);
      }
    }
    else
    {
      fprintf(stderr, "\nError!: Arg(1): Input file is not of .prot type?\n");
      fprintf(stderr, "        : Arg(2): Base protocol Id (decimal) is not applied?\n");
    }
  }
  else
  {
    fprintf(stderr, "\nError!: Wrong parameter use.\n");
    fprintf(stderr, "\nUsage: converter  inputfile.prot   base_protocol_Id (decimal)\n");
    exit(-1);
  }
  return 0;
}

void DoConversion(unsigned int uiProtId)
{
  char  *pcFinder = NULL;
  char  cInputLine[0xFF];
  char  cInternalLine[0x60];
  int   iEventsCounted = 0x00;
  unsigned int uiSignalDirection = 0x00;
  unsigned int uiSignalHeader = 0x00;
  unsigned int uiProtocolId = 0x00;
  char  cEndOfEvent = 0x00;

  uiProtocolId = uiProtId << 8; /* Shift protocol ID to its final bitspace*/
  
  while( fgets(cInputLine, sizeof(cInputLine), pcReadLofarProtFile) != NULL )
  {
    cEndOfEvent = 0x00; /* Exit condition */
    /* just copy the file */
    if (strncmp(cInputLine, "event", 5) == 0 )
    {
      // Nu zoeken naar de string "dir"  -> switch IN/OUT/INOUT
      fputs(cInputLine, pcOutputFile);
      iEventsCounted++;
      uiSignalDirection = 0x00;
      while(( fgets(cInputLine, sizeof(cInputLine), pcReadLofarProtFile) != NULL ) && ( cEndOfEvent == 0x00))
      {
        pcFinder = &cInputLine[0];
        uiSignalHeader = 0x00;
        
        while ((isspace(*pcFinder) != 0) && (*pcFinder != '\n'))  /* remove the whitespaces before a string compare */
        {
          pcFinder++;
        }
        
        switch (TestForDirection (pcFinder))
        {
          case 0x01 : 
          {
            *pcFinder = '\0';
            uiSignalDirection |= F_IN;
            PrintMessageHeader(cInternalLine,uiSignalDirection,uiProtocolId,iEventsCounted);
            strcat(cInputLine, cInternalLine); 
            break;
          }
          case 0x02 :
          {
            *pcFinder = '\0';
            uiSignalDirection |= F_OUT;
            PrintMessageHeader(cInternalLine,uiSignalDirection,uiProtocolId,iEventsCounted);
            strcat(cInputLine, cInternalLine); 
            break;
          }
          case 0x03 :
          {
            *pcFinder = '\0';
            uiSignalDirection |= F_INOUT;
            PrintMessageHeader(cInternalLine,uiSignalDirection,uiProtocolId,iEventsCounted);
            strcat(cInputLine, cInternalLine); 
            break;
          }
          default   : 
          { 
            if (strncmp(cInputLine, "};", 2) == 0 ) /* end of event */
            {
              cEndOfEvent = 0x01;
            } 
            break;
          }
        }   
        fputs(cInputLine, pcOutputFile); /* Write something back */
      }      
    }
    if (cEndOfEvent <= 0x00)
    { 
      fputs(cInputLine, pcOutputFile); /* sometimes this went wrong, so conditional*/ 
    }
  }
  fprintf(stderr, "\nFound %d events\n", iEventsCounted);
}


unsigned int SearchIdAndCopyLines(unsigned int uiProtId)
{
  char  *pcFinder = NULL;
  char  cInputLine[0xFF];
  unsigned int uiBaseId = 0x00;
  unsigned char ucMultFact = 1;
  unsigned char ucReady = 0x00;

  while( (fgets(cInputLine, sizeof(cInputLine), pcReadLofarProtFile) != NULL) && (ucReady == 0) )
  {
    if (strncmp(cInputLine, "id = ", 5) == 0 )
    {
      // Nu zoeken naar de offset
      pcFinder = &cInputLine[0];
      
      while ((*pcFinder != '+') && (*pcFinder != '\n'))  /* remove the whitespaces before a string compare */
      {
        pcFinder++;
      }
      pcFinder++;
      while ( isalnum (*pcFinder) != 0 )
      {
        uiBaseId *= ucMultFact;
        uiBaseId += (*pcFinder - 0x30);
        pcFinder++; 
        ucMultFact *=10;
      }
      uiBaseId += uiProtId;
      fprintf(stderr, "Protocol Id = %d\n", uiBaseId);
      ucReady++;
    } 
    fputs(cInputLine, pcOutputFile);
  }
  return uiBaseId;
}


char TestForDirection(char *pcStartOfLine)
{
  if (strncmp(pcStartOfLine, "dir = IN;", 9) == 0)
  {
    fprintf(stderr, "Found a dir = IN\n");
    return 1;
  }
  else
  {
    if (strncmp(pcStartOfLine, "dir = OUT;", 10) == 0)
    {
      fprintf(stderr, "Found a dir = OUT\n");
      return 2;
    }
    else
    {
      if (strncmp(pcStartOfLine, "dir = INOUT;", 12) == 0)
      {
        fprintf(stderr, "Found a dir = INOUT\n");
        return 3;
      }
    }
  }
  return 0;
}

FILE *OpenFile(char *pcFileName, char ucRW)
{
  FILE *pcFile;
  switch (ucRW)
  {
    case 0x01:
    {
      pcFile = fopen(pcFileName, "w");
      break;
    }
    case 0x10:
    {
      pcFile = fopen(pcFileName, "r");
      break;
    }
    case 0x11:
    {
      pcFile = fopen(pcFileName, "w+");
      break;
    }
    default :
    {
      pcFile = NULL;
    }
  }
  
  if( pcFile != (FILE *)NULL)
  {
    return pcFile;
  }
  else
  {
    fprintf(stderr, "Error!: Cannot open file: %s\n", pcFileName);
    return NULL;
  }
}

void CloseFile(FILE *pcFile)
{
  fclose(pcFile);
}

void PrintMessageHeader(char* cInternalLine, unsigned int uiSignalDirection, unsigned int uiProtocolId, int iEventsCounted)
{
  uiSignalDirection <<= 14;
  unsigned int uiSignalHeader = uiSignalDirection | uiProtocolId | iEventsCounted;
  sprintf (cInternalLine, "SigNr = 0x%02X%02X;\n", (uiSignalHeader&0xFF),(uiSignalHeader>>8));
}

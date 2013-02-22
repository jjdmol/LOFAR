#ifndef _INI_PARSER_H_
#define _INI_PARSER_H_

struct tIniSettings
{
  /* Looping:     enabled = 1, disabled = 0 (default)*/
  int16 iLoop;
  /* StopOnError: enabled = 1(default), disabled = 0 */
  int16 iStopOnError;
  /* LogView:     enabled = 1, disabled = 0 (default) */
  int16 iLogView;
  /* LogLine:     enabled = 1, disabled = 0 (default)*/
  int16 iLogLine;
  /* LogToFile:   enabled = 1, disabled = 0 (default)*/
  int16 iLogToFile;
  /* Replay:      enabled = 1, disabled = 0 (default)*/
  int16 iReplay;
} ;

extern struct tIniSettings ParsedIniSettings;

/* ParseIniFile                                                             */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Filename - filename of the ini file                                     */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/*  Fills ParsedIniSettings with the values read from the ini file          */
/* ------------------------------------------------------------------------ */
extern int16 ParseIniFile( char *pcFilename );

#endif /* _INI_PARSER_H_ */

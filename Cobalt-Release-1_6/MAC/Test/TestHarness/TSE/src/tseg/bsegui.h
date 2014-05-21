/*$T BSEG/bsegui.h GC 1.131 11/08/01 15:04:28 */
/* 2001-11-15 EPKRUPE Updated after code review 2001-11-09 */
#ifndef _BSEGUI_H_
#define _BSEGUI_H_

#ifdef __cplusplus
extern "C"
{
#endif
    /* BSEG_BatchStopped will be called when a batch file is stopped.           */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* --                                                                       */
    /* Output parameters:                                                       */
    /* pReason                 : ptr to a string with the reason for stopping   */
    /* Return value:                                                            */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* The function only reports errors.                                        */
    /* ------------------------------------------------------------------------ */
    void    BSEG_BatchStopped(char* pcReason);

    /* BSEG_CheckUnselected not used                                            */
    /* ------------------------------------------------------------------------ */
    /* Input parameters : Ready                                                 */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value     : None                                                  */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* returns -1 to be compatible                                              */
    /* ------------------------------------------------------------------------ */
    extern int     BSEG_CheckUnselected(char* pcFileName);

    /* BSEG_ClearLogging will delete the current content of the log window.     */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* --																	                                    	*/
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* --																		                                    */
    /* Remarks:                                                                 */
    /* This function is not used in the CLI version.                            */
    /* ------------------------------------------------------------------------ */
    extern void    BSEG_ClearLogging(void);

    /* BSEG_GetDirectoryName                                                    */
    /* ------------------------------------------------------------------------ */
    /* Input parameters : None                                                  */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value     : char*                                                 */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* returns the current working directory                                    */
    /* ------------------------------------------------------------------------ */
     extern char*   BSEG_GetDirectoryName();

    /* BSEG_GetIncludeHeader used to allow the user to disable the inclusion of */
    /* an include header                                                        */
    /* ------------------------------------------------------------------------ */
    /* Input parameters : None                                                  */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value     : int                                                   */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* returns (1==1) to always include the header. Maybe extended in the       */
    /* future with an option in the .ini file                                   */
    /* ------------------------------------------------------------------------ */
    extern int     BSEG_GetIncludeHeader(void);

    /* BSEG_LogLine will log the string to a log window.                        */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* pLogLine                : ptr to string which will be added to the text  */
    /*                           in the log window. After logging, the GUI will */
    /*                           free the memory block.                         */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* This function is not used in the CLI version.                            */
    /* ------------------------------------------------------------------------ */
    extern void    BSEG_LogLine(char* pcLogLine);

    /* BSEG_Loop will be called when the scripts ends to determine if the script*/
    /* or batch has to be looped                                                */
    /* ------------------------------------------------------------------------ */
    /* Input parameters : None                                                  */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value     : None                                                  */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* This function restarts the batch or test script.                         */
    /* ------------------------------------------------------------------------ */
    extern void    BSEG_Loop();

    /* BSEG_Overview will view the string in the status window, if visible.     */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* pOverview               : ptr to string containing an overview of the    */
    /*                           running state machines and test script.        */
    /*                           The GUI will be responsible for freeing the    */
    /*                           memory block.                                  */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* This function is not used in the CLI version.                            */
    /* ------------------------------------------------------------------------ */
    extern void    BSEG_Overview(char* pcOverview);

    /* BSEG_ScriptStopped will be called when a single script is stopped.       */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* --                                                                       */
    /* Output parameters:                                                       */
    /* pReason                 : ptr to a string telling the reason for stopping*/
    /* Return value:                                                            */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* The function only reports errors.                                        */
    /* ------------------------------------------------------------------------ */
    extern void    BSEG_ScriptStopped(char* pcReason);

    /* BSEG_SetBatchStateImages not used                                        */
    /* ------------------------------------------------------------------------ */
    /* Input parameters :                                                       */
    /*    Filename, status - combinatie to identify the node of which the image */
    /*     is updated.                                                          */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value     : None                                                  */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* Does not do anything                                                     */
    /* ------------------------------------------------------------------------ */
    extern void    BSEG_SetBatchStateImages( char *pcFileName, char* pcStatus);

    /* BSEG_UpdateProgressBar not used                                          */
    /* ------------------------------------------------------------------------ */
    /* Input parameters : Ready                                                 */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value     : None                                                  */
    /* --                                                                       */
    /* Remarks:                                                                 */
    /* returns -1 to be compatible                                              */
    /* ------------------------------------------------------------------------ */
    extern int     BSEG_UpdateProgressBar(int bReady);

#ifdef __cplusplus
}
#endif

#endif /* _BSEGUI_H_ */

/* Declare some functionality common to the AU processing */
/* programs (e.g. "as_proc1h", "as_proc13c", etc.).       */


/*********************************************************/
/* Function: GetPdfFilePath                              */
/* Desc:     Determine where pdf files for this data     */
/*           should be written to.                       */
/* Created:  John Hollerton, Sept-2006                   */
/* History:  Phil Branton, 20-Oct-2006                   */
/*           Added extra error handling to the POC code. */
/*********************************************************/
void GetPdfFilePath( char* pdf_Name, char* szFileStatus )
{
  /* Declare some constants and variables */
 /* const char PDF_ROOT[] = "//stvdsntp001/dis-elnb-printpdf/stvnmr/";  */
const char PDF_ROOT[] = "d:/disk2/data/olympiad/";
  const char NO_NETWORK_PDF_ROOT[] = "C:/pdffiles/";
  const char AWN_UPDATE_FILE[] = "awmup.txt";

  int i;
  char szLogFileMsg[1000];
  char pdf_Path[PATH_MAX];
  char awm_File[PATH_MAX];
  char title_File[PATH_MAX];
  FILE *fpAwmFile;
  FILE *fpTitleFile;
  char oi[80];
  char juser[80];
  char title1[80];

  /* The AWM update file (awmup.txt) contains necessary    */
  /* information for creating the pdf directory structure. */

  /* Get the path to the awmup.txt file for this experiment */
  sprintf( awm_File, "%s/data/%s/nmr/%s/%d/%s", disk, user, name, expno, AWN_UPDATE_FILE );
     
  /* Check the program has read access to the awmup.txt file */
  if( access( awm_File, R_OK ) == 0 )
  {
    /* Open the file for read access */
    fpAwmFile = fopen( awm_File, "r" );

    /* Read the oi from the AWN update file */
    fgets( oi, sizeof(oi), fpAwmFile );

    /* Check that the oi exists (and not just a blank line or eof) */
    if( strlen( oi ) > 2 )
    {
      /* Remove trailing CR LF */
      oi[strlen(oi)-2] = NULL;
    }
    else
    {
      /* Update the errors log file */
      sprintf( szLogFileMsg, "LNB not found in \"%s\" - Using \"LNB_Unknown\"", awm_File );
      WriteToErrorLogFile( szLogFileMsg );
      strcpy( oi, "LNB_Unknown" );
    }

    /* Windows filenames cannot contain any of the following characters: */
    /* \/:*?"<>                                                          */

    /* Replace these characters with '_' so that we don't break the      */
    /* directory structure.                                              */
    for( i = 0; i < strlen(oi); i++ )
    {
      if( oi[i] == '\\' || oi[i] == '/' || oi[i] == ':' || oi[i] == '*' ||
          oi[i] == '?' || oi[i] == '\"' || oi[i] == '<' || oi[i] == '>' )
      {
        oi[i] = '_';
      }
    }

    /* Read the userid from the AWN update file */
    fgets( juser, sizeof(juser), fpAwmFile );

    /* Check that the userid exists (and not just a blank line or eof) */
    if( strlen( juser ) > 2 )
    {
      /* Remove trailing CR LF */
      juser[strlen(juser)-2] = NULL;
    }
    else
    {
      /* Update the errors log file */
      sprintf( szLogFileMsg, "UserID not found in \"%s\" - Using \"User_Unknown\"", awm_File );
      WriteToErrorLogFile( szLogFileMsg );
      strcpy( juser, "User_Unknown" );
    }

    /* Replace invalid characters with '_' so that we don't break the */
    /* directory structure (also replace any spaces in the userid).   */
    for( i = 0; i < strlen(juser); i++ )
    {
      if( juser[i] == '\\' || juser[i] == '/' || juser[i] == ':' || juser[i] == '*' ||
          juser[i] == '?' || juser[i] == '\"' || juser[i] == '<' || juser[i] == '>' ||
          juser[i] == ' ' )
      {
        juser[i] = '_';
      }
    }

    /* Finished reading from the update file */
    fclose( fpAwmFile );

  }
  /* Else unable to open AWM update file */
  else
  {
    /* Update the errors log file */
    sprintf( szLogFileMsg, "Unable to open \"%s\"", awm_File );
    WriteToErrorLogFile( szLogFileMsg );
    /* Try to open the title file and get information from that  */
   sprintf( title_File, "%s/data/%s/nmr/%s/%d/pdata/%d/title", disk, user, name, expno, procno );
   if( access( title_File,  R_OK ) == 0 )
   {
    fpTitleFile = fopen( title_File, "r" );
     
      fgets( title1, sizeof(title1), fpTitleFile );
      
      fgets( oi, sizeof(oi), fpTitleFile );
      if( strlen( oi ) > 2)
      {
        oi[strlen(oi)-1] = NULL;
      }
      else
    {
      /* Update the errors log file */
      sprintf( szLogFileMsg, "LNB not found in \"%s\" - Using \"LNB_Unknown\"", title_File );
      WriteToErrorLogFile( szLogFileMsg );
      strcpy( oi, "LNB_Unknown" );
    }

    /* Windows filenames cannot contain any of the following characters: */
    /* \/:*?"<>                                                          */

    /* Replace these characters with '_' so that we don't break the      */
    /* directory structure.                                              */
    for( i = 0; i < strlen(oi); i++ )
    {
      if( oi[i] == '\\' || oi[i] == '/' || oi[i] == ':' || oi[i] == '*' ||
          oi[i] == '?' || oi[i] == '\"' || oi[i] == '<' || oi[i] == '>' )
      {
        oi[i] = '_';
      }
    }
      fgets( juser, sizeof(juser), fpTitleFile );

    /* Check that the userid exists (and not just a blank line or eof) */
    if( strlen( juser ) > 2 )
    {
      /* Remove trailing CR LF */
      juser[strlen(juser)-1] = NULL;
    }
    else
    {
      /* Update the errors log file */
      sprintf( szLogFileMsg, "UserID not found in \"%s\" - Using \"User_Unknown\"", title_File );
      WriteToErrorLogFile( szLogFileMsg );
      strcpy( juser, "User_Unknown" );
    }

    /* Replace invalid characters with '_' so that we don't break the */
    /* directory structure (also replace any spaces in the userid).   */
    for( i = 0; i < strlen(juser); i++ )
    {
      if( juser[i] == '\\' || juser[i] == '/' || juser[i] == ':' || juser[i] == '*' ||
          juser[i] == '?' || juser[i] == '\"' || juser[i] == '<' || juser[i] == '>' ||
          juser[i] == ' ' )
      {
        juser[i] = '_';
      }
    }   
       fclose( fpTitleFile);     
   }
   else
   {
    /* Give OI and UserID default values */
    strcpy( oi, "LNB_Unknown" );
    strcpy( juser, "User_Unknown" );
   }
  }

  /* Find the directory path for the PDF file being created */
  sprintf( pdf_Path, "%s%s/%s/", PDF_ROOT, juser , oi );

  /* If this file path doesn't already exist, then create it */
  if( mkudir( pdf_Path ) < 0 )
  {
    /* If there was a problem creating the directory, update */
    /* the LOG file, and display an error message.           */
    /* (Network may be unavailable or may not have the       */
    /* required permissions.)                                */
    sprintf( szLogFileMsg, "Could not create \"%s\"", pdf_Path );
    WriteToErrorLogFile( szLogFileMsg );
    Proc_err( DEF_ERR_OPT, "Could not create :\n%s", pdf_Path );
    sleep( 5 );

    /* Couldn't create network dir so store the pdf locally */
    sprintf( pdf_Path, "%s%s/%s/", NO_NETWORK_PDF_ROOT, juser, oi );
    mkudir( pdf_Path );
  }

  /* Update the pdf_Name variable with the full path of the PDF */
  /* file to be created (so that the AU program making use of   */
  /* this subroutine knows where to create the PDF file).       */
  sprintf( pdf_Name, "%s%s_%d.pdf", pdf_Path, name, expno );

  /* Check whether this file currently exists */
  if( access( pdf_Name, F_OK ) == 0 )
  {
    /* If it does, check whether it is read-only */
    if( access( pdf_Name, W_OK ) == 0 )
    {
      strcpy( szFileStatus, "Exists" );
    }
    else
    {
      strcpy( szFileStatus, "ReadOnly" );
    }
  }
  else
  {
    strcpy( szFileStatus, "New" );
  }

}



/*********************************************************/
/* Function: UpdateWorkLog                               */
/* Desc:     Updates the work log file after the pdf     */
/*           file should have been created.              */
/* Created:  Phil Branton, 25-Oct-2006                   */
/*********************************************************/
void UpdateWorkLog( char* szTypeOfData, char* pdf_Name, char* szFileStatus )
{
  char szLogFileMsg[1000];

  /* Check for the existence of the pdf file that should have been created */
  if( access( pdf_Name, F_OK ) == 0 )
  {
    if( strcmp( szFileStatus, "ReadOnly" ) == 0 )
    {
      sprintf( szLogFileMsg, "%s: Encountered read-only file \"%s\"", szTypeOfData, pdf_Name );
    }
    else if( strcmp( szFileStatus, "Exists" ) == 0 )
    {
      sprintf( szLogFileMsg, "%s: Updated \"%s\"", szTypeOfData, pdf_Name );
    }
    else
    {
      sprintf( szLogFileMsg, "%s: Created \"%s\"", szTypeOfData, pdf_Name );
    }
    /* Update work log file */
    WriteToWorkLogFile( szLogFileMsg );
  }
  else
  {
    /* If pdf file hasn't been created, write to errors log file */
    sprintf( szLogFileMsg, "%s: Failed to create \"%s\"", szTypeOfData, pdf_Name );
    WriteToErrorLogFile( szLogFileMsg );
  }

}



/*********************************************************/
/* Function: WriteToErrorLogFile                         */
/* Desc:     Writes the string passed into the function  */
/*           to the file used to log errors.             */
/* Created:  Phil Branton, 20-Oct-2006                   */
/*********************************************************/
void WriteToErrorLogFile( char* szErrorMessage )
{
  /* Declare some constants and variables */
  const char ERROR_LOG_FILE[] = "C:/temp/NMR_PDF_ERRORS.txt";

  time_t currentTime;
  char szTime[200];
  FILE *fpErrorLogFile;

  /* Open the log file to write the error message to */
  fpErrorLogFile = fopen( ERROR_LOG_FILE, "a" );
  if( fpErrorLogFile == NULL )
  {
    /* Could not open file - display warning */
    Proc_err( DEF_ERR_OPT, "Could not open error log file:\n%s", ERROR_LOG_FILE );
    sleep( 5 );
    return;
  }

  /* Get the current time formatted as e.g. "Wed Jul 31, 14:32:00, 2006" */
  time( &currentTime );
  strftime( szTime, sizeof(szTime), "%a %b %d, %H:%M:%S, %Y", localtime( &currentTime ) );

  /* Write the error message then close the file */
  fprintf( fpErrorLogFile, "%s\r\n%s\r\n\r\n", szTime, szErrorMessage );
  fclose( fpErrorLogFile );

}



/*********************************************************/
/* Function: WriteToWorkLogFile                          */
/* Desc:     Writes the string passed into the function  */
/*           to the work log file.                       */
/* Created:  Phil Branton, 20-Oct-2006                   */
/*********************************************************/
void WriteToWorkLogFile( char* szMessage )
{
  /* Declare some constants and variables */
  const char WORK_LOG_FILE[] = "C:/temp/NMR_PDF_WORK_LOG.txt";

  time_t currentTime;
  char szTime[200];
  FILE *fpWorkLogFile;

  /* Open the work log file */
  fpWorkLogFile = fopen( WORK_LOG_FILE, "a" );
  if( fpWorkLogFile == NULL )
  {
    /* Could not open file - display warning */
    Proc_err( DEF_ERR_OPT, "Could not open work log file:\n%s", WORK_LOG_FILE );
    sleep( 5 );
    return;
  }

  /* Get the current time formatted as e.g. "Wed Jul 31, 14:32:00, 2006" */
  time( &currentTime );
  strftime( szTime, sizeof(szTime), "%a %b %d, %H:%M:%S, %Y", localtime( &currentTime ) );

  /* Write the message to the work log, then close the file */
  fprintf( fpWorkLogFile, "%s at \"%s\"\r\n", szMessage, szTime);
  fclose( fpWorkLogFile );

}
/*****************************************************************************/
/*****************************************************************************/
/*
Include file: as_sysutil  
Version:  1.0
Author:   Sean Lynn, GSK R&D
Description: compendium of processing utilities used by as_proc programs, 
see also "as_pdf_common.inc", used for pdf generation.  List of functions:

unlinkpr - delete process data

as_projref - set projections and reference 2d

gethighest - find highest expno

as_phasehsqc - phase hsqc

as_phaseroesy - phase roesy

as cycalc - calculate CY for 1D 1H

as_phase1d - checks APKS and does APK if required

set_sreglist - sets solvents regions file

/*****************************************************************************/
/*****************************************************************************/

/****************************************************************/
/*	as_unlinkpr		21.02.1997 ENG			*/
/****************************************************************/
/*	This function unlinks the processed data files which	*/
/*	occupy to much disk space.				*/
/*  last_plot.xwp added - S Lynn 14/11/2006 */
/****************************************************************/

int unlinkpr (basename)
char *basename;
{
  char pathname[PATH_MAX];

  (void)sprintf(pathname,"%s/1r",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/1i",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/2rr",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/2ri",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/2ir",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/2ii",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/dsp",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/dsp.hdr",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/dsp_low",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  (void)sprintf(pathname,"%s/last_plot.xwp",basename);
  (void)remove(pathname); /* No error, if this doesn't work */
  
  return (0);
}

/*****************************************************************************/
/*****************************************************************************/
/*
Program:  as_projref
Version:  1.0
Author:   Sean Lynn, GSK R&D
Description: sets projections & references 2D spectra
/*****************************************************************************/
/*****************************************************************************/
as_projref(curdat)
char *curdat;
{
  char homepul[40], pul[40], directory[40], data_File[80];
int mode, dexpno, i_expno, i_limit, Hexpno, Cexpno;
float Hsr, Csr;

Hexpno=0;
Cexpno=0;
i_expno=10;
Hsr=0.0;
Csr=0.0;

GETCURDATA
dexpno = expno;
FETCHPAR("PPARMOD", &mode)
if ( mode != 1 )
{
	Proc_err (INFO_OPT, "This is not a 2D dataset \npparmod = %d ", mode);
	sleep(2);
	exit(1);
}

FETCHPAR("PULPROG", &homepul)

(void) sprintf(directory, "%s/data/%s/nmr/%s", disk, user, name);
i_limit = gethighest(directory);

if( i_limit > 999 ) i_limit = 999;

while (i_expno < i_limit)
{
sprintf( data_File, "%s/data/%s/nmr/%s/%d/fid", disk, user, name, i_expno );
if( access( data_File, R_OK ) == 0 )
 {
 DATASET(name, i_expno, procno, disk, user)
 FETCHPAR("PULPROG", &pul)

 if  (strstr(pul, "zg30" ))
	{
	SETCURDATA
	/*XAUPW*/	/*optional process 1H, if required*/
	Hexpno = expno;
	}
  if ((strstr(pul, "zgpg30")) || (strstr(pul, "zgdc30")))
	{
	SETCURDATA
	/*XAUPW*/	/*optional process 13C, if required*/
	Cexpno = expno;
	}
 }	
i_expno++;
}

if ((strstr(homepul, "cosy")) || (strstr(homepul, "roesy"))) Cexpno = Hexpno;	/*test for homo 2D*/

if ( Cexpno != 0)
{
DATASET(name,Cexpno,procno,disk,user)
FETCHPAR("SR",&Csr)
}
DATASET(name,Hexpno,procno,disk,user)
FETCHPAR("SR",&Hsr)
DATASET(name,dexpno,procno,disk,user)
if ( Cexpno  != 0)
{
STOREPAR1("SR",Csr)
DATASET3(name,Cexpno,procno,disk,user)
}
STOREPAR("SR",Hsr)
DATASET2(name,Hexpno,procno,disk,user)
DATASET(name,dexpno,procno,disk,user)
}

/****************************************************************/
/*	gethighest		6.6.1997 ENG			*/
/****************************************************************/
/*	Returns the highest experiment number of the data set.	*/
/****************************************************************/

int gethighest(pathname)
char *pathname;
{
  char **listofexpnos=0;/* pointer to list of EXPNOS for getdir() */
  int nr=0;		/* number of existing EXPNOS */
  int ret=0;		/* existing EXPNO */
  int highexp=0;	/* return next available EXPNO */
  int iii;		/* loop counter */
  
  nr = getdir (pathname,&listofexpnos,"[1-9]*");
  if ( nr > 0 )
  {
    for ( iii = nr-1; iii >= 0; iii-- )
    {
      if ( (ret = atoi(listofexpnos[iii])) != 0 )
      {
	if  ( ret > highexp )
	  highexp = ret;
      }
    }
    highexp++;
  }
  if ( highexp == 0 )
    highexp = 1;
  if ( listofexpnos != 0 )
    freedir(listofexpnos);
  return (highexp);
}

/*****************************************************************************/
/*****************************************************************************/
/*
Program:  as_phasehsqc
Version:  1.0
Author:   Mark Vine, GSK R&D, edited by Sean Lynn oa2dinv
Description: phase hsqc
/*****************************************************************************/
/*****************************************************************************/
as_phasehsqc(curdat)
char *curdat;
{
#define MAXPEAKS		10000
#define NUMOFPPHEADERLINES	4		// header lines in peaks.txt file

char lineBuf[BUFSIZ];				// To hold lines from the peakpick file
int numOfPeaks;
FILE *peaksFp;					// Pointer to peakpicking output file
int i, j;					// Loop counters

//------------------------------------------------------------------------------
//	Identify the nuclei and solvent 
//------------------------------------------------------------------------------


char f1Nucleus[6];				// The inverse-detected nucleus
char f2Nucleus[6];				// Observed nucleus, usually H1
char solvent[BUFSIZ];

if (is_avance())				// retrieve indirectly-detected nucleus
{
    FETCHPARS("NUC1",f2Nucleus)
    FETCHPARS("NUC2",f1Nucleus)
    if (strcmp("off",f1Nucleus) == 0)		// NUC3 being used instead
        FETCHPARS("NUC3",f1Nucleus)		// should do something if this is off
}
else
{
    FETCHPARS("NUCLEUS",f2Nucleus)
    FETCHPARS("DECNUC",f1Nucleus)
    if (strcmp("off",f1Nucleus) == 0)		// channel 3 being used instead
        FETCHPARS("DECNUC2",f1Nucleus)		// is this the correct paramname ? 
}
FETCHPARS("SOLVENT",solvent)

//------------------------------------------------------------------------------
//	Get PH_mod settings
//------------------------------------------------------------------------------




int f2phmod;
int f1phmod;					// To hold PH_mod values
int xf2mFlag;

FETCHPAR("PH_mod",&f2phmod)			// Check if F2 needs phasing
FETCHPAR1("PH_mod",&f1phmod)			// Same for F1
if (f2phmod == 0 && f1phmod == 1)		// F2 = "no", F1 = "pk", do xf2m at end
{
    STOREPAR("PH_mod",1)			// Do "pk", reset at end of processing
    f2phmod = 1;
    xf2mFlag = 1;				// Do xf2m at end of processing
}
else
    xf2mFlag = 0;


//------------------------------------------------------------------------------
//	Construct an F1 projection. 
//------------------------------------------------------------------------------


char regionFile[PATH_MAX];

int origExpno;					// contains original 2D data
int origProcno;					// contains original 2D spectrum
int f1ProjExpno;
int f1ProjProcno;				// contains 1D projections
int currRowProcno;
int nextRowProcno;
int sumProcno;					// To contain sum of F2 rows for phasing
int rowNum;
int prevRowNum;
int currRow;
int rowAddress[MAXPEAKS];

float newPHC0;
float newPHC1;					// Phase parameters

origExpno = expno;
origProcno = procno;
f1ProjProcno = 1000;
    
XF2M						// Just in case phase in F1 is wrong
 
RVPP(f1ProjProcno)				// Create positive F1 projection
DATASET(name,origExpno,f1ProjProcno,disk,user)
    
//ABS						// Create intrgn file

STOREPAR("CY",100.0)
STOREPAR("MAXI",101.0)				// bugfix, pp only picks < MAXI, not <=
STOREPAR("MI",5.0)
STOREPAR("PC",2.00)
strcpy(regionFile,f1Nucleus);			// reconstruct correct region filename
strcat(regionFile,".");
strcat(regionFile,solvent);
STOREPAR("SREGLST",regionFile)			// update param with correct name 
    
XWP_PP						// peakpick into peak.txt


//------------------------------------------------------------------------------
//	Autophase the peaks in the F2 dimension, if requested
//------------------------------------------------------------------------------

char currRowProcnoPath[PATH_MAX];
char nextRowProcnoPath[PATH_MAX];
char sumProcnoPath[PATH_MAX];
int  i1; 

if (f2phmod == 1)				// PH_mod = "pk"
{
    numOfPeaks = 0;
    if ((peaksFp = fopen(PROCPATH("peak.txt"),"r")) != NULL)
    {
        for (i = 0; i < NUMOFPPHEADERLINES; i++)	// Skip the file header
    	    fgets(lineBuf,sizeof(lineBuf),peaksFp);
    
        j = 0;
        while ((fgets(lineBuf,sizeof(lineBuf),peaksFp) != NULL) && (j < MAXPEAKS))
    	    sscanf(lineBuf,"%d %d",&numOfPeaks,&rowAddress[j++]); 
        fclose(peaksFp);
    
        if (numOfPeaks > 0)			// Have peaks, can phase-correct F2
        {
    	    currRow = 0;
    	    currRowProcno = f1ProjProcno + 1;
    	    nextRowProcno = currRowProcno + 1;
    	    sumProcno = nextRowProcno + 1;
    
    	    DATASET(name,origExpno,origProcno,disk,user)
    	    XFB					// Remove effect of xf2m
    
    	    rowNum = (int)(rowAddress[currRow] + 0.5);
    	    RSR(rowNum,currRowProcno)		
    	    RSR(rowNum,nextRowProcno)	
    	    RSR(rowNum,sumProcno)		// In case we have only one peak
    
    	    DATASET(name,origExpno,sumProcno,disk,user)
    	    DATASET2(name,origExpno,currRowProcno,disk,user)
    	    DATASET3(name,origExpno,nextRowProcno,disk,user)
    	    STOREPAR("DATMOD",1)
    	    STOREPAR("DC",1.0)
    
    	    prevRowNum = rowNum;
    	    while (++currRow < j)		// Have some more peaks
    	    {
    	        rowNum = (int)(rowAddress[currRow] + 0.5);
    	        if (rowNum > prevRowNum)	// don't add the same row twice
    	        {
    	            WRPA(name,origExpno,currRowProcno,disk,user)
    
	            DATASET(name,origExpno,origProcno,disk,user)
	            RSR(rowNum,nextRowProcno)
	            DATASET(name,origExpno,sumProcno,disk,user)
	            ADD
	            prevRowNum = rowNum;
	            }
	        }
	    APK
	    FETCHPAR("PHC0",&newPHC0)
	    FETCHPAR("PHC1",&newPHC1)

	    
	    strcpy(sumProcnoPath, PROCPATH(".."));
      sprintf( currRowProcnoPath, "%s/data/%s/nmr/%s/%d/pdata/%d", disk, user, name, expno, currRowProcno );
      sprintf( nextRowProcnoPath, "%s/data/%s/nmr/%s/%d/pdata/%d", disk, user, name, expno, nextRowProcno );
      sprintf( sumProcnoPath, "%s/data/%s/nmr/%s/%d/pdata/%d", disk, user, name, expno, sumProcno );
      i1 = unlinkpr( currRowProcnoPath );		// delete the currRowProcno directory
      i1 = unlinkpr( nextRowProcnoPath );		// Delete the nextRowProcno directory
      DATASET( name,origExpno,origProcno,disk,user )	// Return to the 2D spectrum
      i1 = unlinkpr( sumProcnoPath );		// now can delete the sumProcno directory
      }    
    }
}
//DATASET(name,origExpno,origProcno,disk,user)	// Return to the 2D spectrum
if (numOfPeaks > 0 && f2phmod == 1)
{
    STOREPAR("PHC0",fmod(newPHC0,360.0))	// Minimize phase-induced baseline roll
    STOREPAR("PHC1",newPHC1)
}

XFB						// reprocess with correct F2 phase
VIEWDATA_SAMEWIN


//------------------------------------------------------------------------------
//	Construct an F2 projection. For edited spectra, invert negative peaks
//------------------------------------------------------------------------------


int f2ProjProcno;
int posProcno, negProcno;
int currColProcno;
int nextColProcno;
int colNum;
int prevColNum;
int currCol;
int colAddress[MAXPEAKS];

f2ProjProcno = 2000;
posProcno = f2ProjProcno - 2;
negProcno = f2ProjProcno - 1;

RHPP(f2ProjProcno)				// just to create correct parameters
RHPP(posProcno)					// Create positive F1 projection
RHNP(negProcno)					// Create negative F1 projection

DATASET(name,origExpno,f2ProjProcno,disk,user)
DATASET2(name,origExpno,posProcno,disk,user)
DATASET3(name,origExpno,negProcno,disk,user)			
STOREPAR("DATMOD",1)
STOREPAR("DC",-1.0)				// invert the negative peaks
ADD						// Create spectrum with all peaks
    
//ABS						// Create the intrgn file

STOREPAR("CY",100.0)
STOREPAR("MAXI",101.0)
STOREPAR("MI", 10.0)
STOREPAR("PC",2.0)
STOREPAR("PSIGN",0)				// pos peaks only, if DC = -1 
STOREPAR("PSCAL",1)				// preg
strcpy(regionFile,f2Nucleus);			// reconstruct correct region filename
strcat(regionFile,".");
strcat(regionFile,solvent);
STOREPAR("SREGLST",regionFile)			// update param with correct name 
    
XWP_PP

//------------------------------------------------------------------------------
//	Autophase the peaks in the F1 dimension, if requested
//------------------------------------------------------------------------------


if (f1phmod == 1)				// PH_mod = "pk"
{
    numOfPeaks = 0;
    if ((peaksFp = fopen(PROCPATH("peak.txt"),"r")) != NULL)
    {
        for (i = 0; i < NUMOFPPHEADERLINES; i++)	// Skip the file header
    	    fgets(lineBuf,sizeof(lineBuf),peaksFp);
    
        j = 0;
        while ((fgets(lineBuf,sizeof(lineBuf),peaksFp) != NULL) && (j < MAXPEAKS))
    	    sscanf(lineBuf,"%d %d",&numOfPeaks,&colAddress[j++]); 
        fclose(peaksFp);
    
        if (numOfPeaks > 0)			// Have peaks, can phase-correct F1
        {
    	    currCol = 0;
    	    currColProcno = f2ProjProcno + 1;
    	    nextColProcno = currColProcno + 1;
    	    sumProcno = nextColProcno + 1;
    
    	    DATASET(name,origExpno,origProcno,disk,user)
    	    XFB
    
    	    colNum = (int)(colAddress[currCol] + 0.5);
    	    RSC(colNum,currColProcno)		// This creates the new procnos 
    	    RSC(colNum,nextColProcno)	
    	    RSC(colNum,sumProcno)		// In case we have only one peak
    
    	    DATASET(name,origExpno,sumProcno,disk,user)
    	    DATASET2(name,origExpno,currColProcno,disk,user)
    	    DATASET3(name,origExpno,nextColProcno,disk,user)
    	    STOREPAR("DATMOD",1)
    	    STOREPAR("DC",1.0)
    
    	    prevColNum = colNum;
    	    while (++currCol < j)		// Have some more peaks
    	    {
    	        colNum = (int)(colAddress[currCol] + 0.5);
    	        if (colNum > prevColNum)
    	        {
    	            WRPA(name,origExpno,currColProcno,disk,user)
    
    	            DATASET(name,origExpno,origProcno,disk,user)
    	            RSC(colNum,nextColProcno)
    	            DATASET(name,origExpno,sumProcno,disk,user)
    	            ADD
    		    prevColNum = colNum;
    	        }
	    }
	    APK					// zero-order should be sufficient
	    ABS					// regions maybe needed for setting F1P, F2P
	    FETCHPAR("PHC0",&newPHC0)		
	    FETCHPAR("PHC1",&newPHC1)
        }    
    }
}
DATASET(name,origExpno,origProcno,disk,user)	// Return to the 2D spectrum
if (numOfPeaks > 0 && f1phmod == 1)
{
    STOREPAR1("PHC0",fmod(newPHC0,360.0))	// Minimize phase-induced baseline roll
    STOREPAR1("PHC1",newPHC1)
}

//------------------------------------------------------------------------------
//	Reprocess the 2D spectrum, now with the correct phase
//------------------------------------------------------------------------------


XFB						// now uses corrected phases
ABS2						// baseline correct both dimensions
ABS1

if (xf2mFlag == 1)				// Botch to call xf2m if needed
{
    STOREPAR("PH_mod",0)			// Set PH_mod = "no" 
    XF2M					// Need magnitude-mode in F2
}
LEVCALC
}

/*****************************************************************************/
/*****************************************************************************/
/*
Program:  as_phaseroesy
Version:  1.0
Author:   Sean Lynn, GSK R&D
Description: phases ROESY spectra based on oa_2dinv code from Mark Vine
/*****************************************************************************/
/*****************************************************************************/
as_phaseroesy(curdat)
char *curdat;
{
#define MAXPEAKS		10000
#define NUMOFPPHEADERLINES	4		// header lines in peaks.txt file

char lineBuf[BUFSIZ];				// To hold lines from the peakpick file
int numOfPeaks;
FILE *peaksFp;					// Pointer to peakpicking output file
int i, j;					// Loop counters

//------------------------------------------------------------------------------
//	Identify the nuclei and solvent 
//------------------------------------------------------------------------------


char f1Nucleus[6];				// The inverse-detected nucleus
char f2Nucleus[6];				// Observed nucleus, usually H1
char solvent[BUFSIZ];

FETCHPARS("NUC1",f2Nucleus)
FETCHPARS("NUC2",f1Nucleus)

FETCHPARS("SOLVENT",solvent)

//------------------------------------------------------------------------------
//	Get PH_mod settings
//------------------------------------------------------------------------------




int f2phmod;
int f1phmod;					// To hold PH_mod values
int xf2mFlag;

FETCHPAR("PH_mod",&f2phmod)			// Check if F2 needs phasing
FETCHPAR1("PH_mod",&f1phmod)			// Same for F1
if (f2phmod == 0 && f1phmod == 1)		// F2 = "no", F1 = "pk", do xf2m at end
{
    STOREPAR("PH_mod",1)			// Do "pk", reset at end of processing
    f2phmod = 1;
    xf2mFlag = 1;				// Do xf2m at end of processing
}
else
    xf2mFlag = 0;


//------------------------------------------------------------------------------
//	Construct an F1 projection. 
//------------------------------------------------------------------------------


char regionFile[PATH_MAX];

int origExpno;					// contains original 2D data
int origProcno;					// contains original 2D spectrum
int f1ProjExpno;
int f1ProjProcno;				// contains 1D projections
int currRowProcno;
int nextRowProcno;
int sumProcno;					// To contain sum of F2 rows for phasing
int rowNum;
int prevRowNum;
int currRow;
int rowAddress[MAXPEAKS];

float newPHC0;
float newPHC1;					// Phase parameters

origExpno = expno;
origProcno = procno;
f1ProjProcno = 1000;
    
XF2M						// Just in case phase in F1 is wrong
 
RVPP(f1ProjProcno)				// Create positive F1 projection
DATASET(name,origExpno,f1ProjProcno,disk,user)
    
//ABS						// Create intrgn file

STOREPAR("CY",100.0)
STOREPAR("MAXI",101.0)				// bugfix, pp only picks < MAXI, not <=
STOREPAR("MI",5.0)
STOREPAR("PC",2.00)
strcpy(regionFile,f1Nucleus);			// reconstruct correct region filename
strcat(regionFile,".");
strcat(regionFile,solvent);
STOREPAR("SREGLST",regionFile)			// update param with correct name 
    
XWP_PP						// peakpick into peak.txt


//------------------------------------------------------------------------------
//	Autophase the peaks in the F2 dimension, if requested
//------------------------------------------------------------------------------

char currRowProcnoPath[PATH_MAX];
char nextRowProcnoPath[PATH_MAX];
char sumProcnoPath[PATH_MAX];
int  i1; 

if (f2phmod == 1)				// PH_mod = "pk"
{
    numOfPeaks = 0;
    if ((peaksFp = fopen(PROCPATH("peak.txt"),"r")) != NULL)
    {
        for (i = 0; i < NUMOFPPHEADERLINES; i++)	// Skip the file header
    	    fgets(lineBuf,sizeof(lineBuf),peaksFp);
    
        j = 0;
        while ((fgets(lineBuf,sizeof(lineBuf),peaksFp) != NULL) && (j < MAXPEAKS))
    	    sscanf(lineBuf,"%d %d",&numOfPeaks,&rowAddress[j++]); 
        fclose(peaksFp);
    
        if (numOfPeaks > 0)			// Have peaks, can phase-correct F2
        {
    	    currRow = 0;
    	    currRowProcno = f1ProjProcno + 1;
    	    nextRowProcno = currRowProcno + 1;
    	    sumProcno = nextRowProcno + 1;
    
    	    DATASET(name,origExpno,origProcno,disk,user)
    	    XFB					// Remove effect of xf2m
    
    	    rowNum = (int)(rowAddress[currRow] + 0.5);
    	    RSR(rowNum,currRowProcno)		
    	    RSR(rowNum,nextRowProcno)	
    	    RSR(rowNum,sumProcno)		// In case we have only one peak
    
    	    DATASET(name,origExpno,sumProcno,disk,user)
    	    DATASET2(name,origExpno,currRowProcno,disk,user)
    	    DATASET3(name,origExpno,nextRowProcno,disk,user)
    	    STOREPAR("DATMOD",1)
    	    STOREPAR("DC",1.0)
    
    	    prevRowNum = rowNum;
    	    while (++currRow < j)		// Have some more peaks
    	    {
    	        rowNum = (int)(rowAddress[currRow] + 0.5);
    	        if (rowNum > prevRowNum)	// don't add the same row twice
    	        {
    	            WRPA(name,origExpno,currRowProcno,disk,user)
    
	            DATASET(name,origExpno,origProcno,disk,user)
	            RSR(rowNum,nextRowProcno)
	            DATASET(name,origExpno,sumProcno,disk,user)
	            ADD
	            prevRowNum = rowNum;
	        }
	    }
	    APK
	    FETCHPAR("PHC0",&newPHC0)
	    FETCHPAR("PHC1",&newPHC1)

      sprintf( currRowProcnoPath, "%s/data/%s/nmr/%s/%d/pdata/%d", disk, user, name, expno, currRowProcno );
      sprintf( nextRowProcnoPath, "%s/data/%s/nmr/%s/%d/pdata/%d", disk, user, name, expno, nextRowProcno );
      sprintf( sumProcnoPath, "%s/data/%s/nmr/%s/%d/pdata/%d", disk, user, name, expno, sumProcno );
      i1 = unlinkpr( currRowProcnoPath );		// delete the currRowProcno directory
      i1 = unlinkpr( nextRowProcnoPath );		// Delete the nextRowProcno directory
      DATASET( name,origExpno,origProcno,disk,user )	// Return to the 2D spectrum
      i1 = unlinkpr( sumProcnoPath );		// now can delete the sumProcno directory
        }    
    }
}
//DATASET(name,origExpno,origProcno,disk,user)	// Return to the 2D spectrum
if (numOfPeaks > 0 && f2phmod == 1)
{
    newPHC0 = newPHC0 + 180.0;
    STOREPAR("PHC0",fmod(newPHC0,360.0))	// Minimize phase-induced baseline roll
    STOREPAR("PHC1",newPHC1)
}

XFB						// reprocess with correct F2 phase
}

/*****************************************************************************/
/*****************************************************************************/
/*
Program:  CYCalc
Version:  3.0
Author:   John Hollerton, GSK R&D
Description:
Calculates vertical scale setting (CY) for 1D proton spectra (not tested for
heteronuclei)

History

V3.0 Major change to algorithm - now ignores all singlets and scales the
multiplets to predefined value (JY).  Average and standard deviation are no
longer used.  

V2.1 Ignores solvent peaks

V2.0 Takes top N largest peaks to perform calculation

V1.1 Modifies CY to improve peak picking

V1.0 First release - calculates average peak height and then discards peaks
greater than 2 standard deviations from average. Then scales the average 
peak height of those peaks left
*/
/*****************************************************************************/
/*****************************************************************************/
as_cycalc(curdat)
char *curdat;
{
int i,j,isave,numpeaks,nregions,peakvalid,nmultiplets;
float JPA[501],JPH[501],JPP[501],JPI[501],reglow[10],reghigh[10],cytmp,JY;
float TPA[501],TPH[501],TPP[501],TPI[501];
float AvY, sumY, peakmax, peakmin, initialcy;
float maxcy,mincy,jmi;
char a[120],del[80],*ptr, path[80],*fgets(),curp[20],foutname[200],outstr[200],jcmd[200];
char nucleus[10],solvent[20],sreglist[60],sregpath[200],h1[120],h2[120],h3[120],h4[120];
char logfile[200];
FILE *fptr,*fopen(),*fout,*flog;

// Increase JY if you want bigger spectra (& v.v.)
JY=30;

// Set maximum range of CY values
maxcy=150.0;
mincy=30.0;

// initialcy and jmi help define what intensity of peaks are picked
initialcy=500.0;
jmi=2.0;

GETCURDATA
STOREPAR("PC", 1.5)
STOREPAR("ASSFAC", 0)
STOREPAR("CY", initialcy)
STOREPAR("F1P",15.0)
STOREPAR("F2P",-0.5)
STOREPAR("MI",jmi)
STOREPAR("PSCAL",4)
FETCHPARS( "NUC1", nucleus )
FETCHPARS( "SOLVENT", solvent )

// Create files for filtered peaks and a log file of what went on
sprintf(foutname,"%s/data/%s/nmr/%s/%d/pdata/%d/filteredpeaks.txt",disk,user,name,expno,procno);
sprintf(logfile,"%s/data/%s/nmr/%s/%d/pdata/%d/cycalc.log",disk,user,name,expno,procno);
flog=fopen(logfile,"w");
fout=fopen(foutname,"w");
fprintf(flog,"JY=%f, maxcy=%f, mincy=%f, MI=%f\n",JY,maxcy,mincy,jmi);

// Find and read SREG file to allow solvent regions to be ignored
sprintf( sreglist, "%s.%s", nucleus, solvent );
sprintf( sregpath, "%slists/scl/%s", getstan(0,0), sreglist );
fprintf(flog,"region file = %s\n",sregpath);
fptr=fopen(sregpath,"r");
nregions=1;
reglow[1]=15.0;
reghigh[1]=-0.5;
if (fptr!=NULL)
{
  fgets(a,100,fptr);
  for(i=1;i<=10;i++)
  {
    if (fscanf(fptr,"%f %f",&reglow[i],&reghigh[i])<2)
    {
      fclose(fptr);
      break;
    }
    else
    {
      nregions=i;
    }
  }
}
if (nregions>1) fprintf(flog,"(%d) regions read successfully\n",nregions);


XWP_PP

// Construct path to peak pick file and open it for reading
(void)sprintf(path,"%s/data/%s/nmr/%s/%d/pdata/%d/peak.txt",disk,user,name,expno,procno);
fptr=fopen(path,"r");
if (fptr==NULL)
  {
  Proc_err(DEF_ERR_OPT,"peakfile not found");
  STOREPAR("CY", 40.0)
  fprintf(flog,"unable to open peak pick file: %s\n",path);
  fclose(flog);
  STOP
  }
//Get the peak file header information
fgets(h1,120,fptr);
fgets(h2,120,fptr);
fgets(h3,120,fptr);
fgets(h4,120,fptr);

//Read the peaks from the peak pick file
isave=1;
for (i=1;i<=1000;i++)
{
  if(fscanf(fptr,"%*d %f %f %f %f",&JPA[isave],&JPH[isave],&JPP[isave],&JPI[isave])<3)
     {
     break;
     }
     else
     {
        //keep the peaks if they are not solvent peaks
        peakvalid=0;
        for (j=1;j<=nregions;j++)
        {
          if ((JPP[isave]<reglow[j])&&(JPP[isave]>reghigh[j])) (peakvalid=1);
        }
        if (peakvalid==1) isave++;
     }
}
close(fptr);
//isave=isave-1;
fprintf(flog,"Peak file read and filtered: %d peaks\n",isave);
if (isave<5)
  {
  STOREPAR("CY", 40.0)
  fprintf(flog,"Aborting because fewer than 5 peaks in peak file: %s\n",path);
  fclose(flog);
  STOP
  }

// now remove singlets (multiplet defined as a peak with another one within 10Hz
// and an intensity of more than 0.3 of its intensity)
nmultiplets=0;
for (i=1;i<isave;i++)
{
  if (((JPH[i]-JPH[i+1]<10)&&(JPI[i+1]>0.3*JPI[i]))||((JPH[i-1]-JPH[i]<10)&&(JPI[i-1]>0.3*JPI[i])))
  {
    nmultiplets++;
    TPP[nmultiplets]=JPP[i];
    TPI[nmultiplets]=JPI[i];
    TPA[nmultiplets]=JPA[i];
    TPH[nmultiplets]=JPH[i];
  }
}

fprintf(flog,"%d multiplets detected\n",nmultiplets);
// if there are fewer than 5 multiplet peaks then use all peaks
if (nmultiplets>5)
{
// Now re-fill the array
  isave=nmultiplets;
  for (i=1;i<=isave;i++)
  {
    JPI[i]=TPI[i];
    JPP[i]=TPP[i];
    JPA[i]=TPA[i];
    JPH[i]=TPH[i];
  }
}
// find the biggest peak now they have been filtered
peakmax=0.0;
sumY=0.0;
for (i=1;i<=isave;i++)
{
  sumY=sumY+JPI[i];
  if (JPI[i]>peakmax) peakmax=JPI[i];
}

AvY=sumY/isave; // Calculate the average height
fprintf(flog,"Average peak height = %f, Max height = %f\n",AvY,peakmax);

// Look for peaks > 10% of average
peakmax=peakmax+0.1;
peakmin=0.1*AvY;
fprintf(flog,"Max peak size= %f (not used)\n",peakmax);
fprintf(flog,"Min peak size= %f\n",peakmin);
numpeaks=0;
// Now write out the filtered peak list in Bruker format
fprintf(fout,"%s",h1);
fprintf(fout,"%s",h2);
fprintf(fout,"%s",h3);
fprintf(fout,"%s",h4);
for (i=1;i<=isave;i++) {
   if (JPI[i]>peakmin) {
      numpeaks++;
      sprintf(outstr,"%6d %11.1f %12.3f %12.4f %12.2f\n",numpeaks,JPA[i],JPH[i],JPP[i],JPI[i]);
      fputs(outstr,fout);
      }
   }
fclose(fout);

// Calculate CY based on the largest multiplet peak
cytmp=(initialcy/peakmax)*JY;
fprintf(flog,"Calculated CY value= %f\n",cytmp);

// Rescale if calculated CY is outside boundaries
if (cytmp>maxcy) cytmp=maxcy;
if (cytmp<mincy) cytmp=mincy;
fprintf(flog,"Calculated CY value after limits applied= %f\n",cytmp);
fclose(flog);
sprintf(jcmd,"Calculated CY=%f",cytmp);
Show_status(jcmd);
STOREPAR("CY",cytmp)
/*
sprintf(jcmd,"cp %s %s/data/%s/nmr/%s/%d/pdata/%d/orig_peak.txt",path,disk,user,name,expno,procno);
system(jcmd);
sprintf(jcmd,"mv %s %s",foutname,path);
system(jcmd);
*/
}
/*****************************************************************************/
/*****************************************************************************/
/*
Program:  as_phase1d
Version:  1.0
Author:   John Hollerton, GSK R&D
Description: checks APKS and applies APK if necessary
/*****************************************************************************/
/*****************************************************************************/
as_phase1d(curdat)
char *curdat;
{
char a[200],path[200];
float PPI,ppmax,ppmin;
int ppminneg,i;
FILE *fptr;

GETCURDATA
APKS
STOREPAR("PC", 1.5)
STOREPAR("ASSFAC", 0)
STOREPAR("CY", 100.0)
STOREPAR("F1P",15.0)
STOREPAR("F2P",-0.5)
STOREPAR("MI",1.0)
STOREPAR("PSCAL",4)
STOREPAR("PSIGN",2)

XWP_PP
(void)sprintf(path,"%s/data/%s/nmr/%s/%d/pdata/%d/peak.txt",disk,user,name,expno,procno);
fptr=fopen(path,"r");
if (fptr==NULL)
  {
  Proc_err(DEF_ERR_OPT,"peakfile not found");
  STOP
  }
//Get the peak file header information
TIMES(4)
  fgets(a,120,fptr);
END

//Read the peaks from the peak pick file
ppmax=0.0;
ppmin=1000.0;
for (i=1;i<=1000;i++)
{
  if(fscanf(fptr,"%*d %*f %*f %*f %f",&PPI)<1)
     {
     break;
     }
     else
     {
        if (PPI<ppmin) ppmin=PPI;
        if (PPI>ppmax) ppmax=PPI;
       // Proc_err(INFO_OPT, "ppmin = %f \nppmax = %f ", ppmin, ppmax);
       // sleep(1);
     }
}
close(fptr);
ppminneg=0;
if (ppmin<0) ppminneg=1;
ppmin=ppmin*ppmin;
ppmax=ppmax*ppmax;
if ((ppmin>ppmax)&&(ppminneg=1)) APK;
}

/*****************************************************************************/
/*****************************************************************************/
/*
Program:  set_sreglist
Version:  
Author:   Joost Lohman, Bruker
Description: checks APKS and applies APK if necessary
/*****************************************************************************/
/*****************************************************************************/
set_sreglist(curdat)
char *curdat;
{
char  nucleus[20], solvent[20], sreglist[40], sregpath[PATH_LENGTH], text[100] ;

/*    Go to current data                                      */
GETCURDATA

/*    Get Nucleus and Solvent STATUS parameters               */
if ( is_avance() )
{
  FETCHPARS( "NUC1", nucleus );
}
else
{
  FETCHPARS( "NUCLEUS", nucleus );
}
FETCHPARS( "SOLVENT", solvent );

/*    Construct SREGLST parameter and store it                */
sprintf( sreglist, "%s.%s", nucleus, solvent );
STOREPAR( "SREGLST", sreglist );

/*    Construct sreglist PATH and test if it exists           */
sprintf( sregpath, "%slists/scl/%s", getstan(0,0), sreglist );
if ( access(sregpath,0) )
{
  sprintf( text, "SREGLST %s:\n%s:\nfile not found", sreglist, sregpath );
  STOPMSG( text );
}
}

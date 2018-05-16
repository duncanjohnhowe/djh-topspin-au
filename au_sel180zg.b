/*** ^^A -*-C++-*- **********************************************/
/*	au_sel180zg		23.07.2009			*/
/****************************************************************/
/*	Short Description :					*/
/*	Acquisition AU program for Selective Experiments	*/
/*	using composite experiment in ICONNMR			*/
/****************************************************************/
/*	Keywords : ICONNMR, selective excitation		*/
/****************************************************************/
/*	Description/Usage :					*/
/*	To be used as AUNM in selective experiments employing	*/
/*	a 180deg selective pulse. Pulse width of Gaussian	*/
/*      inversion pulse is calculated from SIGF1 and SIGF2      */
/*      defining the excitation region				*/
/*      Only works in this way in ICONNMR Automation mode       */
/*	for SELCOGP, SELNOGP, SELMLGP and SELROGP		*/
/****************************************************************/
/*	Author(s) :						*/
/*	Name		: Ruediger Weisemann			*/
/*	Organisation	: Bruker BioSpin GmbH			*/
/*	Email		: ruediger.weisemann@bruker-biospin.de	*/
/****************************************************************/
/*	Name		Date	Modification:			*/
/*	rwe		080129	created				*/
/*	rwe		090723  changed PL/SP to PLdB/SPdB      */
/****************************************************************/
/*
$Id: au_sel180zg,v 1.2 2009/07/23 13:36:42 wem Exp $
*/

  char  infile[PATH_MAX];
  FILE* fpi;
  float sigf1, sigf2, plref, pw_hard, sr;
  double sfo1, offs, bf1, bwref, newlev;
  double offset, pw_shape, bw_factor, int_factor, levout, pin;
  char disk_sav[PATH_MAX], user_sav[PATH_MAX], type_sav[PATH_MAX], name_sav[PATH_MAX];
  int expno_sav= expno;
    int procno_sav = procno;
  /* read important parameters */

  FETCHPAR("SIGF1", &sigf1)
  FETCHPAR("SIGF2", &sigf2)
  FETCHPAR("PLdB 1", &plref)
  FETCHPAR("P 1", &pw_hard)
  FETCHPAR("SFO1", &sfo1)
  FETCHPAR ("BF1", &bf1)
  FETCHPAR("O1", &offs)
  FETCHPAR("USERA2", text)
  
 (void)strcpy(disk_sav,disk);
 (void)strcpy(user_sav,user);
 (void)strcpy(type_sav,type);
 (void)strcpy(name_sav,name);
  
  GETCURDATA2;
  DATASET(name2,expno2,procno2,disk2,user2);
  FETCHPARS("SR",&sr)  
  DATASET(name_sav,expno_sav,procno_sav,disk_sav,user_sav);
  
  if (strcmp(text, "iconnmr") == 0)
  {
    static const char shapefile[] = "Gaus1_180r.1000";
    /***** check wave file *****/

    if (getParfileDirForRead(shapefile, SHAPE_DIRS, infile) < 0)
    {
      Proc_err(DEF_ERR_OPT, "%s: %s", shapefile, infile);
      STOP
    }

    fpi = fopen(infile,"rt");
    if (fpi == 0)
    {
      Proc_err(DEF_ERR_OPT, "cannot open file for reading:\n%s", infile);
      return -1;
    }

    /***** get bandwidth and integral factor *****/

    bw_factor = 0.0;
    int_factor = 0.0;

    while (fgets(text, sizeof(text), fpi))
    {
      char varname[40], varvalue[128];

      sscanf(text, "%40s %128s", varname, varvalue);
      if (strncmp(varname, "##$SHAPE_BWFAC", 14) == 0)
	bw_factor = atof(varvalue);

      if (strncmp(varname, "##$SHAPE_INTEGFAC", 17) == 0)
      {
	int_factor = atof(varvalue);
	break;
      }
    }

    fclose(fpi);

    bwref = (sigf1 - sigf2) * bf1;	/* desired bandwidth */
   
   
    /* one offset in the middle of the excitation region */
    offset = (sigf1 + sigf2) * (bf1/2) - offs + sr;

    /***** determine pulse length and amplitude *****/
    pw_shape = 1000000.0 * 1.0657 * bw_factor / bwref;
    pin = pw_shape * int_factor;
    levout  = 20 * log10(pin /pw_hard) + 0.05;
    levout -= fmod(levout, 0.1);
    newlev  = levout + plref;

    /*****  we need a 180deg pulse *****/
    newlev=newlev-6.0;

    STOREPAR("SPNAM2", shapefile)
    STOREPAR("P 12", pw_shape)
    STOREPAR("SPOFFS 2", offset)
    STOREPAR("SPdB 2", newlev)
  }
  ssleep(3);
  AUTOPHASE
  ssleep(3);
  XAU("au_zg", "")
  QUIT

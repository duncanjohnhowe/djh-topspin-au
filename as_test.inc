/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */


int delproc(procpath)
char *procpath;
{
//char cmd_str[100];
//sprintf( cmd_str, "rm -r %s", procpath );
//system( cmd_str );
(void) remove(procpath);

return(0);
}

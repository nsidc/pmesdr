/* this file provides dummy calls to standard ll_nr ramtek calls */

int ramopen_(int *icha, int *idev, int *iddev, int *ierr)
     /*Open and initialize a Ramtek X window */
{
  *icha = -1;
  *ierr = -1;
  return(0);
}


int rmtexture_(int *icha, int *itex1, int *iwid, int *isiz, int *ierr)
{
  return(0);
}

int rmplot_(int *icha, int *m, int *p, int *ico, int *ier)
     /*  Plot a line of connected points */
{
  return(0);
}

int rminfo_(int *icha, int *iopen, int *ix, int *iy, int *ncol, int *lcol, int *xw, int *yw, int *isize)
/*Get info on Ramtek device*/
{
  return(0);
}

int ramclose_(int *icha)   /*Terminate Ramtek X Window plotting (destroy window)*/
{
  return(0);
}

int rambox_(int *chan, int *x1, int *y1, int *x2, int *y2, int *col)
     /*Fills in a box on the Ramtek */
{
  return(0);
}

int rmclear_(int *icha, int *ierr)
/*Clear RamTek screen*/
{  
  return(0);
}

/* other dummy ramxlib calls */

int ramout_(int ichan, int ic, int *evnts)  /*evnts is really a point to an event structure */
     /*Central event loop */
{
  return(0);
}

int rmreadcursor2_(int *user(), int *icha, int ipnts[], int *jcopt, int *ihigh, 
                   int *ix1, int *iy1, int *ix2, int *iy2, int *icc)
{  /* Call event loop routine to return button press or key press */
  return(0);
}

int ramoutin_(int ichan, char *a, short int *b, long int *c, int numwords, 
		int mode)
     /*    Read/Write data from Ramtek in image mode   */
{
  return(0);
}

int rmreadcursor_(int *icha, int *idev, int *ix, int *iy, int *itrk, int *ivis, int *ient, int *ierr)
/*Call event loop routine to return button press or key press*/
{
  return(0);
}

int rmreadcol_(int *icha, int *itab, int *m, int *ierr)
/*Read color table from screen device */
{
  return(0);
}

int rmwritecol_(int *icha, int *itab, int *m, int *ierr)
/*Write color table to screen*/
{
  return(0);
}

int rmreadbyte_(int *icha, char *is, int *m, int *ierr)
     /*Read data byte from Ramtek image mode */
{
  return(0);
}

int rmreadword_(int *icha, short int *is, int *m, int *ierr)
     /*Read 2 byte word from Ramtek image mode*/
{
  return(0);
}
int rmwritebyte_(int *icha, char *is, int *m, int *ierr)
     /*Write byte data to Ramtek in image mode*/
{
  return(0);
}


int rmwriteword_(int *icha, short int *is, int *m, int *ierr)
     /*Write 2 byte words to Ramtek in image mode*/
{
  return(0);
}

int rmstart_(int *icha, int *x, int *y, int *ierr)
/*Set start of image write (cop) location to (ix,iy)*/
{
  return(0);
}

int rmwind_(int *icha, int *x, int *y, int *xm, int *ym, char *storage)
     /*Set image mode window on Ramtek*/
{
  return(0);
}

int rmdir_(int *icha, int *seq, int *ierr)
     /*Set scan sequence for image writing on Ramtek*/
{
  return(0);
}

int rmzoom_(int *ichan, int *ifact, int *ierr)
{
  return(0);
}


int rmpan_(int *ichan, int *leftx, int *topy, int *ierr)
{
  return(0);
}


int rmsetcur_(int *ichan, int *icur, int *ix, int *iy, int *ibl, int *ivi, int *ierr)
{
  return(0);
}


int rmtext_(int *icha, int *col, int *x, int *y, int *iss, char *text, int *length, int *ierr)
     /*Print text to Ramtek screen*/
{
  return(0);
}

int rmfntsize_(int *ichan, int *hd, int *vd, int *vs, int *ierr)
{
  return(0);
}


int rmreset_(int *ichan, int *ierr)
{
  return(0);
}


int rmpause_(int *ichan, int *iwait)
/*iwait = 0  checks for event in Ramtek window and processes it if present
  iwait = 1  waits for event in Ramtek window and then processes it
  */
{
  return(0);
}

int rmwrcolind_(int *icha, long *ire, long *igree, long *iblu, long *inde, int *ierr)
/*Write single color table index to Ramtek device*/
{
  return(0);
}

int rmrdcolind_(int *icha, long *ired, long *igreen, long *iblue, long *index, int *ierr)
/*Read single color table index value from screen device */
{
  return(0);
}

int rmwr4_(int *ichan, long int *is, int *n, int *ierr)
/*Write 4 byte integer words to Ramtek in image mode  */
{
  return(0);
}

int rmrd4_(int *ichan, long int *is, int *n, int *ierr)
/*Read 4 byte integer words to Ramtek in image mode  */
{
  return(0);
}

long int rmspix_(int *icha, int *x, int *y, long int *val, int *rdwr, int *ierr)
/*Read/write a single pixel on Ramtek  (very inefficient)    */
{
  return(0);
}


int rmstcol_(int *icha, int *tab, int *ncol, int *ierr)
     /*Writes a standard color table to Ramtek device*/
{
  return(0);
}

int rmmode_(int *icha, int *iwind, int *ipix, int *iswap, int *ierr)
{
  return(0);
}

int rmpoint_(int *icha, int *x, int *y, int *ico, int *ier)
{
  return(0);
}

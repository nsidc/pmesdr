/*****************************************************************************/
/* sirtool.cpp                                                               */
/*	program to display and navigate BYU .SIR image format file           */
/*                                                                           */
/* originally written March 2002 by Vaugh Clayton                            */
/* modified DGL 18 May 2002                                                  */
/* modified DGL 27 May 2002 + additional options, small memory               */
/* modified DGL 4 June 2002 + added config file options                      */
/* modified Isaac Wagner on 5 March 2003 + added midget SIR window           */
/* modified Isaac Wagner on 19 March 2003 + fixed display bug in midget win  */
/* modified Isaac Wagner on 21 March 2003 + fixed bug in option parsing      */
/*                                                                           */
/* requires fltk library and BYU sir C library code                          */
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/filename.H>

#define VER_MAJOR 1
#define VER_MINOR 6

#include "libsirtool.h"
#include "export.h"

class SirWindow;
class SirMidgetWindow;
class SirBox;

void handleChildren(SirWindow *master);
void printUsage(char *pname);

/*
void functionToUpdateMidgetWindow(Fl_Widget *w, void *data) {
  printf("Changed!\n");
}
*/
/*
 * Currently, this class is useless. Several attempts have been
 * made to use scrollbar events to update the midget window
 * viewable image rectangle. However, fltk stops the scrollbars from
 * working whenever we even try to look at an event. Maybe in a
 * future version this will be fixed. At that point we'll try again.
 */
class SirScroll: public Fl_Scroll {
public:
  SirScroll(int x, int y, int w, int h) : Fl_Scroll(x,y,w,h) {
    //scrollbar.callback(functionToUpdateMidgetWindow, NULL);
    //hscrollbar.callback(functionToUpdateMidgetWindow, NULL);
  }
};

class SirBox: public Fl_Box
{
  int mx, my;
  int zoomx, zoomy, zoomSize;
  SirScroll *scroller;
  SirWindow *master;
  SirMidgetWindow *midgetWindow;

public:
  unsigned char *img;

  friend class SirWindow;
  friend class SirZoomWindow;
  friend class SirMidgetWindow;
  friend void parseOptions(int&,char**,SirWindow*);

protected:
  void draw()
  {
    fl_overlay_clear();
    if(img) fl_draw_image(img, x(), y(), w(), h());		
    drawZoomer();
  }
  
  void drawZoomer()
  {
    fl_clip(scroller->x()+2,scroller->y()+2,scroller->scrollbar.x()-2,
	    scroller->h()-scroller->hscrollbar.h()-4);		
    fl_overlay_rect(zoomx-zoomSize/2+x(),(h()-zoomy)-zoomSize/2+y(), zoomSize, zoomSize);		
    fl_pop_clip();
  }
	
public:
	
  SirBox(int x, int y, int w, int h, SirScroll *scroll) :Fl_Box(x, y, w, h, NULL)
  {
    scroller=scroll;
    img=NULL;
    zoomx=zoomy=0;
    master=NULL;
    zoomSize=25;
  }

  void setMidget(SirMidgetWindow *mw) {
    midgetWindow = mw;
  }

  void reset_scroll()
  {	
    scroller->position(-2, -32);
  }

  int handle(int event)
  {
    //printf("event %d %d  %d %d\n",Fl::event_button(),Fl::event_key(),scroller->xposition(),scroller->yposition());
    if(Fl::event()==FL_KEYUP) {

      mx = zoomx;
      my = zoomy;

      switch(Fl::event_key()) {
      case 106:  // move left one pixel (J)
	mx--;
	break;
      case 108:  // move right one pixel (L)
	mx++;
	break;
      case 105:  // move up one pixel (I)
	my++;
	break;
      case 109:  // move down one pixel (M)
      case 44:  // move down one pixel (,)
	my--;
	break;
      case 107:  // move left five pixels (K)
	mx=mx-5;
	break;
      case 59:  // move right five pixels (;)
	mx=mx+5;
	break;
      case 117:  // move up five pixels (U)
	my=my+5;
	break;
      case 110:  // move down five pixels (N)
	my=my-5;
	break;
      default:
	// printf("Keypress: %d\n",Fl::event_key());
	return 0;		    
      }
      // printf("Got %d: %d %d %d %d\n",Fl::event_key(),mx,my,zoomx,zoomy);

      zoomx = mx;
      zoomy = my;
		  
      if(zoomx<0) zoomx=0;
      else if(zoomx>=w()) zoomx=w()-1;
      if(zoomy<0) zoomy=0;
      else if(zoomy>=h()) zoomy=h()-1;
				
      window()->make_current();
      drawZoomer();
      handleChildren(master);
      return 1;			
	  
    } else if(Fl::event_button()==FL_RIGHT_MOUSE)
      switch(event)
	{
	case FL_RELEASE:
	  fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
	  return 1;
	case FL_PUSH:
	  fl_cursor(FL_CURSOR_MOVE, FL_BLACK, FL_WHITE);
	  mx = Fl::event_x();
	  my = Fl::event_y();
	  return 1;
	case FL_DRAG:
	  int dx, dy, nx, ny;
	  nx = Fl::event_x();
	  ny = Fl::event_y();
	  dx = mx - nx;
	  dy = my - ny;
	  mx=nx;
	  my=ny;
	  if(!scroller) return 1;
	  nx = scroller->xposition();
	  ny = scroller->yposition();
	  if(nx+dx>= scroller->hscrollbar.minimum() && 
	     nx+dx<=scroller->hscrollbar.maximum())
	    nx += dx;
	  if(ny+dy>= scroller->scrollbar.minimum() && 
	     ny+dy<=scroller->scrollbar.maximum())
	    ny += dy;
	  scroller->position(nx, ny);
	  return 1;
	}

    /*
      else if(Fl::event_button()==FL_MIDDLE_MOUSE)
      switch(event)
      {
      case FL_DRAG:
      case FL_PUSH:
      handleChildren(master);
      return 1;			
      case FL_RELEASE:
      return 1;
      }
    */

    else if(Fl::event_button()==FL_LEFT_MOUSE)
      switch(event)
	{
	case FL_DRAG:
	case FL_PUSH:
	  fl_cursor(FL_CURSOR_CROSS, FL_BLACK, FL_WHITE);
	  mx = Fl::event_x();
	  my = Fl::event_y();
	  mx -= x();
	  my -= y();
	  zoomx = mx;
	  zoomy = h()-(my);

	  if(zoomx<0) zoomx=0;
	  else if(zoomx>=w()) zoomx=w()-1;
	  if(zoomy<0) zoomy=0;
	  else if(zoomy>=h()) zoomy=h()-1;
				
	  window()->make_current();
	  drawZoomer();
	  handleChildren(master);
	  return 1;			
	case FL_RELEASE:
	  fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
	  return 1;
	}
    return 0;
  }	
};

class SirMidgetWindow: public Fl_Window {
  SirBox *imageBox;
  unsigned char *midgetData, *cleanData;
  int nsx, nsy; /* how many pixels in original image */
  int scale;
  int mx,my;
  
public:

  SirMidgetWindow(int x, int y, int w, int h, SirBox *img):
    Fl_Window(x, y, w, h, "Mini Sir") 
  {
    scale = 8;
    mx = my = 0;
    midgetData = new unsigned char[w*h*3];
    memset(midgetData, 0, w*h*3);
    imageBox=img;
    end();
    set_non_modal(); // Always on top, alternative is set_normal()
  }
  
  ~SirMidgetWindow()
  {
    delete midgetData;
    delete cleanData;
  }

  void updateScale(int s) {
    scale = s;
    updateImage();
  }
  
  void updateBoundBox()
  {
    if (imageBox) {
      int x,y,w,h;
      x = imageBox->scroller->hscrollbar.value()/scale;
      y = imageBox->scroller->scrollbar.value()/scale;
      w = imageBox->scroller->hscrollbar.w()/scale;
      h = imageBox->scroller->scrollbar.h()/scale;
      if (x+w>nsx)
	w = nsx-x;
      if (y+h>nsy)
	h = nsy-y;
      if (w > nsx)
	w = nsx;
      if (h > nsy)
	h = nsy;
      //printf("x:%d y:%d w:%d h:%d\n", x,y,w,h);
      moveRect(x,y,w,h);
    }
    redraw();
  }
  
  void updateImage()
  {
    if(imageBox && imageBox->img) {
      if (midgetData) {
	delete midgetData;
	delete cleanData;
      }
      unsigned char *img = imageBox->img;
      //int nsx, nsy;
      int i,j,k,l,n,width;
      int R,G,B;

      nsx = imageBox->w();
      nsy = imageBox->h();

      width = nsx*3;

      while (nsx/scale < 75 && scale > 0) {
	scale >>= 1;
      }
      if (scale == 0) scale = 1;
      nsx = nsx/scale;
      nsy = nsy/scale;
      midgetData = new unsigned char[nsx*nsy*3];
      cleanData = new unsigned char[nsx*nsy*3];
      memset(midgetData, 0, nsx*nsy*3);
  
      for (i=0; i < nsy; i++) {
	for (j=0; j < nsx; j++) {
	  R = G = B = 0;
	  n = 0;
	  for (k=0; k < scale; k++) {
	    for (l=0; l < scale*3; l+=3) {
	      n++;
	      R = R + *(img+(i * scale+k)*width+(j * scale * 3 + l));
	      G = G + *(img+(i * scale+k)*width+(j * scale * 3 + l)+1);
	      B = B + *(img+(i * scale+k)*width+(j * scale * 3 + l)+2);
	    }
	  }
	  R = R / n;
	  G = G / n;
	  B = B / n;
	  *(midgetData + i * nsx * 3 + j * 3) = R;
	  *(midgetData + i * nsx * 3 + j * 3 + 1) = G;
	  *(midgetData + i * nsx * 3 + j * 3 + 2) = B;
	}
      }
      size(nsx,nsy);
      memcpy(cleanData, midgetData, nsx*nsy*3);
    }
    redraw();

    updateBoundBox();
  }
  
  void draw()
  {	
    fl_draw_image(midgetData, 0, 0, w(), h());
  }

  int handle(int event)
  {
    //printf("event %d %d  %d %d\n",Fl::event_button(),Fl::event_key(),scroller->xposition(),scroller->yposition());
    if(Fl::event_button()==FL_LEFT_MOUSE)
      switch(event)
	{
	case FL_DRAG:
	case FL_PUSH:
	  int wx,wy,ww,wh;
	  fl_cursor(FL_CURSOR_CROSS, FL_BLACK, FL_WHITE);
	  wx = imageBox->scroller->hscrollbar.value()/scale;
	  wy = imageBox->scroller->scrollbar.value()/scale;
	  ww = imageBox->scroller->hscrollbar.w()/scale;
	  wh = imageBox->scroller->scrollbar.h()/scale;
	  if (wx+ww>nsx)
	    ww = nsx-wx;
	  if (wy+wh>nsy)
	    wh = nsy-wy;
	  if (ww > nsx)
	    ww = nsx;
	  if (wh > nsy)
	    wh = nsy;
	  mx = Fl::event_x();
	  my = Fl::event_y();
	  if (mx+ww/2 >= w()) 
	    mx = w() - ww;
	  else if (mx-ww/2 < 0) 
	    mx = 0;
	  else 
	    mx = mx-ww/2;
	  if (my+wh/2 >= h())
	    my = h() - wh;
	  else if (my-wh/2 < 0) 
	    my = 0;
	  else
	    my = my-wh/2;
	  //imageBox->scroller->position(mx*scale,my*scale);
	  //updateBoundBox();
	  moveRect(mx,my,ww,wh);
	  return 1;			
	case FL_RELEASE:
	  fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
	  imageBox->scroller->position(mx*scale,my*scale);
	  //updateBoundBox();
	  return 1;
	}
    return 0;
  }
private:
  void moveRect(int x, int y, int w, int h) {
    memcpy(midgetData, cleanData, nsx*nsy*3);
    for (int j = y; j < y+h; j++) {
      for (int i = x; i < (x+w); i++) {
	// make the box two pixels wide
	if (j == y     || j == y+1   ||
	    j == y+h-1 || j == y+h-2 ||
	    i == x     || i == x+1   ||
	    i == x+w-1 || i == x+w-2) {
	  // complement the color of the pixel
	  midgetData[j*nsx*3 + i*3] = 255 - midgetData[j*nsx*3 + i*3];
	  midgetData[j*nsx*3 + i*3 + 1] = 255 - midgetData[j*nsx*3 + i*3+1];
	  midgetData[j*nsx*3 + i*3 + 2] = 255 - midgetData[j*nsx*3 + i*3+2];
	}
      }
    }
    redraw();
  }
};



class SirZoomWindow: public Fl_Window
{
  SirBox *imageBox;
  unsigned char *zoomData;
  int mx,my;
	
public:
  int zoomx, zoomy, xpixels, ypixels;
  int zoomFactor, zoomSize;

  SirZoomWindow(int x, int y, int w, int h, SirBox *img):
    Fl_Window(x, y, w, h, "Zoom")
  {
    zoomx=0; zoomy=0;
    mx = my = 0;
    xpixels=0; ypixels=0;
    zoomFactor=4;
    zoomSize=25;
    zoomData = new unsigned char[w*h*3];
    memset(zoomData, 0, w*h*3);
    imageBox=img;
    end();
    set_non_modal(); // Always on top, alternative is set_normal()
  }
	
  ~SirZoomWindow()
  {
    delete zoomData;
  }
	
  void updateZoom(int zfactor, int zsize)
  {
    int s = zfactor*zsize;		
    zoomFactor=zfactor;
    zoomSize=zsize;

    delete zoomData;
    zoomData = new unsigned char[s*s*3];
    size(s, s);
    updateImage(zoomx, zoomy);
    redraw();
  }
	
  void updateImage(int zx, int zy)
  {
    zoomx=zx;
    zoomy=zy;
		
    if(imageBox && imageBox->img) {
      unsigned char *img = imageBox->img;
      int q, r, s, t, x, y;
      char R,G,B;
      unsigned int offset=0;
			
      for(r=0; r<zoomSize; r++)
	for(q=0; q<zoomSize; q++) {
	  x = zoomx + q -zoomSize/2;
	  y = ypixels - zoomy + r - zoomSize / 2 - 1;
	  if(x>=0 && x<xpixels && y>=0 && y<ypixels) {
	    offset = x*3+y*xpixels*3;
	    R = img[offset++];
	    G = img[offset++];
	    B = img[offset];
	  } else 
	    R=G=B=128;

	  for(s=0; s<zoomFactor; s++)
	    for(t=0; t<zoomFactor; t++) {
	      zoomData[(q*zoomFactor+s)*3+(r*zoomFactor+t)*w()*3]=  R;
	      zoomData[(q*zoomFactor+s)*3+(r*zoomFactor+t)*w()*3+1]=G;
	      zoomData[(q*zoomFactor+s)*3+(r*zoomFactor+t)*w()*3+2]=B;
	    }			
	}
      x = zoomSize * zoomFactor / 2;
      r = zoomFactor / 2;
      for(q=-r-1; q<=r; q++) {
	for(t=0; t<3; t++) {
	  zoomData[(x+q)*3+(x-r-1)*w()*3+t]=255;
	  zoomData[(x+q)*3+(x+r)*w()*3+t]  =255;
	  zoomData[(x-r-1)*3+(x+q)*w()*3+t]=255;
	  zoomData[(x+r)*3+(x+q)*w()*3+t]  =255;
	}
      }
    }
  }

  int handle(int event)
  {
    //printf("event %d %d  %d %d\n",Fl::event_button(),Fl::event_key(),scroller->xposition(),scroller->yposition());
    if(Fl::event_button()==FL_LEFT_MOUSE)
      switch(event)
	{
	  //case FL_DRAG:
	case FL_PUSH:
	  fl_cursor(FL_CURSOR_CROSS, FL_BLACK, FL_WHITE);
	  mx = Fl::event_x();
	  my = Fl::event_y();
	  mx = mx/zoomFactor;
	  my = my/zoomFactor;
	  mx = (mx-zoomSize/2)+zoomx;
	  my = (zoomSize/2-my)+zoomy;
	  //printf("x=%d y=%d\n",mx,my);
	  return 1;			
	case FL_RELEASE:
	  fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
	  //imageBox->window()->make_current();
	  //imageBox->drawZoomer();
	  //handleChildren(imageBox->master);
	  updateImage(mx,my);
	  redraw();
	  return 1;
	}
    return 0;
  }
	
  void draw()
  {	
    fl_draw_image(zoomData, 0, 0, w(), h());
  }
};




class SirWindow: public Fl_Window
{
  int mx, my;
  int zoomx, zoomy;
  SirBox *imageBox;
  Fl_Output *statusBar;
  SirScroll *scroller;
  SirZoomWindow *zoomWindow;
  SirMidgetWindow *midgetWindow;

  float *sirdata;
  int small_mem;
  FILE *sir_file;

  char *cfg_args[MAX_CFG];
  int ncfg_args;
  char *input_fname;  

  unsigned char *img;
  struct sirheader sh;
  
  float datamin, datamax;
  float lon, lat;

  int z_opt, s_opt, e_opt;
  float usemin, usemax;	
  unsigned char palette[768];

  friend void parseOptions(int&, char**, SirWindow*);
  friend void readConfig(char*, char**, int*, SirWindow*);
  friend class SirMidgetWindow;

public:

  SirWindow(int w, int h, const char *title=0): Fl_Window(w,h,title)
  {
    imageBox=NULL;
    statusBar=NULL;
    scroller=NULL;
    ncfg_args=0;
    z_opt=0; s_opt=0; e_opt=255;

    Fl_Menu_Item menuitems[] = 
    {
      {"&File",	0, 0, 0, FL_SUBMENU },
      {"&About", FL_CTRL + 'a', (Fl_Callback *)cbAbout, this },
      {"&Open SIR file", FL_CTRL + 'o', (Fl_Callback *)cbOpen, this },
      {"&Export", 0, 0, 0, FL_SUBMENU},
      {"&ASCII form", FL_ALT + 'a', (Fl_Callback*)cbExportA, this },
      {"One &Byte/pixel", FL_ALT + 'b', (Fl_Callback*)cbExportB, this },
      {"&GIF", FL_ALT + 'g', (Fl_Callback*)cbExportGIF, this },
      {"B&MP", FL_ALT + 'm', (Fl_Callback*)cbExportBMP, this },
      {0},
      {"Image &Properties", FL_ALT+FL_Enter, (Fl_Callback*)cbProp, this },
      {"&Read config file", FL_CTRL + 'r', (Fl_Callback *)cbReadCfg, this, FL_MENU_DIVIDER },
      {"&Quit", FL_CTRL + 'q', (Fl_Callback *)cbQuit, 0 },
      {0},
      {"Data_Range", FL_CTRL+'d', (Fl_Callback*)cbData, this, FL_MENU_DIVIDER},
      {"&Zoom", 0, 0, 0, FL_SUBMENU },
      {"&Factor", 0, 0, 0, FL_SUBMENU },
      {"2X", FL_CTRL+'1', (Fl_Callback*)cbZoom, this },
      {"4X", FL_CTRL+'2', (Fl_Callback*)cbZoom, this },
      {"6X", FL_CTRL+'3', (Fl_Callback*)cbZoom, this },
      {"8X", FL_CTRL+'4', (Fl_Callback*)cbZoom, this },
      {0},
      {"&Size", 0, 0, 0, FL_SUBMENU},
      {"B&igger", FL_CTRL+'[', (Fl_Callback*)cbZoom, this },
      {"S&maller", FL_CTRL+']', (Fl_Callback*)cbZoom, this },
      {0},
      {0},
      {"&Colortable", 0, 0, 0, FL_SUBMENU },
      {"&Gray Colortable", 0, (Fl_Callback*)cbGrayColor, this },
      {"&Open New Colortable", 0, (Fl_Callback*)cbOpenColor, this },
      {"&Save Colortable", 0, 0, 0, FL_SUBMENU },
      {"&ASCII form", 0, (Fl_Callback*)cbSaveColor, this },
      {"&Binary form", 0, (Fl_Callback*)cbSaveColor, this },
      {0},
      {0},
      {"Help", FL_CTRL+'h', (Fl_Callback*)cbHelp, this},
      {0}
    };
		
    // printf("init menu\n");
    Fl_Menu_Bar *mainmenu = new Fl_Menu_Bar(0, 0, w, 30);
    // printf("menu init'd\n");
    if(!mainmenu)
      printf("Error initializing menu!");
    else
      mainmenu->copy(menuitems);
    // printf("menu items set\n");
		
    SirScroll *scroller = new SirScroll(0,30,w,h-60);
    imageBox = new SirBox(0,0,0,0, scroller);
    imageBox->master=this;
    imageBox->box(FL_NO_BOX);
    imageBox->color(FL_BLACK);
    scroller->box(FL_DOWN_BOX);
    scroller->end();
    statusBar = new Fl_Output(0,h-30,w,30);
    statusBar->color(FL_GRAY);
    statusBar->value("No image loaded");

    for(int q=0; q<256; q++) // Default gray palette
      palette[q]=palette[q+256]=palette[q+512]=(unsigned char) q;

    LoadColorTable("default.ct", 1, 1);  // load default colortable file if present

    resizable(scroller);
    end();

    zoomWindow = new SirZoomWindow(1,30,100,100,imageBox);
    zoomWindow->xpixels = imageBox->w();
    zoomWindow->ypixels = imageBox->h();

    midgetWindow = new SirMidgetWindow(1,200,100,100,imageBox);
    imageBox->setMidget(midgetWindow);
		
    usemin=99999;
    usemax=-99999;
    lon=999;
    lat=999;
  }

  void NoMini(void) {
    if (midgetWindow) {
      delete midgetWindow;
      midgetWindow = NULL;
    }
  }

  void LoadFile(const char *fname, int *nsx, int *nsy)
  {
    float float_x = -100000.0, float_y = 100000.0, xs;
    long size;
    int i, j;

    //printf("In LoadFile %d %d %d\n", small_mem, (int) sirdata, (int) sir_file);
		
    if (sir_file != NULL) {  // close open file if small memory option on
      fclose(sir_file);
      sir_file = NULL;
      if (sirdata != NULL) delete sirdata;
    } 
    // else if (sirdata != NULL) free(sirdata);
		
    // check for existence of file

    sir_file = fopen((const char *) fname, "r");
    if (sir_file == NULL) {
      fprintf(stderr,"*** Could not open input file %s\n",fname);
      fflush(stderr);
      return;
    }

    if (small_mem) {  // use minimum memory (slower, requires more file accesses)
      if (read_sir_header(sir_file, &sh)) {
	fprintf(stderr,"*** Could not read input file ***\n");
	fflush(stderr);
	return;
      }
      sirdata = new float[sh.nsx+1];
      if(sirdata==NULL) {
	fprintf(stderr, "*** Error allocating SIR memory. \n"); 
	fflush(stderr);
	return;
      }

      for(i = 0; i < sh.nsy; i++) {
	j=sir_datablock(sir_file, sirdata, &sh, 1, i, sh.nsx, i);
	for(size = 0; size < sh.nsx; size++) {
	  xs = sirdata[size];
	  if(xs < float_y) float_y = xs;
	  if(xs > float_x) float_x = xs;
	}
      }
		  
    } else {    // store full sir image in memory (faster, but requires more memory)
      fclose(sir_file);
      sir_file=NULL;

      //printf("in load file %s\n",fname);		
      sirdata = LoadSIR((char*)fname, &sh);
      //printf("file loaded %d\n",(int) sirdata);
		
      if(sirdata==NULL) {
	fprintf(stderr, "*** Error allocating SIR memory. \n"); fflush(stderr);
	return;
      }

      for(size = 0;  size < sh.nsx * sh.nsy; size++) {
	xs = sirdata[size];
	if(xs < float_y) float_y = xs;
	if(xs > float_x) float_x = xs;
      }

    }
		
    datamin = float_y;
    datamax = float_x;

    if(usemin==99999) usemin = sh.v_min;
    if(usemax==-99999) usemax = sh.v_max;
		
    char buf[1024];
    sprintf(buf, "Image '%s': %dx%d Min: %f Max: %f", fname, sh.nsx, sh.nsy, usemin, usemax);
    statusBar->value(buf);
		
    *nsx = sh.nsx;
    *nsy = sh.nsy;

    Fl_Window::label(fname);   // set window title to name of file
  }

  void GenerateImage()
  {
    float dscale, val;
    long size, i, j, q;
    int k;
		
    size = sh.nsx * sh.nsy * 3;
    if(img) delete img;
    img = new unsigned char[size];
    if (img == NULL) {
      printf("*** Problem creating image display array\n");
      exit(-1);
    }

    // printf("Gen image: %f %f %d %d %d\n",usemin, usemax,z_opt,s_opt,e_opt);
    dscale = 255.0/(usemax-usemin);
    q=0;
    for(j=0; j<sh.nsy; j++) {
      if (small_mem) {
	q=0;
	(void) sir_datablock(sir_file, sirdata, &sh, 1, j, sh.nsx, j);
      }		  
      for(i=0; i<sh.nsx; i++) {
	val = sirdata[q++];
	if (fabs(val-sh.anodata) < 0.001)  // image is at nodata value
	  k = z_opt;
	else {
	  val = (val-usemin)*dscale;
	  if(val<s_opt) val=s_opt;
	  if(val>e_opt) val=e_opt;
	  k = (int) val;
	}
	img[i*3+(sh.nsy-j-1)*sh.nsx*3+0] = palette[k];
	img[i*3+(sh.nsy-j-1)*sh.nsx*3+1] = palette[k+256];
	img[i*3+(sh.nsy-j-1)*sh.nsx*3+2] = palette[k+512];
      }
    }

    imageBox->size(sh.nsx, sh.nsy);
    imageBox->img = img;

    if (midgetWindow) {
      midgetWindow->updateImage();
      midgetWindow->show();
    }

    // add annotation from config file

    // printf("sir_plot %d\n",ncfg_args);
    if (ncfg_args > 0)
      sir_plot(&sh,img,palette,cfg_args,ncfg_args,input_fname,s_opt,e_opt);

  }


  void LoadColorTable(const char *fname, int ttype, int skip_error)
  {
    char fullname[1024];
    FILE *f;
    int q, r, g, b;

    fl_filename_expand(fullname, fname);
    if((f=fopen(fullname, "r"))==NULL) {
      if (skip_error == 0)
	fprintf(stderr, "Could not open colortable: '%s'\n", fullname);
      if(f) fclose(f);
      return;
    }
    if(ttype==1) // Binary table, red*256, green*256, blue*256. (Not interleaved.)
      fread(palette, 1, 768, f);
    else if(ttype==2) { // ASCII colortable
      for(q=0; q<256; q++)
	if(fscanf(f, "%d %d %d", &r, &g, &b)==EOF) {
	  fprintf(stderr, "Error reading colortable: '%s'\n", fullname);
	  if(f) fclose(f);
	  return;
	} else {
	  palette[q]=r;
	  palette[q+256]=g;
	  palette[q+512]=b;
	}
    }
    fclose(f);
  }

  void SaveColorTable(const char *fname, int ttype)
  {
    char fullname[1024];
    FILE *f;
    int q;

    fl_filename_expand(fullname, fname);
    if((f=fopen(fullname, "w"))==NULL) {
      fprintf(stderr, "Could not open file '%s' for writing\n", fullname);
      fclose(f);
      return;
    }
    if(ttype==1) // Binary table, red*256, green*256, blue*256. (Not interleaved.)
      fwrite(palette, 1, 768, f);
    else if(ttype==2) // ASCII colortable
      for(q=0; q<256; q++)
	fprintf(f, "%d %d %d\n", palette[q], palette[q+256], palette[q+512]);
    fclose(f);
  }

  void Export_file(const char *fname, int ttype)
  {
    char fullname[1024];
    FILE *f;
    int q, i, j;
    unsigned char byt;
    float dscale, val;

    fl_filename_expand(fullname, fname);
    if((f=fopen(fullname, "w"))==NULL) {
      fprintf(stderr, "Could not open file '%s' for writing\n", fullname);
      fclose(f);
      return;
    }
    if(ttype==1) { // Binary
      dscale = 255.0/(usemax-usemin);

      q=0;
      for(j=0; j < sh.nsy-1; j++) {
	if (small_mem) {
	  q=0;
	  (void) sir_datablock(sir_file, sirdata, &sh, 1, j+1, sh.nsx, j+1);
	}
	for(i=0; i < sh.nsx; i++) { 
	  val = (sirdata[q++]-usemin)*dscale;
	  if(val<0) val=0;
	  if(val>255) val=255;
	  byt = (unsigned char) ((int) val);
	  fwrite(&byt, 1, 1, f);
	}		    
      }

    } else {         // ASCII

      q=0;
      for(j=0; j < sh.nsy-1; j++) {
	if (small_mem) {
	  q=0;
	  (void) sir_datablock(sir_file, sirdata, &sh, 1, j+1, sh.nsx, j+1);
	}
	for(i=0; i < sh.nsx; i++)
	  fprintf(f,"%f ",sirdata[q++]);
	fprintf(f,"\n");
      }
    }
    fclose(f);
    //printf("Export file size: %d x %d\n",sh.nsx,sh.nsy);
		
    char message[1024];
    sprintf(message,"Export file size: %d x %d\n",sh.nsx,sh.nsy);
    fl_message((const char *) message);
    //printf("Export file size: %d x %d\n",sh.nsx,sh.nsy);

  }




private:
  void hide()
  {
    if(zoomWindow) zoomWindow->hide();
    if(midgetWindow) midgetWindow->hide();
    Fl_Window::hide();
  }
	
  static void cbAbout(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbAbout();}
  void _cbAbout()
  {
    printf("\nsirtool: BYU X-windows .SIR file viewer\n");
    printf("Use sirtool -h or sirtool --help for arguments\n\n");
    printf("Written by V.Clayton and D.Long 2002\n\n");
  }

  static void cbOpen(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbOpen();}
  void _cbOpen()
  {
    const char *fname;
    fname = fl_file_chooser("Select .sir file to open","*.sir","");
    if(fname) {
      fl_cursor(FL_CURSOR_WAIT, FL_BLACK, FL_WHITE);
      Fl::flush();
      int nsx,nsy;

      input_fname = (char *) fname;
      LoadFile(fname, &nsx, &nsy);
      GenerateImage();

      fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
      Fl::flush();
    }
    redraw();
  }

  static void cbExportA(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbExportA();}
  void _cbExportA()
  {
    if(sirdata==NULL) { // no file loaded	
      printf("No SIR file loaded\n");
      return;  
    }		

    const char *fname;
    fname = fl_file_chooser("Select ASCII export file name","","");
    if(fname)
      Export_file(fname, 0);

  }

  static void cbExportB(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbExportB();}
  void _cbExportB()
  {
    if(sirdata==NULL) { // no file loaded	
      printf("No SIR file loaded\n");
      return;  
    }		

    const char *fname;
    fname = fl_file_chooser("Select byte export file name","","");
    if(fname)
      Export_file(fname, 1);

  }

  static void cbExportGIF(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbExportGIF();}
  void _cbExportGIF()
  {
    if(sirdata==NULL) { // no file loaded	
      printf("No SIR file loaded\n");
      return;  
    }		

    const char *fname;
    int i,j,k;
    fname = fl_file_chooser("Select GIF export file name","*.gif","");
    if(fname) {

      int size = sh.nsx * sh.nsy;
      unsigned char *data;
      data = new unsigned char[size];
      float dscale = 255.0/(usemax-usemin), val;

      int q=0;
      for(j=0; j < sh.nsy; j++) {
	if (small_mem) {
	  q=0;
	  (void) sir_datablock(sir_file, sirdata, &sh, 1, j+1, sh.nsx, j+1);
	} 	      
	for(i=0; i<sh.nsx; i++) {
	  val = (sirdata[q++]-usemin)*dscale;
	  if(val<0) val=0;
	  if(val>255) val=255;
	  data[i+(sh.nsy-j-1)*sh.nsx] = (int) val;
	}
      }
	    
      i=0;
      k=strlen(fname);
      if (writegif((char *) fname, &k, (char *) data, &sh.nsx, &sh.nsy, (char *) &palette[0], (char *)&palette[256], (char *) &palette[512], &j, &i) < 0)
	printf ("*** ERROR writing export file ***\n");
      delete data;
    }
  }


  static void cbExportBMP(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbExportBMP();}
  void _cbExportBMP()
  {
    if(sirdata==NULL) { // no file loaded	
      printf("No SIR file loaded\n");
      return;  
    }		

    const char *fname;
    int i,j;
    fname = fl_file_chooser("Select BMP export file name","*.bmp","");
    if(fname) {

      int size = sh.nsx * sh.nsy;
      unsigned char *data;
      data = new unsigned char[size];
      float dscale = 255.0/(usemax-usemin), val;

      int q=0;
      for(j=0; j < sh.nsy; j++) {
	if (small_mem) {
	  q=0;
	  (void) sir_datablock(sir_file, sirdata, &sh, 1, j+1, sh.nsx, j+1);
	}
	for(i=0; i<sh.nsx; i++) {
	  val = (sirdata[q++]-usemin)*dscale;
	  if(val<0) val=0;
	  if(val>255) val=255;
	  data[i+j*sh.nsx] = (int) val;
	}
      }
	    
      if (writebmp((char *) fname, (char *) data, &sh.nsx, &sh.nsy, (char *) &palette[0], (char *)&palette[256], (char *) &palette[512], &j) < 0)
	printf ("*** ERROR writing export file ***\n");
      delete data;
    }
  }

	
  static void cbProp(Fl_Widget *src, void *cbData){((SirWindow*)cbData)->_cbProp();}
  void _cbProp() // File Properties dialog
  {
    if(sirdata==NULL) { // no file loaded
      printf("No SIR file loaded\n");
      return;  
    }		
    print_sir_header(sh);
  }


  static void cbReadCfg(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbReadCfg((SirWindow*)cbData);}
  void _cbReadCfg(SirWindow *wnd)
  {
    const char *fname;
    fname = fl_file_chooser("Select config file to open","*.s2g","");
    if(fname) {
      fl_cursor(FL_CURSOR_WAIT, FL_BLACK, FL_WHITE);
      Fl::flush();

      if (ncfg_args > 0) {  // free storage from previous config file
	int i;
	for (i=0; i< ncfg_args; i++)
	  free(cfg_args[i]);
	ncfg_args = 0;
      }

      readConfig((char *) fname, cfg_args, &ncfg_args, wnd);

      GenerateImage();

      fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
      Fl::flush();
    }
    redraw();
  }

	
  static void cbQuit(Fl_Widget *src, void *cbData)
  {
    exit(0);
  }

  static void cbZoom(Fl_Widget *src, void *cbData)
  {
    Fl_Menu_ *mnu;
    mnu = (Fl_Menu_ *)src;
    const Fl_Menu_Item *itm = mnu->mvalue();
    ((SirWindow*)cbData)->_cbZoom(itm->text[0]-'0');
  }
	
  void _cbZoom(int val)
  {
    int s;
    if(val>0 && val<=16) {	// Zoom Factor

      zoomWindow->updateZoom(val, zoomWindow->zoomSize);
      zoomWindow->show();

    } else {	// Zoom window size

      if(val=='B'-'0') {   //  bigger

	s = (int)(zoomWindow->zoomSize * 1.5);
	if(!(s&1)) s++;
	if(s>75) return;
	zoomWindow->updateZoom(zoomWindow->zoomFactor, s);
	imageBox->zoomSize=s;
	redraw();

      } else {              //  smaller

	s = (int)(zoomWindow->zoomSize * 0.666667);
	if(!(s&1)) s++;
	if(s<8) return;
	zoomWindow->updateZoom(zoomWindow->zoomFactor, s);
	imageBox->zoomSize=s;
	redraw();
      }
    }
  }
	
  static void cbData(Fl_Widget*src, void*data){((SirWindow*)data)->_cbData();}
  void _cbData()
  {
    float dmin, dmax;
    char val[1024];
    const char *ptr;
		
    sprintf(val, "%f", usemin);
  GetMin:
    ptr = fl_input("Enter the minimum visible data value:", val);
    if(!ptr) return;
    if(!sscanf(ptr, "%f", &dmin)) goto GetMin;
    sprintf(val, "%f", usemax);
  GetMax:
    ptr = fl_input("Enter the maximum visible data value:", val);
    if(!ptr) return;
    if(!sscanf(ptr, "%f", &dmax)) goto GetMax;
    usemin=dmin;
    usemax=dmax;
    GenerateImage();
    redraw();
  }

  static void cbGrayColor(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbGrayColor();}
  void _cbGrayColor()
  {	
    int q;  // reset color table to gray
    for(q=0; q<256; q++) {
      palette[q]=q;
      palette[q+256]=q;
      palette[q+512]=q;
    }

    if(sirdata==NULL) return;  // no image loaded

    GenerateImage();

    handleChild();  // update color in zoom window
    redraw();       // update color in main window
  }

  static void cbOpenColor(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbOpenColor();}
  void _cbOpenColor()
  {
    const char *fname;
    fname = fl_file_chooser("Select colortable to open","*.ct*","");
    if(fname) {
      if(fname[strlen(fname)-1]=='t')
	LoadColorTable(fname, 1, 0);
      else
	LoadColorTable(fname, 2, 0);
      if(sirdata==NULL) return;  // no image loaded	
      GenerateImage();
    }
    if(sirdata==NULL) return;  // no image loaded
    handleChild();  // update color in zoom window
    redraw();       // update color in main window
  }

  static void cbSaveColor(Fl_Widget *src, void *cbData)
  {
    Fl_Menu_ *mnu;
    mnu = (Fl_Menu_ *)src;
    const Fl_Menu_Item *itm = mnu->mvalue();
    ((SirWindow*)cbData)->_cbSaveColor(itm->text[0]=='A'?2:1);
  }
  void _cbSaveColor(int type)
  {
    const char *fname;
    fname = fl_file_chooser("Select output for colortable ","*.ct","");
    if(fname)
      SaveColorTable(fname, type);
  }

  static void cbHelp(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbHelp();}
  void _cbHelp()
  {
    printf("\nsirtool: BYU X-windows .SIR file viewer\n");
    printUsage((char *) "sirtool");
    printf("\nClicking left mouse button outputs cursor position and pixel value at bottom of\nthe window.  The cursor positions is given in SIR pixels (counting from lower-\nleft corner) and lat/lon.  The right mouse button moves the image within\nthe window if the image is larger than the window.\n");
		
    printf("Exporting dumps the pixel values to a specified file as either the floating\npoint ASCII (one ASCII value/pixel) or as scaled bytes (one byte/pixel).\nThe exported pixel order is left-to-right, bottom-to-top (standard SIR order).\n");
    printf("ASCII files are space delimited with a newline after each row.\n");

    printf("\nColor tables have 256 entries.  ASCI files have 256 lines with 3 integer\n(0..255) values (R,G,B) per line.  Binary files have 256 bytes of R, 256 bytes\nof G, and 256 bytes B in that order.\n");
    printf("\nAt start up, the program read the colortable file 'default.ct' from\nthe local directory, if it exists.\n");
    printf("\nSingle pixel keyboard movement: j=left l=right i=up m or ,=down\n");
    printf("\nFive pixel keyboard movement: k=left ;=right u=up n=down\n");

  }

	

public:
  void handleChild()
  {
    float lat, lon, temp;
    char buf[1024];
    *buf=0;
		
    zoomx = imageBox->zoomx;
    zoomy = imageBox->zoomy;
		
    pixtolatlon(&sh, zoomx, zoomy, &lon, &lat);

    if (small_mem)
      (void) sir_datablock(sir_file, &temp, &sh, zoomx+1, zoomy+1, zoomx+1, zoomy+1);
    else
      temp=sirdata[zoomx+zoomy*sh.nsx];

    sprintf(buf, "%d, %d,  Lon: %f Lat: %f  Value: %f", 
	    zoomx, zoomy, lon, lat, temp);
		    
    if(statusBar) statusBar->value(buf);

    if(zoomWindow) {
      zoomWindow->zoomx = zoomx;
      zoomWindow->zoomy = zoomy;
      zoomWindow->xpixels = sh.nsx;
      zoomWindow->ypixels = sh.nsy;
      zoomWindow->updateImage(zoomx, zoomy);
      zoomWindow->redraw();
      zoomWindow->show();
    }
		
    if (midgetWindow) {
      midgetWindow->updateBoundBox();
      midgetWindow->show();
    }
		
  }

};

void handleChildren(SirWindow *master)
{
  master->handleChild();	
}



void readConfig(char *fname, char *cfg_args[], int *ncfg_args, SirWindow *wnd)  // read annotation info into an array
{
  FILE *fid;
  char line[1024], *ptr, *c;
  int len, i, flag;
  int z_opt, s_opt, e_opt, k,r,g,b;

  char *opts[]=  // one-time execute configu options recognized here
  {
    "-e",   // color table options (overrides default)
    "-s",   // image scale options (overrides default)
    "-C",   // read a binary color table file (overrides default)
    "-A",   // read an ascii color table file  (overrides default)
    0
  };

  fid=fopen(fname,"r");
  if (fid == NULL) {
    fprintf(stderr,"*** Could not open config file '%s' ***\n",fname);
    return;
  }
  while (fscanf(fid,"%s",line) > 0) {

    // determine if config command is a one-time execute (e.g. set color table)
    // or if it is reused each time image is regenerated

    flag = 1;
    for(i=0; opts[i]; i++) {
      if(!strncmp(opts[i], line, 2)) { // match
	switch(i) {
	case 0:  // "-e"  // set color table index value
	  k=r=g=b=0;
	  if ((c = strstr(line+1,"i=")) != NULL) { /* index */
	    sscanf(c+2,"%d",&k);
	    if (k > 255) k=255;
	    if (k < 0) k=0;
	  }
	  if ((c = strstr(line+1,"r=")) != NULL) { /* index */
	    sscanf(c+2,"%d",&r);
	    if (r > 255) r=255;
	    if (r < 0) r=0;
	  }
	  if ((c = strstr(line+1,"g=")) != NULL) { /* index */
	    sscanf(c+2,"%d",&g);
	    if (g > 255) g=255;
	    if (g < 0) g=0;
	  }
	  if ((c = strstr(line+1,"b=")) != NULL) { /* index */
	    sscanf(c+2,"%d",&b);
	    if (b > 255) b=255;
	    if (b < 0) b=0;
	  }
	  wnd->palette[k]=(unsigned char) r;
	  wnd->palette[k+256]=(unsigned char) g;
	  wnd->palette[k+512]=(unsigned char) b;
	  // printf("Set color table index %d to r=%d g=%d b=%d\n",k,r,g,b);
	  break;
	case 1:  // "-s"
	  z_opt = wnd->z_opt;
	  s_opt = wnd->s_opt;
	  e_opt = wnd->e_opt;
	  if ((c = strstr(line+1,"z=")) != NULL) { /* nodata color index */
	    sscanf(c+2,"%d",&z_opt);
	    if (z_opt > 255) z_opt=255;
	    if (z_opt < 0) z_opt=0;
	  }
	  if ((c = strstr(line+1,"s=")) != NULL) { /* starting color index */
	    sscanf(c+2,"%d",&s_opt);
	    if (s_opt > 255) s_opt=255;
	    if (s_opt < 0) s_opt=0;
	  }
	  if ((c = strstr(line+1,"e=")) != NULL) { /* ending color index */
	    sscanf(c+2,"%d",&e_opt);
	    if (e_opt > 255) e_opt=255;
	    if (e_opt < 0) e_opt=0;
	  }
	  if ((c = strstr(line+1,"l")) != NULL) { /* low image value */
	    if (*(c+1) == '=')
	      sscanf(c+2,"%f",&wnd->usemin);
	  }
	  if ((c = strstr(line+1,"h")) != NULL) { /* high image value */
	    if (*(c+1) == '=')
	      sscanf(c+2,"%f",&wnd->usemax);
	  }
	  if (e_opt < s_opt) e_opt=s_opt;
	  wnd->z_opt = z_opt;
	  wnd->s_opt = s_opt;
	  wnd->e_opt = e_opt;
	  // printf("Set scale range %f %f to %d %d  nodata=%d\n",wnd->usemin,wnd->usemax,s_opt,e_opt,z_opt);
	  break;
	case 2:  // "-C"
	  // printf("load binary colortable '%s'\n",&line[3]);
	  wnd->LoadColorTable(&line[3], 1, 1);
	  break;
	case 3:  // "-A"
	  // printf("load ASCII colortable '%s'\n",&line[3]);
	  wnd->LoadColorTable(&line[3], 2, 1);
	  break;
	}
	flag = 0;
      }
    }
    if (flag) {
      len = strlen(line)+1;
      if (len > 1024) len=1024;
      ptr = new char[len];
      if (ptr == NULL) {
	printf("*** Error internally storing config file\n");
	exit(-1);
      }
      cfg_args[(*ncfg_args)++] = strncpy(ptr,line,len);
    }
  }
  fclose(fid);
  // printf("%d entries read from %s\n",*ncfg_args,fname);
}

const char *versionString()
{
  static char vstr[32];
  sprintf(vstr, "%d.%d", VER_MAJOR, VER_MINOR);
  return vstr;
}

void printUsage(char *pname)
{
  printf("Usage: %s <options> <sir_file> <vis_min <vis_max <fltk_args>>>\n", pname);
  printf("Options:\n");
  printf("  -dmin n.n      Minimum visible data value\n");
  printf("  -dmax n.n      Maximum visible data value\n");
  printf("  -lon n.n       Initial cursor longitude\n");
  printf("  -lat n.n       Initial cursor latitude\n");
  printf("  -cfg file      Read config annotations from file (see below)\n");
  printf("  -cta file      Use ASCII colortable (R1 G1 B1\\nR2 G2 B2\\n...\n");
  printf("  -ctb file      Use binary colortable (R1..R255,G1..G255,B1..B255\n");
  printf("  -mem,-smallmem Use mimimum memory (slower, more disk i/o)\n");
  printf("  -h, --help     Print this help\n");
  printf("  -v, --version  Print version info\n");
	
  printf("\n  Some fltk_arg options are:\n");
  //	printf("   -bg2 color\n");
  //	printf("   -bg color\n");
  printf("   -d[isplay] host:n.n   send windows do diplay n on host\n");
  //	printf("   -fg color\n");
  printf("   -g[eometry] WxH+X+Y   set initial window size and location\n");
  //	printf("   -i[conic]\n");
  //	printf("   -k[bd]\n");
  //	printf("   -n[ame] classname\n	");
  //	printf("   -nok[bd]\n");
  //	printf("   -s[cheme] scheme      set window color scheme\n");
  printf("   -t[itle] windowtitle  override window title\n");

  printf("\nconfig file content annotation options:  (can occur multiple times)\n");

  printf("   -ANAME   = read ascii color table file NAME\n");
  printf("   -bstring = add rectangular box to image ([x1,y1],[x2,y2],w,c,I)\n");
  printf("   -cstring = add circle to image ([x,y],c,w,d)\n");
  printf("   -CNAME   = read binary color table file NAME\n");
  printf("   -estring = color table options (i,r,g,b)\n");
  printf("   -Gstring = add color bar to image ([x1,y1],[x2,y2],M)\n");
  printf("   -lstring = add line to image ([x1,y1],[x2,y2],w,c,I)\n");
  printf("   -Sstring = symbol plot ([x1,y1],n,D,w,c)\n");
  printf("   -sstring = image scaling options (z,l,h,s,e)\n");
  printf("   -tstring = add text to image ([x,y],t,a,w,c,m)\n");
  printf("   -Tstring = add text from file name to image ([x,y],t,a,w,c,S)\n");
  printf("   -zstring = add lines specified in ascii input file (F,o,Z,w,c,I)\n");
  printf("   -%%string or -#string = comment (string ignored)\n");
  printf("\n The first letter after the dash is significant and is followed by a string\n");
  printf(" command parameters which have the form L=value.  Multiple parameters are\n");
  printf(" separated by commas. Possible parameters are: (units generally pixels)\n");
  printf("     a = angle (deg)\n");
  printf("     b = blue color table entry values [0..255]\n");
  printf("     c = color [0..255]\n");
  printf("     d = circle diameter in pixels [>0]\n");
  printf("     D = symbol dimension in pixels [>0]\n");
  printf("     e = end color table index [1..255, e > s]\n");
  printf("     F = file name\n");
  printf("     h = high image value\n");
  printf("     i = color table index [0..255] (if multibyte output, <0 switches on 24 bit color input)\n");
  printf("     g = green color table entry values [0..255]\n");
  printf("     I = number of lat/lon increments during line plotting (only valid for n,e) \n");
  printf("     l = low image value\n");
  printf("     m = text\n");
  printf("     M = color bar direction [0..3] (0=l-r,1=r-l,2=b-t,3=t-b)\n");
  printf("     n = symbol id [0..2] (0=+,1=x,2=*)\n");
  printf("     o = line plot flag option (def=0 flag read but ignored, <0 flag not read, >0 match flag>\n");
  printf("     r = red color table entry values [0..255]\n");
  printf("     s = start color table index [0..255]\n");
  printf("     S = starting char number in input name, C=number of chars (=0 length)\n");
  printf("     t = char height\n");
  printf("     X = x dimension size (pixels)\n");
  printf("     Y = y dimension size (pixels)\n");
  printf("     w = line width [>0]\n");
  printf("     z = no data color index [0..255]\n");
  printf("     Z = line plot type Z=0: x,y (def) Z!=0: e,n\n");
	  
  printf("\n  The [x,y] parameters are pixel coordinates but [e,n] (east,north)\n");
  printf("  can be used instead.  x1,y1,x2,y2->e1,n1,e2,n2  Mixing x,y and n,e in same\n");
  printf("  command string is NOT allowed.\n");
  printf("Examples:\n -lx1=10,y1=4,x2=100,y2=300,c=255,w=2 creates a 2 pixel wide line from\n");
  printf("   (10,2) to (100,300) which uses color table entry 255.\n");
  printf(" -sl=-32.5,h=-5.0,s=1,e=254 sets the color table so that entry 1 corresponds\n");
  printf("    to an image value of -32.5 while table index 254 corresponds to -5.0\n");
  printf(" -ei=255,r=255,g=0,b=0 sets color table entry 255 to saturated red\n");
  printf(" -Tx=1,y=1,S=3,C=2 prints characters (3:4) from the SIR file name at (1,1)\n");

}


void parseOptions(int &argc, char **argv, SirWindow *wnd)
{
  int a, c;
  char *opts[]=  // program run options recognized here, options also recognized by fltk
  {
    "-h","--help",
    "-v","--version",
    "-cfg", "-s2g",
    "-dmin", "-dmax",
    "-lon", "-lat",
    "-cta", "-ctb",
    "-smallmem", "-mem",
    "-nomini",
   0, 0
  };
  char *cfg_fname = NULL;
	
  wnd->small_mem=0;
  wnd->sir_file=NULL;

  bool done = false;
  if (argc <= 1) 
    done = true;
  while (!done) {
    done = true;
    for(a=0; opts[a]; a++) { // cycle through the list of args
      //			printf("'%s'\n", argv[1]);
      //			printf("'%s'\n", opts[a]);
      //			printf("%d\n", strcmp(opts[a], argv[1]));
      if(!strcmp(opts[a], argv[1])) { // match
	done = false;
				// printf("'%s'\n", argv[1]);
	switch(a) {
	case 0:case 1:  // help
	  printUsage(argv[0]);
	  exit(0);
	case 2:case 3: // version
	  printf("%s version %s\n", argv[0], versionString());
	  exit(0);
	case 4: // cfg
	case 5: // s2g
	  cfg_fname = argv[2];
	  break;
	case 6: // dmin
	  wnd->usemin=atof(argv[2]);
	  break;
	case 7: // dmax
	  wnd->usemax=atof(argv[2]);
	  break;
	case 8: // lon
	  wnd->lon=atof(argv[2]);
	  break;
	case 9: // lat
	  wnd->lat=atof(argv[2]);
	  break;
	case 10: // cta
	  wnd->LoadColorTable(argv[2], 2, 0);
	  break;
	case 11: // ctb
	  wnd->LoadColorTable(argv[2], 1, 0);
	  break;
	case 12: // smallmem
	case 13: // mem
	  wnd->small_mem=1;
	  break;
	case 14: // nomini
	  wnd->NoMini();
	  break;
	} // case
				
	for(c=1; c<argc-1; c++) // Remove used args from argument list
	  argv[c]=argv[c+1];
	argc--;

	if (!(a == 12 || a == 13 || a == 14)) {  // only for args w/option
	  for(c=1; c<argc-1; c++) // Remove used args from argument list
	    argv[c]=argv[c+1];
	  argc--;
	}				

	break; // get out of for loop
      } // if
    } // for
    if (argc <= 1) 
      done = true;
  } // while
 		

  if(argc>=2) { // next argument must be the file name (i.e. 

    if(argc>=3)  // check for visibility range arguments
      wnd->usemin=atof(argv[2]);
    if(argc>=4)
      wnd->usemax=atof(argv[3]);
    
    wnd->input_fname = argv[1];
    
    if (cfg_fname != NULL)
      readConfig(cfg_fname,wnd->cfg_args,&wnd->ncfg_args,wnd);
      
    //printf("load file '%s'\n",wnd->input_fname);
    int nsx,nsy;
    wnd->LoadFile(wnd->input_fname, &nsx, &nsy);

    //try to set initial window size to match image size if not too big 
    
    if (nsx < 450) 
      nsx=450;     // minimum horizontal window size 
    else
      nsx=nsx+70;			
    if (nsx > 1248) nsx=1248;   //maximum horizontal window size 
    if (nsy < 175) 
      nsy=175;     // minimum vertical window size
    else
      nsy=nsy+70;
    if (nsy > 1024) nsy=1024;   // maximum vertical size
    wnd->Fl_Widget::size(nsx,nsy);
    wnd->imageBox->reset_scroll();  // reset scroll bars
    
    //printf("generate image\n");
    wnd->GenerateImage();
    
    //printf("contining...\n");
    for(c=1; c<argc-1; c++) // Remove used args from arg list
      argv[c]=argv[c+1];
    argc--;
    
    if(argc>=3) {
      for(c=1; c<argc-1; c++) // Remove used args from arg list
	argv[c]=argv[c+1];
      argc--;
    }
    
    if(argc>=4) {
      for(c=1; c<argc-1; c++) // Remove used args from arg list
	argv[c]=argv[c+1];
      argc--;
    }
    
    if(wnd->lon!=999 && wnd->lat!=999) {  // set initial lat/lon for pointer
      int px, py;
      latlon2pix(&(wnd->sh), wnd->lon, wnd->lat, &px, &py);
      wnd->imageBox->zoomx=px;
      wnd->imageBox->zoomy=py;
      wnd->handleChild();
    }
    
    return;
  }
}



SirWindow *mainWindow=NULL;

int main(int argc, char **argv) 
{
  if(Fl::visual(FL_DOUBLE|FL_RGB) != 0) {
    printf("Could not get desired visual (things might not be pretty...)\n");
    printf("visual code=%d\n",Fl::visual(FL_DOUBLE|FL_RGB));
  }

  //	printf("create main windowm\n");
  mainWindow = new SirWindow(600,400,"BYU SIRtool");

  //	printf("parse options\n");
  parseOptions(argc, argv, mainWindow);

  //	printf("show\n");
  mainWindow->show(argc, argv);

  //     	printf("wait\n");
  while(Fl::wait());
  return 0;
}

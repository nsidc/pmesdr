/*****************************************************************************/
/* sirtool.cpp                                                               */
/*	program to display and navigate BYU .SIR image format file           */
/*                                                                           */
/* originally written March 2002 by Vaugh Clayton                            */
/* modified DGL 18 May 2002                                                  */
/*                                                                           */
/* requires fltk library and BYU sir C library code                          */
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/filename.H>
#include <FL/fl_show_colormap.H>


#define VER_MAJOR 1
#define VER_MINOR 0

#include "libsirtool.h"

class SirWindow;

void handleChildren(SirWindow *master);
void printUsage(char *pname);


class SirBox: public Fl_Box
{
	int mx, my;
	int zoomx, zoomy, zoomSize;
	Fl_Scroll *scroller;
	SirWindow *master;

	public:
	unsigned char *img;

	friend class SirWindow;
	friend class SirZoomWindow;
        friend class SirHelpWindow;
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
	
	SirBox(int x, int y, int w, int h, Fl_Scroll *scroll) :Fl_Box(x, y, w, h, NULL)
	{
		scroller=scroll;
		img=NULL;
		zoomx=zoomy=0;
		master=NULL;
		zoomSize=25;
	}
	void reset_scroll()
	{	
		scroller->position(-2, -32);
	}
	
	int handle(int event)
	{
	       // printf("event %d %d\n",Fl::event_button(),Fl::event_key());

     	        if(Fl::event()==FL_KEYUP) {

		  mx = zoomx;
		  my = zoomy;

		  switch(Fl::event_key()) {
		  case 106:  // move left (J)
		    mx--;
		    break;
		  case 108:  // move right (L)
		    mx++;
		    break;
		  case 105:  // move up (U)
		    my++;
		    break;
		  case 109:  // move down (M)
		    my--;
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
					printf("scroller position: %d %d\n",scroller->Fl_Scroll::xposition(),scroller->Fl_Scroll::yposition());
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




class SirZoomWindow: public Fl_Window
{
	SirBox *imageBox;
	unsigned char *zoomData;
	
	public:
	int zoomx, zoomy, xpixels, ypixels;
	int zoomFactor, zoomSize;
	SirZoomWindow(int x, int y, int w, int h, SirBox *img):
		Fl_Window(x, y, w, h, "Zoom")
	{
		zoomx=0; zoomy=0;
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
	
	void draw()
	{	
		fl_draw_image(zoomData, 0, 0, w(), h());
	}
};



class SirHelpWindow: public Fl_Window
{
	Fl_Multiline_Output *textBox;

	public:
	SirHelpWindow(int x, int y, int w, int h):
		Fl_Window(x, y, w, h, "sirtool: help")
	{
	  printf("creating new Fl_Output %d %d %d %d\n",x,y,w,h);
	  
		textBox = new Fl_Multiline_Output(x,y,w,h,"label");
		textBox->value("This is a test");
		
		end();
		//		set_non_modal(); // Always on top, alternative is set_normal()
	}
	
	~SirHelpWindow()
	{
	}	
	
        void text(char *str)
	{
	  textBox->value(str);
	}	

	void draw()
	{ 
	  textBox->value("This is another test");
	  
	  //		fl_draw_image(zoomData, 0, 0, w(), h());
	}
};






class SirWindow: public Fl_Window
{
	int mx, my;
	int zoomx, zoomy;
	SirBox *imageBox;
	Fl_Output *statusBar;
	Fl_Scroll *scroller;
	SirHelpWindow *helpWindow;
	SirZoomWindow *zoomWindow;

	unsigned char palette[768];
	float *sirdata;
	unsigned char *img;
	struct sirheader sh;
  
	float datamin, datamax;
	float usemin, usemax;
	float lon, lat;
	
	friend void parseOptions(int&,char**,SirWindow*);
	public:
	
	SirWindow(int w, int h, const char *title=0): Fl_Window(w,h,title)
	{
		imageBox=NULL;
		statusBar=NULL;
		scroller=NULL;
		helpWindow=NULL;

		Fl_Menu_Item menuitems[] = 
		{
		  {"&File",	0, 0, 0, FL_SUBMENU },
		    {"&About", FL_CTRL + 'a', (Fl_Callback *)cbAbout, this },
		    {"&Open", FL_CTRL + 'o', (Fl_Callback *)cbOpen, this },
		    {"&Export", 0, 0, 0, FL_SUBMENU},
		       {"&ASCII form", FL_ALT + 'a', (Fl_Callback*)cbExportA, this},
		       {"One &Byte/pixel", FL_ALT + 'b', (Fl_Callback*)cbExportB, this},
		  //   {"&GIF", FL_ALT + 'g', (Fl_Callback*)cbExport, this},
		       {0},
		    {"Image &Properties", FL_ALT+FL_Enter, (Fl_Callback*)cbProp, this, FL_MENU_DIVIDER},
		    {"&Quit", FL_CTRL + 'q', (Fl_Callback *)cbQuit, 0 },
		    {0},
		  {"&Data_Range", FL_CTRL+'d', (Fl_Callback*)cbData, this, FL_MENU_DIVIDER},
		  {"&Zoom", 0, 0, 0, FL_SUBMENU },
		    {"&Factor", 0, 0, 0, FL_SUBMENU},
		      {"2X", FL_CTRL+'1', (Fl_Callback*)cbZoom, this},
		      {"4X", FL_CTRL+'2', (Fl_Callback*)cbZoom, this},
		      {"6X", FL_CTRL+'3', (Fl_Callback*)cbZoom, this},
		      {"8X", FL_CTRL+'4', (Fl_Callback*)cbZoom, this},
		      {0},
		    {"&Size", 0, 0, 0, FL_SUBMENU},
		      {"B&igger", FL_CTRL+'[', (Fl_Callback*)cbZoom, this},
		      {"S&maller", FL_CTRL+']', (Fl_Callback*)cbZoom, this},
	              {0},
	            {0},
	          {"&Colortable", 0, 0, 0, FL_SUBMENU},
		    {"&Gray Colortable", 0, (Fl_Callback*)cbGrayColor, this},
	            {"&Open New Colortable", 0, (Fl_Callback*)cbOpenColor, this},
	            {"&Save Colortable", 0, 0, 0, FL_SUBMENU},
	              {"&ASCII form", 0, (Fl_Callback*)cbSaveColor, this},
	              {"&Binary form", 0, (Fl_Callback*)cbSaveColor, this},
	              {0},
	            {"Show &Current Colortable", 0, (Fl_Callback*)cbShowColor, this},
	            {0},
		  {"&Help", FL_CTRL+'h', (Fl_Callback*)cbHelp, this},
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
		
		Fl_Scroll *scroller = new Fl_Scroll(0,30,w,h-60);
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

		zoomWindow->end();
		helpWindow = new SirHelpWindow(1,30,100,100);
		helpWindow->end();

		usemin=99999;
		usemax=-99999;
		lon=999;
		lat=999;
	}

	void LoadFile(const char *fname, int *nsx, int *nsy)
	{
		float float_x, float_y, xs;
		long size;

		//printf("in load file %s\n",fname);		
		sirdata = LoadSIR((char*)fname, &sh);
		//printf("file loaded %d\n",sirdata);
		
		if(sirdata==NULL) {
			fprintf(stderr, "Error allocating SIR memory. \n"); fflush(stderr);
			return;
		}

		float_x = -10000;
		float_y = 10000;
		for(size = sh.nsx * sh.nsy - 1; size>=0; size--) {
			xs = sirdata[size];
			if(xs < float_y) float_y = xs;
			if(xs > float_x) float_x = xs;
		}
		datamin = float_y;
		datamax = float_x;

		if(usemin==99999) usemin = sh.v_min;
		if(usemax==-99999) usemax = sh.v_max;
		
		char buf[1024];
		sprintf(buf, "Image '%s': %dx%d Min: %f Max: %f", fname, sh.nsx, sh.nsy, usemin, usemax);
		statusBar->value(buf);
		
       		Fl_Window::label(fname);
		
		*nsx = sh.nsx;
		*nsy = sh.nsy;

	}

	void GenerateImage()
	{
		float dscale, val;
		long size, i, j, q;
		
		size = sh.nsx * sh.nsy * 3;
		if(img) delete img;
		img = new unsigned char[size];
		q=0;
		dscale = 255.0/(usemax-usemin);
		for(j=sh.nsy-1; j>=0; j--)
		  for(i=0; i<sh.nsx; i++) {
		      val = (sirdata[q++]-usemin)*dscale;
		      if(val<0) val=0;
		      if(val>255) val=255;
		      
		      img[i*3+j*sh.nsx*3+0] = palette[(int)val];
		      img[i*3+j*sh.nsx*3+1] = palette[(int)val+256];
		      img[i*3+j*sh.nsx*3+2] = palette[(int)val+512];
		  }
		
		imageBox->size(sh.nsx, sh.nsy);
		imageBox->img = img;
	}


	void LoadColorTable(const char *fname, int ttype, int skip_error)
	{
		char fullname[1024];
		FILE *f;
		int q, r, g, b;

		filename_expand(fullname, fname);
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

		filename_expand(fullname, fname);
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

		filename_expand(fullname, fname);
		if((f=fopen(fullname, "w"))==NULL) {
			fprintf(stderr, "Could not open file '%s' for writing\n", fullname);
			fclose(f);
			return;
		}
		if(ttype==1) { // Binary
		  dscale = 255.0/(usemax-usemin);

		  for(q=0; q<sh.nsx*sh.nsy; q++) {
		    val = (sirdata[q]-usemin)*dscale;
		    if(val<0) val=0;
		    if(val>255) val=255;
		    byt = (unsigned char) ((int) val);
		    fwrite(&byt, 1, 1, f);
		  }
	        } else {         // ASCII

             	  q=0;
	          for(j=0; j < sh.nsy-1; j++) {
		    for(i=0; i < sh.nsx; i++)
		      fprintf(f,"%f ",sirdata[q++]);
		    fprintf(f,"\n");
		  }
		}
		fclose(f);

		char message[1024];
		sprintf(message,"Export file size: %d x %d\n",sh.nsx,sh.nsy);
		fl_message((const char *) message);
		printf("Export file size: %d x %d\n",sh.nsx,sh.nsy);

	}




	private:
	void hide()
	{
		if(zoomWindow) zoomWindow->hide();
		if(helpWindow) helpWindow->hide();
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
	
	static void cbProp(Fl_Widget *src, void *cbData){((SirWindow*)cbData)->_cbProp();}
	void _cbProp() // File Properties dialog
	{
	        if(sirdata==NULL) { // no file loaded
		  printf("No SIR file loaded\n");
		  return;  
		}		
		print_sir_header(sh);
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

	static void cbShowColor(Fl_Widget *src, void *cbData){ ((SirWindow*)cbData)->_cbShowColor();}
	void _cbShowColor()
	{	
	      (void) fl_show_colormap((Fl_Color) 0);
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
		printUsage("sirtool");
		printf("\nClicking left mouse button outputs cursor position and pixel value at bottom of\nthe window.  The cursor positions is given in SIR pixels (counting from lower-\nleft corner) and lat/lon.  The right mouse button moves the image within\nthe window if the image is larger than the window.\n");
		
		printf("Exporting dumps the pixel values to a specified file as either the floating\npoint ASCII (one ASCII value/pixel) or as scaled bytes (one byte/pixel).\nThe exported pixel order is left-to-right, bottom-to-top (standard SIR order).\n");
		printf("ASCII files are space delimited with a newline after each row.\n");

		printf("\nColor tables have 256 entries.  ASCI files have 256 lines with 3 integer\n(0..255) values (R,G,B) per line.  Binary files have 256 bytes of R, 256 bytes\nof G, and 256 bytes B in that order.\n");
		printf("\nAt start up, the program read the colortable file 'default.ct' from\nthe local directory, if it exists.\n");
		printf("\nSingle pixel keyboard movement: j=left,l=right,i=up,m=down\n");

		helpWindow = new SirHelpWindow(1,40,300,200);
		helpWindow->text("This is test help text");

	}

	

	public:
	void handleChild()
	{
		float lat, lon;
		char buf[1024];
		*buf=0;
		
		zoomx = imageBox->zoomx;
		zoomy = imageBox->zoomy;
		
		pixtolatlon(&sh, zoomx, zoomy, &lon, &lat);
		sprintf(buf, "%d, %d,  Lon: %f Lat: %f  Value: %f", 
			zoomx, zoomy, lon, lat, sirdata[zoomx+zoomy*sh.nsx]);
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
	}

};

void handleChildren(SirWindow *master)
{
	master->handleChild();	
}



void readConfig(char *fname)  // not yet implemented
{
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
	printf("  -dmin n.n     Minimum visible data value\n");
	printf("  -dmax n.n     Maximum visible data value\n");
	printf("  -lon n.n      Initial cursor longitude\n");
	printf("  -lat n.n      Initial cursor latitude\n");
//	printf("  -cfg file     Read options from file\n");
	printf("  -cta file     Use ASCII colortable (R1 G1 B1\\nR2 G2 B2\\n...\n");
	printf("  -ctb file     Use binary colortable (R1..R255,G1..G255,B1..B255\n");
	printf("  -h, --help    Print this help\n");
	printf("  -v, --version Print version info\n");
	
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

}


void parseOptions(int &argc, char **argv, SirWindow *wnd)
{
	int a, c;
	char *opts[]=  // program run options recognized here, options also recognized by fltk
	{
		"-h","--help",
		"-v","--version",
		"-cfg",
		"-dmin", "-dmax",
		"-lon", "-lat",
		"-cta", "-ctb",
		0
	};
	
	while(argc>1) {
		for(a=0; opts[a]; a++) {
//			printf("'%s'\n", argv[1]);
//			printf("'%s'\n", opts[a]);
//			printf("%d\n", strcmp(opts[a], argv[1]));
			if(!strcmp(opts[a], argv[1])) { // match

				// printf("'%s'\n", argv[1]);
				switch(a)
				{
					case 0:case 1:  // help
						printUsage(argv[0]);
						exit(0);
					case 2:case 3: // version
						printf("%s version %s\n", argv[0], versionString());
						exit(0);
					case 4: // cfg
						readConfig(argv[2]);
						break;
					case 5: // dmin
						wnd->usemin=atof(argv[2]);
						break;
					case 6: // dmax
						wnd->usemax=atof(argv[2]);
						break;
					case 7: // lon
						wnd->lon=atof(argv[2]);
						break;
					case 8: // lat
						wnd->lat=atof(argv[2]);
						break;
					case 9: // dmin
						wnd->LoadColorTable(argv[2], 2, 0);
						break;
					case 10: // ctb
						wnd->LoadColorTable(argv[2], 1, 0);
						break;
				}
				for(c=1; c<argc-1; c++) // Remove used args from argument list
					argv[c]=argv[c+1];
				argc--;
				for(c=1; c<argc-1; c++) // Remove used args from argument list
					argv[c]=argv[c+1];
				argc--;
				for(;opts[a]; a++); // Next while
			}
		}
		//printf("initial args parsed\n");
		

		if(argc>=2) { // next argument must be the file name

		        if(argc>=3)  // check for visibility range arguments
				wnd->usemin=atof(argv[2]);
			if(argc>=4)
				wnd->usemax=atof(argv[3]);

			//printf("load file '%s'\n",argv[1]);
			int nsx=600, nsy=400;
			wnd->LoadFile(argv[1], &nsx, &nsy);
			//printf("Image size: %d %d\n",nsx,nsy);

			/* try to set initial window size to match image size if within range */

			if (nsx < 400) 
			  nsx=400;     /* minimum window size */
			else
			  nsx=nsx+70;			
			if (nsx > 1248) nsx=1248;   /* maximum window size */
			if (nsy < 175) 
			  nsy=175;
			else
			  nsy=nsy+70;
			if (nsy > 1024) nsy=1024;
			wnd->Fl_Widget::size(nsx,nsy);
			wnd->imageBox->reset_scroll();  /* reset scroll bars */

			//printf("generate image\n");
			wnd->GenerateImage();

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
}


SirWindow *mainWindow=NULL;

int main(int argc, char **argv) 
{
	if(Fl::visual(FL_DOUBLE|FL_RGB) != 0) {
		printf("Could not get desired visual (things might not be pretty...)\n");
		printf("visual code=%d\n",Fl::visual(FL_DOUBLE|FL_RGB));
	}

	//	printf("create main windowm\n");
	mainWindow = new SirWindow(400,400,"BYU SIRtool");

	//	printf("parse options\n");
	parseOptions(argc, argv, mainWindow);

	//	printf("show\n");
	mainWindow->show(argc, argv);

	//more 	printf("wait\n");
	while(Fl::wait());
	return 0;
}

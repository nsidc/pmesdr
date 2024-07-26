function CETB_viewer_engine(action,filein)
%
% CETB_viewer(input_filename)
%
% matlab utility for viewing sir images with interactive color table,
% and pointing capability
%

%
% function CETB_viewer_engine(action,filein)
%
% matlab utility for interactive image viewing
%
% use 'initialize' to start routine
%

play= 1;
stop=-1;

if nargin<1,
    action='initialize';
end;

if strcmp(action,'initialize'),
    oldFigNumber=watchon;

    if nargin > 1
      filename=filein;
    else
      filename='/mers0/long/sir/data/greeni.sir';
    end;
    
    figNumber=figure( ...
        'Name',filename, ...
        'Color', [.8 .8 .8], ...
        'NumberTitle','off', ...
        'Color', 'white',...
        'Visible','off');
    axes( ...
        'Units','normalized', ...
        'Color', [.8 .8 .8], ...
        'Position',[0.02 0.05 0.75 0.85], ...
	'Visible','off');
    axis([0 1 0 1]);

    %===================================
    % Information for all buttons
    labelColor=[0.8 0.8 0.8];
    top=0.95;
    bottom=0.05;
   
    yInitLabelPos=0.90;
    left=0.825;
    labelWid=0.15;
    labelHt=0.05;
    btnWid=0.15;
    btnHt=0.05;
    % Spacing between the label and the button for the same command
    btnOffset=0.003;
    % Spacing between the button and the next command's label
    spacing=0.05;
    
    %====================================
    % The CONSOLE frame
    frmBorder=0.02;
    yPos=0.05-frmBorder;
    frmPos=[left-frmBorder yPos btnWid+2*frmBorder 0.9+2*frmBorder];
    h=uicontrol( ...
        'Style','frame', ...
        'Units','normalized', ...
        'Position',frmPos, ...
        'BackgroundColor',[0.50 0.50 0.50]);

    %=============================================
 % The view popup button
    btnNumber=1;
    yLabelPos=top-(btnNumber-1)*(btnHt+labelHt+spacing);
    labelStr='Colormap';
    popupStr=str2mat('default', 'hsv', 'hot', 'pink', 'cool', 'bone', 'prism', 'flag', 'gray', 'rand','Poster');
    
    % Generic button information
    ClbkStr = 'CETB_viewer_engine(''color'')';
    labelPos=[left yLabelPos-labelHt labelWid labelHt];
    VpopupHndl=uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'Position',labelPos, ...
        'BackgroundColor',labelColor, ...
        'HorizontalAlignment','left', ...
        'String',labelStr);
    btnPos=[left yLabelPos-labelHt-btnHt-btnOffset btnWid btnHt];
    VpopupHndl=uicontrol( ...
        'Style','popup', ...
        'Units','normalized', ...
        'Position',btnPos, ...
        'String',popupStr, ...
        'Call', ClbkStr);   
    
   %====================================
    % The reshow button
    btnNumber=2;  
    labelStr='Rescale';
    callbackStr='CETB_viewer_engine(''scale'')';
    yLabelPos=top-(btnNumber-1)*(btnHt+labelHt+spacing);
    btnPos=[left yLabelPos-labelHt-btnHt-btnOffset btnWid btnHt];
    infoHndl=uicontrol( ...
        'Style','push', ...
        'Units','normalized', ...
        'position',btnPos, ...
        'string',labelStr, ...
	'call',callbackStr);
    
   %====================================
    % The point button
    btnNumber=3;  
    labelStr='Point';
    callbackStr='CETB_viewer_engine(''pix_locate1'')';
    yLabelPos=top-(btnNumber-1)*(btnHt+labelHt+spacing);
    btnPos=[left yLabelPos-labelHt-btnHt-btnOffset btnWid btnHt];
    infoHndl=uicontrol( ...
        'Style','push', ...
        'Units','normalized', ...
        'position',btnPos, ...
        'string',labelStr, ...
	'call',callbackStr);
    
   %====================================
    % The zoom button
    btnNumber=3.75;  
    labelStr='Zoom';
    callbackStr='zoom';
    yLabelPos=top-(btnNumber-1)*(btnHt+labelHt+spacing);
    btnPos=[left yLabelPos-labelHt-btnHt-btnOffset btnWid btnHt];
    infoHndl=uicontrol( ...
        'Style','push', ...
        'Units','normalized', ...
        'position',btnPos, ...
        'string',labelStr, ...
        'call',callbackStr);

    %====================================
    % The unzoom button
    btnNumber=4.25;  
    labelStr='UnZoom';
    callbackStr='zoom out';
    yLabelPos=top-(btnNumber-1)*(btnHt+labelHt+spacing);
    btnPos=[left yLabelPos-labelHt-btnHt-btnOffset btnWid btnHt];
    infoHndl=uicontrol( ...
        'Style','push', ...
        'Units','normalized', ...
        'position',btnPos, ...
        'string',labelStr, ...
        'call',callbackStr);

    %====================================
    % The INFO button
    %labelStr='Info';
    %callbackStr='CETB_viewer_engine(''info'')';
    %infoHndl=uicontrol( ...
    %    'Style','push', ...
    %    'Units','normalized', ...
    %    'position',[left bottom+2*btnHt+spacing btnWid 2*btnHt], ...
    %    'string',labelStr, ...
    %    'call',callbackStr);

    %====================================
    % The CLOSE button
    labelStr='Close';
    callbackStr='close(gcf)';
    closeHndl=uicontrol( ...
        'Style','push', ...
        'Units','normalized', ...
        'position',[left bottom btnWid 2*btnHt], ...
        'string',labelStr, ...
        'call',callbackStr);
    
    % Uncover the figure
    hndlList=[infoHndl closeHndl VpopupHndl];
    passPrmt.handle=hndlList;
    passPrmt.clmap=[];
    set(figNumber,'Visible','on', ...
        'UserData',passPrmt);
    watchoff(oldFigNumber);
    figure(figNumber);
    shwimg1(filename);
    colormap('gray');
  
  elseif strcmp(action,'color'),
    colorlabels ={'default', 'hsv','hot','pink','cool','bone',...
         'prism','flag','gray','rand','Poster'};
    figNumber=gcf;
    passPrmt=get(figNumber, 'UserData');
    hndlList=passPrmt.handle;
    VpopupHndl=hndlList(3);
    colr=get(VpopupHndl, 'Value');
    if colr==10
     colormap(rand(64,3)); 
    elseif colr==11
     my_cmap;
    else
     colormap(char(colorlabels(colr)));
    end;

elseif strcmp(action,'scale');
    reshow(1);
  
elseif strcmp(action,'pix_locate1');
    pix_locate1(1);

%elseif strcmp(action,'info');
%    helpwin(mfilename);

end;    % if strcmp(action, ...

%%%%  sub functions
    
function pix_locate1(dummy)
%
% interactive position location
%
    global sir_head_viewsir;
    global sir_im_viewsir;
    
    nsx=sir_head_viewsir(3)+1;
    nsy=sir_head_viewsir(4)+1;
    iopt=sir_head_viewsir(1);
    isc=sir_head_viewsir(2);
    
    disp('Left button to locate points, right button to quit');
    button=1;
    while button == 1
      [x y button]=ginput(1);
      y1=nsy-y;
      [lon,lat]=CETB_pix2latlon(x,y1,iopt,isc);
      y1=floor(y);
      x1=floor(x);
      if (x1 > 0) & (x1 < nsy) & (y1 > 0) & (y1 < nsx)
	val=sir_im_viewsir(x1,y1);
      else
	val=0;
      end;
      disp(['Lon ',num2str(lon),'   Lat ',num2str(lat),'  Value ',num2str(val),'  x ',num2str(y1),'  y ',num2str(x1)]);
    end;
    
function shwimg1(filename)
%
% load sir file and display
%
    global sir_head_viewsir;
    global sir_im_viewsir;
    global sir_title1;
    
    disp(['CETB_viewer: Reading file: "',filename,'"']);
    [sir_im_viewsir,iopt,isc]=CETB_load(filename);
    [nsx,nsy]=size(sir_im_viewsir);
    small=min(min(sir_im_viewsir(sir_im_viewsir>50))); % for TB
    large=max(max(sir_im_viewsir));
    sir_title1=escape_underbar(filename);
    sir_head_viewsir=[iopt,isc,nsx,nsy,small,large];

    showsir1(sir_im_viewsir,sir_head_viewsir);
    axis off;
    return;
    
    
    function out=escape_underbar(in)
%
% function out=escape_underbar(in)
% 
% generates "escaped underbar" string out from input string in with
% underbars so that underbars are not interpretted as subscripts
%
% e.g. out="this\_is\_a\_test" from "this_is_a_test"
%

% written 11/17/2007 by DGL at BYU

out=sprintf('%s',in);
ind=find(out=='_');
for k=length(ind):-1:1
  if ind(k)==1
    out=sprintf('\\%s',out(ind(k):end));
  else
    out=sprintf('%s\\%s',out(1:ind(k)-1),out(ind(k):end));
  end
end

function reshow(dummy)
    global sir_im_viewsir;
    
    small=min(min(sir_im_viewsir(sir_im_viewsir>50)));
    large=max(max(sir_im_viewsir));
    disp(['Data min=',num2str(small),'  max=',num2str(large)])
    min_max=[];
    min_max=input('Enter new min, max: "[min max]" ');
    if isempty(min_max),
      min_max=[small large];
    end
    ss=size(min_max);
    if ss(2)<2,
      min_max=[small large];
    end
    large=min_max(2);
    small=min_max(1);
    scale=large-small;
    if scale == 0
      scale = 1;
    end
    scale=64/scale;
    hold on;
    image((sir_im_viewsir'-small)*scale);
    hold off;
    fprintf('calling mycolorbar with %f %f\n',small,large);
    mycolorbar('horiz',small,large);
    
function showsir1(array_in,head,min1,max1)
%
% function showsir1(array_in,head,min1,max1)
%
% function to display sir image 
%
global sir_title1;

if (exist('head','var') == 1)
  small=head(5);
  large=head(6);
  if (large == small)
    small=min(min(array_in));
    large=max(max(array_in));
  end;
  if (exist('min1') == 1)
    small=min1;
    if (exist('max1') == 1)
      large=max1;
    end;
  end;
  title1=sir_title1;
else
  small=min(min(array_in));
  large=max(max(array_in));
  title1=' ';
end;

scale=large-small;
if scale == 0
  scale = 1;
end
scale=64/scale;
min_max=[small large];

image((array_in'-small)*scale);
axis image;
title(title1);
fprintf('calling mycolorbar with %f %f\n',small,large);
mycolorbar('horiz',small,large);
return

function handle=mycolorbar(loc,cmin,cmax)
% handle = mycolorbar(loc,min,max)
%   Display color bar
%
%   loc='vert' appends a vertical color scale to the current
%   axis. loc='horiz' appends a horizontal color scale.
%
%   loc=H places the colorbar in the axes H. The colorbar will
%   be horizontal if the axes H width > height (in pixels).
%
%   COLORBAR without arguments either adds a new vertical color scale
%   or updates an existing colorbar.
%
%   optional cmin,cmax give the labels for the minimum and maximum on scale
%
%   handle = handle to the colorbar axis.

%   Clay M. Thompson 10-9-92
%   Copyright (c) 1984-96 by The MathWorks, Inc.
%   $Revision: 5.18 $  $Date: 1996/10/22 15:10:46 $

%   If called with COLORBAR(H) or for an existing colorbar, don't change
%   the NextPlot property.
changeNextPlot = 1;

if nargin<1, loc = 'vert'; end
ax = [];
t=[1 64];
if nargin >= 1,
    if ishandle(loc)
        ax = loc;
        if ~strcmp(get(ax,'type'),'axes'),
            error('Requires axes handle.');
        end
        units = get(ax,'units'); set(ax,'units','pixels');
        rect = get(ax,'position'); set(ax,'units',units)
        if rect(3) > rect(4), loc = 'horiz'; else loc = 'vert'; end
        changeNextPlot = 0;
    end
    if nargin > 1
      t=[cmin cmax];
    end;
end

h = gca;

%if nargin==0,
    % Search for existing colorbar
    ch = get(gcf,'children'); ax = [];
    for i=1:length(ch),
        d = get(ch(i),'userdata');
        if prod(size(d))==1 & isequal(d,h), 
            ax = ch(i); 
            pos = get(ch(i),'Position');
            if pos(3)<pos(4), loc = 'vert'; else loc = 'horiz'; end
            changeNextPlot = 0;
            break; 
        end
    end
%end

origNextPlot = get(gcf,'NextPlot');
if strcmp(origNextPlot,'replacechildren') | strcmp(origNextPlot,'replace'),
    set(gcf,'NextPlot','add')
end

if loc(1)=='v', % Append vertical scale to right of current plot
    
    if isempty(ax),
        units = get(h,'units'); set(h,'units','normalized')
        pos = get(h,'Position'); 
        [az,el] = view;
        stripe = 0.075; edge = 0.02; 
        if all([az,el]==[0 90]), space = 0.05; else space = .1; end
        set(h,'Position',[pos(1) pos(2) pos(3)*(1-stripe-edge-space) pos(4)])
        rect = [pos(1)+(1-stripe-edge)*pos(3) pos(2) stripe*pos(3) pos(4)];
        
        % Create axes for stripe
        ax = axes('Position', rect);
        set(h,'units',units)
    else
        axes(ax);
    end
    
    % Create color stripe
    n = size(colormap,1);
    image([0 1],t,(1:n)','Tag','TMW_COLORBAR'); set(ax,'Ydir','normal')
    set(ax,'YAxisLocation','right')
    set(ax,'xtick',[])
    
else, % Append horizontal scale to top of current plot
    
    if isempty(ax),
        units = get(h,'units'); set(h,'units','normalized')
        pos = get(h,'Position');
%        stripe = 0.075; space = 0.1;   % original values
	stripe = 0.05; space = 0.05;
        set(h,'Position',...
            [pos(1) pos(2)+(stripe+space)*pos(4) pos(3) (1-stripe-space)*pos(4)])
        rect = [pos(1) pos(2) pos(3) stripe*pos(4)];
        
        % Create axes for stripe
        ax = axes('Position', rect);
        set(h,'units',units)
    else
        axes(ax);
    end
    
    % Create color stripe
    n = size(colormap,1);

    image(t,[0 1],(1:n),'Tag','TMW_COLORBAR'); set(ax,'Ydir','normal')
    set(ax,'ytick',[])
    
end
set(ax,'userdata',h)
set(gcf,'CurrentAxes',h)
if changeNextPlot
    set(gcf,'Nextplot','ReplaceChildren')
end

if nargout>0, handle = ax; end






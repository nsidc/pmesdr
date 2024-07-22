;+========================================================================
; :Author: Mary Jo Brodzik  <brodzik@nsidc-macice.local>  
; :Copyright: (C) <2013> University of Colorado.
; :Version: $Id$
;
; Created 11/22/2013 
; National Snow & Ice Data Center, University of Colorado, Boulder
;
; These routines can be used to make a graphic timeline for operating lifetimes
; of various passive microwave sensors.  For the color palette, they depend on a
; few helper routines that can be found in this repository:
;
; https://bitbucket.org/nsidc/idl_ease_tools
; 
;-========================================================================*/

;+
; Make a plot of PM sensors that overlap AMSR-E/AMSR2
; Include Terra/Aqua/Suomi
;
; :Params:
;    var : in|out, required|optional, type=type
;       description of parameter
;
; :Keywords:
;    keyword1 : in|out, required|optional, type=type
;      Description of keyword
;   
; :Returns: <return info>
;
;-
pro plot_amsre_overlap_sensors, PNG=do_png

  do_png = keyword_set( do_png ) ? 1 : 0

  sensor = ['DMSP-F13 SSM/I', $
            'DMSP-F14 SSM/I', $
            'DMSP-F15 SSM/I>>', $
            'Aqua AMSR-E', $
            'DMSP-F16 SSMIS>>', $
            'DMSP-F17 SSMIS>>', $
            'DMSP-F18 SSMIS>>', $
            'GCOM-W1 AMSR2>>', $
            'Terra MODIS>>', $
            'Aqua MODIS>>', $
            'Suomi VIIRS>>' ]
  ;; From RSS web site
  ;; 
  begin_yyyymmdd = [ '19950503', $ ; f13
                     '19970507', $ ; f14
                     '20000223', $ ; f15
                     '20020601', $ ; AMSR-E
                     '20031001', $ ; f16
                     '20080301', $ ; f17
                     '20100308', $ ; f18
                     '20120508', $ ; AMSR2
                     '20000224', $ ; Terra MODIS begin of data
                     '20020704', $ ; Aqua MODIS begin of data
                     '20111121' ] ; Suomi VIIRS
  end_yyyymmdd = [ '20091119', $ ; f13
                   '20080822', $ ; f14
                   '20170601', $   ; f15
                   '20111004', $   ; AMSR-E
                   '20170601', $   ; f16
                   '20170601', $   ; f17
                   '20170601', $   ; f18
                   '20170601', $   ; AMSR2
                   '20170601', $   ; Terra
                   '20170601', $   ; Aqua
                   '20170601'] ; VIIRS
  ;; Set up PS output
  ease_loadct
  swindow, xs=1000, ys=600
  color = 1
  if do_png then begin
      ease_loadct,/rev
      !p.font = 0
      outfile = '2017_AMSR-E_overlap_sensors_timeline'
      displayps, outfile + '.ps', margin=0.025
      device, /times, font_size=14
      charsize = 1.0
  endif
  
  plot_begin_year = 1995
  plot_end_year = 2018
  plot_begin_jday = julday( 1, 1, plot_begin_year )
  plot_end_jday = julday( 6, 1, plot_end_year )
  jday_range = [ plot_begin_jday, plot_end_jday ]
  xticks=plot_end_year - plot_begin_year
  yrange = [ 0, n_elements( sensor ) + 1 ]
  jdate_plot_config, jday_range, yrange, $
                     color=color, $
                     xticks=xticks, $
                     xtickformat='label_date', $
                     xtickunits='Years', $
                     title='AMSR-E Overlap Sensor History', $
                     xtitle='Year', $
                     ytitle='Satellite/Sensor', $
                     ystyle=1, $
                     yticks=1, $
                     ytickname=[' ',' ']

  for i=0,xticks do begin
      year = i + plot_begin_year
      year_jday = julday( 1, 1, year )
      oplot, [ year_jday, year_jday ], yrange, linestyle=1, color=color
  endfor

  for i=0,n_elements( sensor ) - 1 do begin
      begin_bar_jday = julday( fix( strmid( begin_yyyymmdd[ i ], 4, 2 ) ), $
                               fix( strmid( begin_yyyymmdd[ i ], 6, 2 ) ), $
                               fix( strmid( begin_yyyymmdd[ i ], 0, 4 ) ) )
                               
      end_bar_jday = julday( fix( strmid( end_yyyymmdd[ i ], 4, 2 ) ), $
                             fix( strmid( end_yyyymmdd[ i ], 6, 2 ) ), $
                             fix( strmid( end_yyyymmdd[ i ], 0, 4 ) ) )
      barx = [ begin_bar_jday, end_bar_jday, end_bar_jday, begin_bar_jday ]
      line_index = n_elements( sensor ) - i
      bary = [ line_index + 0.25, line_index + 0.25, $
               line_index - 0.25, line_index - 0.25 ]
      if i lt 7 then begin
          bar_color = 6 + ( i * 7 )
      endif else if i gt 7 then begin
          bar_color = 20 + ( i * 7 )
      endif else begin
          bar_color = 119
      endelse
      polyfill, barx, bary, color=bar_color, /data
      label_offset_days = 90
      label_offset_y = 0.15
      xyouts, begin_bar_jday + label_offset_days, line_index - label_offset_y, $
              sensor[ i ], color=color, /data
  endfor

  if do_png then begin
      displayx
      spawn, "convert -alpha off -density 300 -quality 100 -rotate 270 " $
             + outfile + '.ps ' + outfile + '.png'
      file_delete, outfile + '.ps'
      message, "PostScript saved to " + outfile + '.png', /continue
  endif
  
  return

end

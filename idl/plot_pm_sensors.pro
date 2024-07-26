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
; Make a plot of available PM sensors
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
pro plot_pm_sensors, PNG=do_png

  do_png = keyword_set( do_png ) ? 1 : 0

  sensor = ['Nimbus-7 SMMR', $
            'DMSP-F08 SSM/I', $
            'DMSP-F10 SSM/I', $
            'DMSP-F11 SSM/I', $
            'DMSP-F13 SSM/I', $
            'DMSP-F14 SSM/I', $
            'DMSP-F15 SSM/I', $
            'Aqua AMSR-E', $
            'DMSP-F16 SSMIS>>>>>>>>>>>>>>>>>>>>>>>>>>', $
            'DMSP-F17 SSMIS>>>>>>>>>>>>>>>>>', $
            'DMSP-F18 SSMIS>>>>>>>>>>>>>', $
            'GCOMW1 AMSR2>>>>>>>>', $
            'DMSP-F19 SSMIS', $
            'SMAP Radiometer >>>']
  ;, $
;            'Coriolis WindSat>>', $
;            'GCOM-W1 AMSR2>>' ]
  ;; From RSS web site
  begin_yyyymmdd = [ '19781101', $ ; SMMR
                     '19870709', $ ; f08
                     '19901208', $ ; f10
                     '19911203', $ ; f11
                     '19950503', $ ; f13
                     '19970507', $ ; f14
                     '20000223', $ ; f15
                     '20020601', $ ; AMSR-E
                     '20031001', $ ; f16
                     '20080301', $ ; f17
                     '20100308', $ ; f18
                     '20120518', $ ; AMSR2
                     '20141127', $ ; f19
                     '20150131' ] ; SMAP
;                     '20030106', $ ; WindSat
;                     '20120508' ] ; AMSR2
  end_yyyymmdd = [ '19870731', $ ; SMMR
                   '19911231', $ ; f08
                   '19971114', $ ; f10
                   '20000516', $ ; f11
                   '20091119', $ ; f13
                   '20080822', $ ; f14
                   '20170601', $   ; f15
                   '20111004', $   ; AMSR-E
                   '20230115', $   ; f16
                   '20230115', $   ; f17
                   '20230115', $   ; f18
                   '20230115', $   ; AMSR2
                   '20160201', $   ; f19
                   '20230115']   ; SMAP
;                   '20170601', $   ; WindSat
;                   '20170601' ]    ; AMSR2

  ;; Set up PS output
  ease_loadct
  ;; For all
  swindow, xs=1500, ys=560
  color = 1
  if do_png then begin
      ease_loadct,/rev
      !p.font = 0
      outfile = 'CETB_PM_sensors_timeline'
      displayps, outfile + '.ps', margin=0.025
      device, /times, font_size=13
      charsize = 1.0
  endif
  
  plot_begin_year = 1978
  plot_end_year = 2023
  plot_begin_jday = julday( 1, 1, plot_begin_year )
  plot_end_jday = julday( 6, 1, plot_end_year )
  jday_range = [ plot_begin_jday, plot_end_jday ]
  xticks=plot_end_year - plot_begin_year
  yrange = [ 0, n_elements( sensor ) + 1 ]
;  dummy = label_date(date_format='%Z')
  jdate_plot_config, jday_range, yrange, $
                     color=color, $
                     xticks=xticks, $
                     xtickformat='label_date', $
                     xtickunits='Years', $
;                     title='Passive Microwave Sensor History', $
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

  ; For all, including WindSat and AMSR2
  for i=0,n_elements( sensor ) - 1 do begin
      begin_bar_jday = julday( fix( strmid( begin_yyyymmdd[ i ], 4, 2 ) ), $
                               fix( strmid( begin_yyyymmdd[ i ], 6, 2 ) ), $
                               fix( strmid( begin_yyyymmdd[ i ], 0, 4 ) ) )
                               
      end_bar_jday = julday( fix( strmid( end_yyyymmdd[ i ], 4, 2 ) ), $
                             fix( strmid( end_yyyymmdd[ i ], 6, 2 ) ), $
                             fix( strmid( end_yyyymmdd[ i ], 0, 4 ) ) )
      barx = [ begin_bar_jday, end_bar_jday, end_bar_jday, begin_bar_jday ]
      line_index = n_elements( sensor ) - i
      bary = [ line_index + 0.5, line_index + 0.5, $
               line_index - 0.25, line_index - 0.25 ]
      ;; Use this index in the event that you want to make a
      ;; sensor grey
      if i ne 14 then begin
          bar_color = 6 + ( i * 6 )
      endif else begin
          bar_color = 119
      endelse
      polyfill, barx, bary, color=bar_color, /data
      label_offset_days = 90
      label_offset_y = 0.15
      if i eq n_elements( sensor ) then begin
          align=0.65
      endif else begin
          align=0
      endelse
      xyouts, begin_bar_jday + label_offset_days, line_index - label_offset_y, $
              sensor[ i ], color=color, /data, align=align
  endfor

  ;; !p.font = 1
  ;; device, set_font="-misc-fixed-medium-r-normal--13-100-100-100-c-70-iso8859-1"
  ;; numticks = 5
  ;; labels = ['One', 'Two', 'Three', 'Four', 'Five', 'Six']
  ;; ypos = Replicate(!Y.Window[0] - 0.04, numticks+1)
  ;; xpos = !X.Window[0] + (!X.Window[1] - !X.Window[0]) * $
  ;;        Findgen(numticks + 1) / numticks
  ;; FOR j=0, numticks DO XYOutS, xpos[j], ypos[j], $
  ;;                              labels[j], Alignment=0.0, Orientation=45, /Normal

  if do_png then begin
      displayx
      spawn, "convert -alpha off -density 300 -quality 100 -rotate 270 " $
             + outfile + '.ps ' + outfile + '.png'
      file_delete, outfile + '.ps'
      message, "PostScript saved to " + outfile + '.png', /continue
  endif
  
  return

end

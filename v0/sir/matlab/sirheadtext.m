function head=sirheadtext(head,sensor,title,type,tag,cproc,cdate)
%
%   head=sirheadtext(head,sensor,title,type,tag,cproc,cdate)
%
% modifies contents of text components of sir head array
%
% head:       scaled header information block
% sensor:     40 char string
% title:      80 char string
% type:      138 char string
% tag:        50 char string
% cproc:     100 char string
% cdate:      28 char string
%

if (exist('sensor') == 1)
  s=real(sensor);
  s(41)=0;
  s=s(1:40);
  for i=1:20
    j=(i-1)*2+1;
    head(i+19)=s(j)+s(j+1)*256;
  end;
else
  return;
end;

if (exist('title') == 1)
  s=real(title);
  s(81)=0;
  s=s(1:80);
  for i=1:40
    j=(i-1)*2+1;
    head(i+128)=s(j)+s(j+1)*256;
  end;
else
  return;
end;

if (exist('type') == 1)
  s=real(type);
  s(139)=0;
  s=s(1:138);
  for i=1:69
    j=(i-1)*2+1;
    head(i+57)=s(j)+s(j+1)*256;
  end;
else
  return;
end;

if (exist('tag') == 1)
  s=real(tag);
  s(41)=0;
  s=s(1:40);
  for i=1:20
    j=(i-1)*2+1;
    head(i+169)=s(j)+s(j+1)*256;
  end;
else
  return;
end;

if (exist('cproc') == 1)
  s=real(cproc);
  s(101)=0;
  s=s(1:100);
  for i=1:50
    j=(i-1)*2+1;
    head(i+190)=s(j)+s(j+1)*256;
  end;
else
  return;
end;

if (exist('cdate') == 1)
  s=real(cdate);
  s(29)=0;
  s=s(1:28);
  for i=1:14
    j=(i-1)*2+1;
    head(i+241)=s(j)+s(j+1)*256;
  end;
else
  return;
end;






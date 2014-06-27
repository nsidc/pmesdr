function myfigure(num)
%
% replacement for matlab's figure(num) command
%  - creates a new figure window if not present which is brought forward and
%     made active
%  - if figure exits, this function makes it active (current) but
%     does not raise it to the front
%  
% written DGL at BYU  24 May 1999

H=get(0);  % get root handle
ch=H.Children;
ind=find(ch == num);
if isempty(ind)
  figure(num)
else
  set(0,'CurrentFigure',num);
end

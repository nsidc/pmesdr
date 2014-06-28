%
% check the locations of RSS measurements
%

% two sequential files
fname1='../../../sample_data/RSS_JD061_1997/f13_r10009_tb_v7.dat';
fname2='../../../sample_data/RSS_JD061_1997/f13_r10010_tb_v7.dat';

R1=read_ssmiRSS(fname1);
R2=read_ssmiRSS(fname2);

N1=length(find(R1.ORBIT>0));
N2=length(find(R2.ORBIT>0));

[N1 N2]

r1orb=R1.ORBIT(1:N1)-R1.IORBIT;
r2orb=R2.ORBIT(1:N2)-R1.IORBIT;

ind1=find(r1orb<0);
ind2=find(r1orb>1);
ind3=find(r2orb<1);
ind4=find(r2orb>2);
[length(ind1) length(ind2) length(ind3) length(ind4)]

index=1:max(N1,N2);

myfigure(1)
plot(r1orb,'b')
hold on;plot(r2orb,'r'); hold off
hold on;plot(index(ind1),r1orb(ind1),'c'); hold off
hold on;plot(index(ind2),r1orb(ind2),'g'); hold off
hold on;plot(index(ind3),r2orb(ind3),'c'); hold off
hold on;plot(index(ind4),r2orb(ind4),'g'); hold off
title('Using ORBIT');

myfigure(2)
plot(R1.SC_LON(ind2),R1.SC_LAT(ind2),'b.');
hold on;plot(R2.SC_LON(ind3),R2.SC_LAT(ind3),'r.'); hold off;
hold on;plot(R1.CEL_LON(:,ind2),R1.CEL_LAT(:,ind2),'c.'); hold off;
hold on;plot(R2.CEL_LON(:,ind3),R2.CEL_LAT(:,ind3),'m.'); hold off;

function [m,s,r]=compute_stats(arr)
%
% compute the mean (m), std (s), and rms (r) of the image arr
%
m=mean(arr(:));
s=std(arr(:));
r=sqrt(mean(arr(:).^2));

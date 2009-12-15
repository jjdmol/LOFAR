%
% Utility to examine XST data
%

%cd C:\svnroot\LOFAR\trunk\MAC\Test\Station\gold
close all
% golden XST data
fid = fopen('xst_160.gold','r')
data_gold = fread(fid,'double')
fclose(fid)
corr_gold = reshape(data_gold(1:2:end) + i * data_gold(2:2:end), [32,32])
figure(1)
imagesc(abs(corr_gold))
colorbar
figure(2)
imagesc(angle(corr_gold))
colorbar

% new XST data
fid = fopen('xst.dat','r')
data = fread(fid,'double')
fclose(fid)
corr = reshape(data(1:2:end) + i * data(2:2:end), [32,32])
figure(3)
imagesc(abs(corr))
colorbar
figure(4)
imagesc(angle(corr))
colorbar

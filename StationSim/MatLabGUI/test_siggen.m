%clear all
subband=65;
Nfft=128;
file_name='G:\DATA\everyone\Gerdes\trajectory.txt';
load('data\antenna_signals.mat');
data=AntennaSignals;
FFT_subband(Nfft,subband,subband,data);
load('data\antenna_signals_mod.mat');
%G=colormap(jet(16));
[Phi_theta]=trajectory_reader(file_name);
number_data=size(Phi_theta,1);
b=[];
g=[];
bandwidth=3000000;
freq_res=bandwidth/Nfft;
cfreq=bandwidth/2;
fr=[];
b=angle(SelectedSubBandSignals);
s=SelectedSubBandSignals;
fg=[];
load('data\antenna_config.mat')
load('data\signal_options.mat')
figure
a=[];

t=[-pi:0.01:pi];
freq_shift=(([-Nfft/2:1:Nfft/2-1].' * freq_res + cfreq) / cfreq);
freq_sub=freq_shift(subband);
a=[];
%Phi_theta(1,1)=rfi_phi(1);Phi_theta(1,2)=rfi_theta(1);
for i=1:77
a(:,:,i)=b(1,i)-(freq_shift*2/Nfft*pi*(px(:).* sin(Phi_theta(i,2)) * cos(Phi_theta(i,1)) + py(:).* sin(Phi_theta(i,2)) * sin(Phi_theta(i,1))).').';
end
d=[];
%till number_data
% for time=1:77
%     figure(time)
%     plot(px,py,'r+');
%     hold on
%     for i=1:length(px)
%         d(i,time)=b(i,time)-a(i,subband,time);
%         x=0.04*cos(t)+px(i);
%         y=0.04*sin(t)+py(i);
%         plot(x,y,'b');
%         [Xabs,Yabs]=pol2cart(a(i,subband,time),0.04);
%         plot(px(i)+Xabs,py(i)+Yabs,'r*');
%         [Xfeat,Yfeat]=pol2cart(b(i,time),0.04);
%         plot([px(i) Xfeat+px(i)],[py(i) Yfeat+py(i)],'Color','g');
%     end
%     hold off
%    % mov(time) = getframe(gcf);
% end
% title('Matlab Datagenerator Verification (* is Theorical Phase) (- green) the extracted from the file)');
% d=mod(abs(d),2*pi);
% hold off
% error_pct = 100 * (max(d)-min(d))/(2*pi)
% %movie2avi(mov,'Hello')


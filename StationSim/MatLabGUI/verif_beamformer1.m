file_name1='G:\DATA\everyone\Gerdes\bf_real.txt';
file_name2='G:\DATA\everyone\Gerdes\bf_complex.txt';
load('data/antenna_config.mat');
number_antenna=16;
[out_real]=phase_reader(file_name1);
[out_complex]=phase_reader(file_name2);
result = complex(out_real,out_complex);
B=(ones(number_antenna,1)*unwrap(angle(result(1,1)))-unwrap(angle(result)))/(2*pi);
A=[px.' py.'];
X=B.'/A.';
phi_extracted=atan(X(2)/X(1))
theta_extracted=asin(X(1)/cos(phi))
% b=angle(result);
% for j=1:length(px)
%     a(j,:) = b(1)+(-2*pi*(px(j) * sin(rfi_theta(1)) * cos(rfi_phi(1)) + py(j) * sin(rfi_theta(1)) * sin(rfi_phi(1)))); 
% end
% d=[];
% plot(px,py,'r+');
% hold on
% t=[-pi:0.01:pi];
% for i=1:length(px)
%     d(i)=b(i)-a(i,subband);
%     x=0.02*cos(t)+px(i);
%     y=0.02*sin(t)+py(i);
%     plot(x,y,'b');
%     [Xabs,Yabs]=pol2cart(a(i,subband),0.02);
%     plot(px(i)+Xabs,py(i)+Yabs,'r*');
%     [Xfeat,Yfeat]=pol2cart(b(i),0.02);
%     plot([px(i) Xfeat+px(i)],[py(i) Yfeat+py(i)],'Color','g');
%     hold on 
% end
% title('Matlab Datagenerator Verification (* is Theorical Phase) (- green) the extracted from the file)');
% d=mod(abs(d),2*pi);
% hold off
% error_pct = 100 * (max(d)-min(d))/(2*pi)
function [x,y,z]=polyphase_demo(option,line_style)
%
%   polyphase_demo(option,line_style)
%
%       option      -   'fast'
%                       'real'
%
%       line_style  -   just as plot, leave empty if you do not want lines between subbands
%
%   Example:
%
%       polyphase_demo('fast','r--');
%
%   See also: reordering, partition, prefilter, polyphase_coefficients, polyphase
%
% (C) 2002 Astron, by M.van Veelen

accuracy=2^(-14) ; dBmin=20*log10(accuracy) ;
if nargin < 5 ; line_style = '' ; end ;
if nargin < 1
    option='fast';
end;

% figure(1) ;
subplot(111) ; clf ;
if strcmp(option,'fast') | strcmp(option,'fast-both');  
    K1=16; L1=8 ; M1=K1 ;   N2=16 ; K2=N2; L2=8  ; M2=K2 ;  nms=50000;     dt=10^(-6); 
    x=zeros(1,nms);     t=0:dt:dt*nms;    x=chirp(t,10,nms*dt,0.5/dt);
    [y,z]=polyphase_cascaded(x,K1,L1,M1,K2,L2,M2,N2);
    size(y)
    polyphase_demo_plots(x,y,z,dBmin,1,line_style)
end ;

if strcmp(option,'fast-nc') | strcmp(option,'fast-both'); 
    K1=16; M1=8 ; L1=8 ; N2=16; K2=N2 * floor(K1/M1) ; L2=8 ; M2=K2 ; nms=50000; dt=10^(-6); 
    x=zeros(1,nms);     t=0:dt:dt*nms;    x=chirp(t,10,nms*dt,0.5/dt);
    [y,z]=polyphase_cascaded(x,K1,L1,M1,K2,L2,M2,N2,'keep');
    polyphase_demo_plots(x,y,z,dBmin,1,line_style)
    [y,z]=polyphase_cascaded(x,K1,L1,M1,K2,L2,M2,N2,'cut');
    polyphase_demo_plots(x,y,z,dBmin,2,line_style)
end ;

if strcmp(option,'real-nc') ; 
    K1=64; L1=32 ; M1=48; L1=floor(L1 * K1/M1) ; N2=64; K2=floor(N2*K1/M1/2)*2  ; L2=32 ;M2=K2 ; nms=1000000; dt=15*10^(-9); 
    x=zeros(1,nms);     t=0:dt:dt*nms;    x=chirp(t,10,nms*dt,0.5/dt);
end;
if strcmp(option,'real') ; 
    K1=32; L1=16 ; M1=K1 ;  N2=64;  K2=N2; L2=32 ; M2=K2 ; nms=640000;  dt=15*10^(-9); 
    x=zeros(1,nms);     t=0:dt:dt*nms;    x=chirp(t,10,nms*dt,0.5/dt);
end ;
if strcmp(option,'hard') ; 
    K1=128; L1=32 ; M1=K1 ;  N2=256;  K2=N2; L2=32 ; M2=K2 ; nms=640000;  dt=15*10^(-9); 
    x=zeros(1,nms);     t=0:dt:dt*nms;    x=chirp(t,10,nms*dt,0.5/dt);
end ;
    
% OLD x=chirp(t,50,1,150,'q');
% OLD x=sin(2*pi*t*25) + sin(2*pi*t*50);


% f1=256*64 ; f2=256*16;
% x= [ sin(2*pi*t(1:length(t)/2-1)*f1) , sin(2*pi*t(length(t)/2:length(t))*f2) ] ;

% figure(1)
% K=16;L=8;r=polyphase(x,K,L,K) ; subplot(121) ; imagesc(abs(r)') ; size(r)
% K=16;L=8;r=polyphase(x,K,L,K,'shift') ; subplot(122) ; imagesc(abs(r)') ; size(r)
% subplot(221) ; plot( t,x) ;


function [y,z]=polyphase_cascaded(x,K1,L1,M1,K2,L2,M2,N2,option)

  if nargin<9 ; option='cut' ; end ;
  
  y=polyphase(x,K1,L1,M1) ; y=y ./ max(max(abs(y))) ; %columns are subbands

  if strcmp(option,'cut') ;  dN2=floor((K2-N2)/2) ; else N2=K2; dN2=0; end;

  n_z = floor(length(y)/K2)-L2+1;
  z=zeros( n_z, N2*K1 );
  for k=1:K1
    z_range=N2*(k-1)+[1:N2];
    size(1+dN2:K2-dN2) ;
    size(z_range) ;
    
    %r=polyphase(y(:,k),K2,L2,M2,'centre');
    r=polyphase(y(:,k),K2,L2,M2);
    %z(:,z_range)=r(:,1+dN2:K2-dN2);
    z(:,z_range)=r(:,K2-dN2:-1:1+dN2);
  end;
  z=z./max(max(abs(z))) ;
return ; 

function polyphase_demo_plots(x,y,z,dBmin,row_nr,line_style)
  nr_columns = 3 ; nr_rows = 3 ;
  if nargin < 5 ; row_nr = 1 ; end ;
  K1=length(y')
  K2=length(z')
  subband_demo=1 ;% floor(K1*0.3);
  % plot one subband signal
   plot_nr=1+nr_columns*(row_nr-1) ;  subplot(nr_rows,nr_columns,plot_nr) ; 
   plot( abs(y(:,subband_demo))) ; ylabel( 'Real part of a signal in one subband') ; view(0,90) ;
    % axis( [ 0 length(y) min(min(x)) max(max(x))] )                 

  plot_nr=2+nr_columns*(row_nr-1) ;  subplot(nr_rows,nr_columns,plot_nr) ; 
  imagesc( max(dBmin, 20*log10(abs(y)')) ) ; ylabel( 'Subband after one stage of filtering') ; view(0,90) ;               

  plot_nr=3+nr_columns*(row_nr-1) ;  subplot(nr_rows,nr_columns,plot_nr) ; 
  imagesc( max(dBmin, 20*log10(abs(z)') ) ) ; 
  ylabel( 'Subband after the second stage of filtering') ; view(0,90) ;
  % axis( [ 0 length(y) min(min(z)) max(max(z))] )                 
  % plot sub band regions
  if ~strcmp(line_style,'') ;     hold ; 
    for n=0:K1
        plot( [ 1 length(z) ] , [ 1+n*K2 1+n*K2 ],line_style ); 
    end; hold ;
  end ;

%  subplot(344) ; mesh( max(dBmin,20*log10(abs(z(:,K2*(subband_demo)+[1:K2])))')) ; view(0,90) ;
%                 ylabel( 'The channels within this one subband') ; %

  colormap gray;

return ; 

if 0
  X1=fft(x(partition(length(x),K1)')) ; X1=X1./max(max(abs(X1)));
  subplot(333); mesh(max(dBmin,20*log10(abs(X1)))) ; view(0,90) ; 
                     ylabel( 'FFT result') ; 

  X2=fft(x(partition(length(x),K1*K2)')) ;  X2=X2./max(max(abs(X2)));
  subplot(339); mesh(max(dBmin,20*log10(abs(X2)))) ; view(0,90) ; 
                   ylabel( 'FFT result') ; 
end;
                   
function polyphase_demo(option,line_style)
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

if nargin < 1
    option='fast';
end;

figure(1) ;
if strcmp(option,'fast') ; K1=16; L1=8 ;  K2=16; L2=8 ; nms=50000; end ;
if strcmp(option,'real') ; K1=64; L1=32 ;  K2=64; L2=32 ; nms=1000000; end ;

% x=chirp(t,50,1,150,'q');
    dt=10E-6;
    t=0:dt:dt*nms;
    x=chirp(t,10,nms*dt,0.5/dt);
% x=sin(2*pi*t*25) + sin(2*pi*t*50);

% figure(1)
% K=16;L=8;r=polyphase(x,K,L,K) ; subplot(121) ; imagesc(abs(r)') ; size(r)
% K=16;L=8;r=polyphase(x,K,L,K,'shift') ; subplot(122) ; imagesc(abs(r)') ; size(r)
% subplot(221) ; plot( t,x) ;


y=polyphase(x,K1,L1,K1) ; %columns are subbands
subplot(221) ; imagesc( abs(y)' ) ; ylabel( 'Subband after one stage of filtering') ; 

subband_demo=floor(K1*0.3);

% plot one subband signal
subplot(223) ; plot( real(y(:,subband_demo))) ; ylabel( 'Real part of a signal in one subband') ; 
axis( [ 0 length(y) min(min(x)) max(max(x))] ) 

size(y)
n_z = floor(length(y)/K2)-L2+1;
z=zeros( n_z, K2*K1 );
for k=1:K1
    z_range=K2*(k-1)+[1:K2];
    z(:,z_range)=polyphase(y(:,k),K2,L2,K2,'centre');
end;

subplot(222) ; imagesc( abs(z)' ) ; ylabel( 'Subband after the second stage of filtering') ; 
% plot sub band regions
if nargin > 1
    hold ; 
    for n=0:K1
        plot( [ 1 length(z) ] , [ 1+n*K2 1+n*K2 ],line_style ); 
    end;
    hold ;
end ;

subplot(224) ; imagesc( abs(z(:,K2*(subband_demo)+[1:K2]) )') ; ylabel( 'The channels within this one subband') ; 



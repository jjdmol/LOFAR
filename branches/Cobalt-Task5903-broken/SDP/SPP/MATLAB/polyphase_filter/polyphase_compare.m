function [ctk,ctd,ra,rm] = polyphase_compare(option,n_tests,scale);
%
%   polyphase_compare(option)
%
%       option  :   'simple'    just compare one run with the filterbank
%                   'channels'   compare the implementations by varying the number of channnels
%                   'lenght'     compare the implementations by varying the number of datapoints
%                   'full'       run all the tests
%                   'snap'       technicians only ;-)
%                   'snapall'    technicians only ;-)
%
%       n_tests :   number of tests to run. 
%
%       scale   :   'normal'
%                   'loglog'    (default)
%
%   Don't be too eager with the n_tests ... it's 2^(n_tests+offset) datapoints/channels!
%   Start with a value like 4, 8 should be computable within reasonable time on 1GHz CPUs.
%
%   See also: reordering, partition, prefilter, polyphase_coefficients, polyphase
%
% (C) 2002 Astron, by M.van Veelen
%

format compact;
if nargin < 1 ; option='simple' ; end ;
if nargin < 2 ; n_tests = 4 ; end ;
if nargin < 3 ; scale='loglog' ; end ;

n_figures = 0 ;

ra=[]; rm=[];

nms=2^14;
% x=chirp(t,50,1,150,'q');
    dt=0.001;
    t=0:dt:dt*nms;
    x=chirp(t,5,1,10,'q');
% x=sin(2*pi*t*25) + sin(2*pi*t*50);


K=16; L=8 ;
if strcmp(option,'simple') | strcmp(option,'snap') | strcmp(option,'full') | strcmp(option,'snapall') 
	tic;
	p=alex_polyphase_init(L,K);
	ra=alex_polyphase(x,16,32,32,p,K,L, floor(length(x)/L) );
	t=toc; ctk=t; 
	figure(1); subplot(321) ; 
	imagesc(sqrt(real(ra).^2+imag(ra).^2)') ; colormap gray ; xlabel('time') ; ylabel('channel'); 
	title(['Previous implementation. Computing time ',num2str(t),' [s]'] ) ;
	
	tic
	rm=polyphase(x,K,L,K) ; % M is set to K
	t=toc;  ctd=t ;
	n_figures=n_figures+1 ; figure(n_figures); subplot(322) ; 
	imagesc(sqrt(real(rm).^2+imag(rm).^2)') ; colormap gray ; xlabel('time') ; ylabel('channel'); 
	title(['Previous implementation. Computing time ',num2str(t),' [s]'] ) ;
	
	if strcmp(option,'snap');
        n_figures=n_figures+1 ; figure(n_figures); 
        imagesc(sqrt(real(rm).^2+imag(rm).^2)') ; colormap gray ; xlabel('time') ; ylabel('channel'); 
        title(['Previous implementation. Computing time ',num2str(ctk),' [s]'] ) ;
        n_figures=n_figures+1 ; figure(n_figures); 
        imagesc(sqrt(real(rm).^2+imag(rm).^2)') ; colormap gray ; xlabel('time') ; ylabel('channel'); 
        title(['Previous implementation. Computing time ',num2str(ctd),' [s]'] ) ;
        n_figures=n_figures+1 ; figure(n_figures); 
        mesh(sqrt(real(rm-ra).^2+imag(rm-ra).^2)') ; colormap gray ; xlabel('time') ; ylabel('channel'); 
        title(['Difference. Computing time ',num2str(ctk-ctd),' [s]'] ) ; shading flat; 
	end ;
end ;

if strcmp(option,'full') | strcmp(option,'channels') | strcmp(option,'snapall') ;

    display('running test one ...') ;
    offset=3 ;
    ctk=zeros(n_tests,2);
    for n=1:n_tests;
        K=2^(n+offset-1);
        L=K/2;
        tic; p=alex_polyphase_init(L,K); ra=alex_polyphase(x,16,32,32,p,K,L,1E10); ctk(n,1)=toc ;
        tic; rm=polyphase(x,K,L,K) ; ctk(n,2)=toc ;
        clear('p') ; clear('rm') ;clear('ra') ;
    end;

    if strcmp(scale,'loglog') | strcmp(option,'snapall');
        if strcmp(option,'snapall') ; n_figures=n_figures+1 ; figure(n_figures); subplot(111) ;else ; subplot(313) ; end ;
        loglog( 2.^(offset+[1:length(ctk)]), ctk(:,1),'-', 2.^(offset+[1:length(ctk)]), ctk(:,2),'--' ) ; 
        legend('Previous version', 'New version',2) ; 
        ylabel('time [s]'); xlabel('Number of channels')
        title (['Computing time as function of number of channels (',int2str(length(x)),' data points) L=K/2' ]) ; 
        axis([ 2^(offset-1),2^(offset+n_tests+1), min(min(ctk))*0.9, max(max(ctk))*1.1 ]) ;
        if strcmp(option,'snapall') ; saveas(gcf,['performance_channels_logscale'],'bmp'); end ;
    end ;
    if strcmp(scale,'normal') | strcmp(option,'snapall');
        if strcmp(option,'snapall') ; n_figures=n_figures+1 ; figure(n_figures); subplot(111) ;else ; subplot(313) ; end ;
        plot( 2.^(offset+[1:length(ctk)]), ctk(:,1),'-', 2.^(offset+[1:length(ctk)]), ctk(:,2),'--' ) ; 
        legend('Previous version', 'New version',2) ; 
        ylabel('time [s]'); xlabel('Number of channels')
        title (['Computing time as function of number of channels (',int2str(length(x)),' data points) L=K/2' ]) ; 
        axis([ 2^(offset)*0.9,2^(offset+n_tests)*1.1, min(min(ctk))*0.9, max(max(ctk))*1.1 ]) ;
        if strcmp(option,'snapall') ; saveas(gcf,['performance_channels'],'bmp'); end ;
    end;
    
end ;

if strcmp(option,'length') | strcmp(option,'full') | strcmp(option,'snapall') 
    display('running test two ...') ;
    ctd=zeros(n_tests,2);
    dt=0.001; K=64; L=32 ;
    offset=log2(K*L);
    for n=1:n_tests;
        nms=2^(n+offset-1);
        t=0:dt:dt*nms;
        x=chirp(t,5,1,10,'q');
        tic; p=alex_polyphase_init(L,K); ra=alex_polyphase(x,16,32,32,p,K,L,1E10); ctd(n,1)=toc ;
        tic; rm=polyphase(x,K,L,K) ; ctd(n,2)=toc ;
        clear('p') ; clear('rm') ;clear('ra') ; clear('x') ;
    end;

    if strcmp(scale,'loglog') | strcmp(option,'snapall');
        if strcmp(option,'snapall') ; n_figures=n_figures+1 ; figure(n_figures); subplot(111) ;else ; subplot(313) ; end ;
        loglog( 2.^(offset+[1:length(ctd)]), ctd(:,1),'-', 2.^(offset+[1:length(ctd)]), ctd(:,2),'--' ) ; 
        legend('Previous version', 'New version',2) ; ylabel('time [s]'); xlabel('Number of samples')
        title (['Computing time as function of number of samples (K=',int2str(K),';L=',int2str(L), ')']) ; 
        axis([ 2^(offset-1),2^(offset+n_tests+1), min(min(ctd))*0.9, max(max(ctd))*1.1 ]) ;
        if strcmp(option,'snapall') ; saveas(gcf,['performance_datasize_logscale'],'bmp'); end ;
    end ;
    if strcmp(scale,'normal') | strcmp(option,'snapall');
        if strcmp(option,'snapall') ; n_figures=n_figures+1 ; figure(n_figures); subplot(111) ;else ; subplot(313) ; end ;
        plot( 2.^(offset+[1:length(ctd)]), ctd(:,1),'-', 2.^(offset+[1:length(ctd)]), ctd(:,2),'--' ) ; 
        legend('Previous version', 'New version',2) ; ylabel('time [s]'); xlabel('Number of samples')
        title (['Computing time as function of number of samples (K=',int2str(K),';L=',int2str(L), ')']) ; 
        axis([ 2^(offset)*0.9,2^(offset+n_tests)*1.1, min(min(ctd))*0.9, max(max(ctd))*1.1 ]) ;
        if strcmp(option,'snapall') ; saveas(gcf,['performance_datasize'],'bmp'); end ;
    end;
end ;

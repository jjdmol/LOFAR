function plot_eigenfigures

load('data/output_options.mat');
load('data/bf_options.mat');
load('data/rfi_eigen.mat');

if signal_eigen_acm
    if useACM
        dirpath='data';
        load([dirpath '/temp2.mat']);

        figure(9)
        if usePASTd
            subplot(1,2,1)
        end
        plot( (log10(-sort(-abs(diag(Evalue))))), '+')
        title('Eigen values of ACM')
%        axis([0 100 -5 3]);
    end
    if usePASTd
        figure(9)
        dirpath='data';
        load([dirpath '/temp1.mat']);
        if useACM
            subplot(1,2,2)
        end
        plot((log10(-sort(-abs(diag(Evalue))))), '+')   
        xlabel('index eigenvalue')
        ylabel('^{10}log(eigenvalue)')
        title('Eigen values of ACM from PASTd')
%        axis([0 100 -5 3]); 
        hold off
    end;
end


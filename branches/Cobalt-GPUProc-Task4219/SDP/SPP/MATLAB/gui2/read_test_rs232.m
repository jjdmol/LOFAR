function read_test_rs232(update,test_number)

if test_number==1 
    
    file_name='OutputRS232.txt';
    start_point=1;
    end_point=start_point+127;
    
    cd demo_fpga;                         % get the output  .txt file
    dos('demo_getdata_atod0_inp.bat');

    [index,realdata,channel_real,real_data,channel_ima,imag_data]=textread(file_name,'%8d %8d %8d %8d %8d %8d');     % read the file
    
    cd ..                                   % come back to the application folder
      
    spectr=spectrum(realdata);
    max_pow=max(spectr);
    spectr=spectr/max_pow;    
        
    power_data=real_data.*real_data+imag_data.*imag_data;                             
    max_pow_board=max(power_data);
    power_data=power_data/max_pow_board;

    figure;
    subplot(3,1,1);
    plot(realdata(1:4096));
    title('Input signal versus time in micro seconds','FontSize',10);
    set(gca,'XTick',0:819:4096)
    set(gca,'XTickLabel',{'00','20','40','60','80','100'})
    subplot(3,1,2);
    plot(10*log(1e-8+power_data(1:128)));
    title('Normalised Power density spectrum (dB) without integration vs Frequency (MHz), first Nyquist zone','FontSize',10);
    set(gca,'XTick',0:16:128)
    set(gca,'XTickLabel',{'00','2.5','05','7.5','10','12.5','15','17.5','20'})
    subplot(3,1,3);
    plot(10*log(spectr));
    title('Normalised Power density spectrum (dB) after 16 integrations vs Frequency (MHz), first Nyquist zone','FontSize',10);
    set(gca,'XTick',0:16:128)
    set(gca,'XTickLabel',{'00','2.5','05','7.5','10','12.5','15','17.5','20'})
end;

if test_number==2 
    
    file_name='OutputRS232.txt';
    start_point=1;
    end_point=start_point+127;
    
    cd demo_fpga;                         % get the output  .txt file
    dos('demo_getdata_atod1_inp.bat');

    [index,realdata,channel_real,real_data,channel_ima,imag_data]=textread(file_name,'%8d %8d %8d %8d %8d %8d');     % read the file
    
    cd ..                                   % come back to the application folder
      
    spectr=spectrum(realdata);
    max_pow=max(spectr);
    spectr=spectr/max_pow;    
        
    power_data=real_data.*real_data+imag_data.*imag_data;                             
    max_pow_board=max(power_data);
    power_data=power_data/max_pow_board;

    figure;
    subplot(3,1,1);
    plot(realdata(1:4096));
    title('Input signal versus time in micro seconds','FontSize',10);
    set(gca,'XTick',0:819:4096)
    set(gca,'XTickLabel',{'00','20','40','60','80','100'})
    subplot(3,1,2);
    plot(10*log(1e-8+power_data(1:128)));
    title('Normalised Power density spectrum (dB) without integration vs Frequency (MHz), first Nyquist zone','FontSize',10);
    set(gca,'XTick',0:16:128)
    set(gca,'XTickLabel',{'00','2.5','05','7.5','10','12.5','15','17.5','20'})
    subplot(3,1,3);
    plot(10*log(spectr));
    title('Normalised Power density spectrum (dB) after 16 integrations vs Frequency (MHz), first Nyquist zone','FontSize',10);
    set(gca,'XTick',0:16:128)
    set(gca,'XTickLabel',{'00','2.5','05','7.5','10','12.5','15','17.5','20'})
end;


if test_number==3

        
        if update==0
            
            cd demo_fpga;                                                         % get the output  .txt file
            
            file_name='TestOutput.txt';

            dos('demo_getdata_stats_atod0_inp.bat');
            [index,real_part,imag_part,mean_snapshot]=textread(file_name,'%d %d %d %d');             % read the file
            dos('del TestOutput.txt');
            
            mean_matrix=[10*log10(abs(mean_snapshot))']';
            
            figure;
            subplot(2,1,1);
            plot(10*log10(abs(mean_snapshot)));
            subplot(2,1,2);
            imagesc(mean_matrix);
            colormap(gray)
            xlabel('Mean snapshots');
            ylabel('Frequency (MHz)');
            set(gca,'YTick',0:32:128)
            set(gca,'YTickLabel',{'00','05','10','15','20'})
                        
            save matrix;
            cd ..
        end; 
        
        if update==1
            
            for i=1:100,
                           
                cd demo_fpga;                                                         % get the output  .txt file
                
                file_name='TestOutput.txt';
                
                load matrix;
                clear new_mean_snapshot;
                dos('demo_getdata_stats_atod0_inp.bat');
                [index,real_part,imag_part,new_mean_snapshot]=textread(file_name,'%d %d %d %d');             % read the file
                dos('del TestOutput.txt');
                
                mean_matrix=[mean_matrix';(10*log10(abs(new_mean_snapshot)))']';
                
                close figure 1;
                figure;
                subplot(2,1,1);
                plot(10*log10(abs(new_mean_snapshot)));
                subplot(2,1,2);
                imagesc(mean_matrix);
                colormap(gray)
                xlabel('Mean snapshots');
                ylabel('Frequency (MHz)');
                set(gca,'YTick',0:32:128)
                set(gca,'YTickLabel',{'00','05','10','15','20'})
                
                save matrix;
                cd ..
            end;
        end;
        
  




%         if update==0
%             
%             cd demo_fpga;                                                         % get the output  .txt file
%             
%             file_name='TestOutput.txt';
% 
%             dos('demo_getdata_stats_atod0_inp.bat');
%             [index,mean_snapshot,var_snapshot,skew_snapshot,kurt_snapshot]=textread(file_name,'%d %d %d %d %d');             % read the file
%             dos('del TestOutput.txt');
%             
%             mean_matrix=[10*log10(abs(mean_snapshot))']';
%             var_matrix=var_snapshot';
%             skew_matrix=skew_snapshot';
%             kurt_matrix=kurt_snapshot';
%             
%             figure;
%             
%             subplot(2,2,1);
%             imagesc(mean_matrix);
%             colormap(gray)
%             xlabel('Mean snapshots');
%             ylabel('Frequency (MHz)');
%             set(gca,'YTick',0:32:128)
%             set(gca,'YTickLabel',{'00','05','10','15','20'})
%             
%             subplot(2,2,2);
%             imagesc(var_snapshot);
%             colormap(gray)
%             xlabel('Variance snapshots');
%             ylabel('Frequency (MHz)');
%             set(gca,'YTick',0:32:128)
%             set(gca,'YTickLabel',{'00','05','10','15','20'})
%             
%             subplot(2,2,3);
%             imagesc(skew_snapshot);
%             colormap(gray)
%             xlabel('Skewness snapshots');
%             ylabel('Frequency (MHz)');
%             set(gca,'YTick',0:32:128)
%             set(gca,'YTickLabel',{'00','05','10','15','20'})
% 
%             subplot(2,2,4);
%             imagesc(kurt_snapshot);
%             colormap(gray)
%             xlabel('Kurtosis snapshots');
%             ylabel('Frequency (MHz)');
%             set(gca,'YTick',0:32:128)
%             set(gca,'YTickLabel',{'00','05','10','15','20'})
%             
%             save matrix;
%             cd ..
%         end; 
%         
%         if update==1
%             
%             for i=1:15,
%                             
%                 cd demo_fpga;                                                         % get the output  .txt file
%                 
%                 file_name='TestOutput.txt';
%                 
%                 load matrix;
%                 clear new_mean_snapshot;
%                 clear new_var_snapshot;
%                 clear new_skew_snapshot;
%                 clear new_kurt_snapshot;
%                 
%                 dos('demo_getdata_stats_atod0_inp.bat');
%                 [index,new_mean_snapshot,new_var_snapshot,new_skew_snapshot,new_kurt_snapshot]=textread(file_name,'%d %d %d %d %d');             % read the file
%                 dos('del TestOutput.txt');
%                 
%                 mean_matrix=[mean_matrix';(10*log10(abs(new_mean_snapshot)))']';
%                 var_matrix=[var_matrix';new_var_snapshot']';
%                 skew_matrix=[skew_matrix';new_skew_snapshot']';
%                 kurt_matrix=[kurt_matrix';new_kurt_snapshot']';
%                 
%                 close figure 1;
%                 figure;
%                 
%                 subplot(2,2,1);
%                 imagesc(mean_matrix);
%                 colormap(gray)
%                 xlabel('Mean snapshots');
%                 ylabel('Frequency (MHz)');
%                 set(gca,'YTick',0:32:128)
%                 set(gca,'YTickLabel',{'00','05','10','15','20'})
%                 
%                 subplot(2,2,2);
%                 imagesc(var_snapshot);
%                 colormap(gray)
%                 xlabel('Variance snapshots');
%                 ylabel('Frequency (MHz)');
%                 set(gca,'YTick',0:32:128)
%                 set(gca,'YTickLabel',{'00','05','10','15','20'})
%                 
%                 subplot(2,2,3);
%                 imagesc(skew_snapshot);
%                 colormap(gray)
%                 xlabel('Skewness snapshots');
%                 ylabel('Frequency (MHz)');
%                 set(gca,'YTick',0:32:128)
%                 set(gca,'YTickLabel',{'00','05','10','15','20'})
%                 
%                 subplot(2,2,4);
%                 imagesc(kurt_snapshot);
%                 colormap(gray)
%                 xlabel('Kurtosis snapshots');
%                 ylabel('Frequency (MHz)');
%                 set(gca,'YTick',0:32:128)
%                 set(gca,'YTickLabel',{'00','05','10','15','20'})
%                 
%                 save matrix;
%                 cd ..
%                 
%             end;
%         end;











        
end;
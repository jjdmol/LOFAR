function eigensystem(xq,dirpath,px,py,Beam_Phi_theta,Phi_theta,EVD_Bool,SVD_Bool,PASTd_Bool,VecPosition,indicator)

    load([dirpath 'data/output_options.mat'])
    load([dirpath 'data/subband_options.mat']);
    load([dirpath 'data/bf_options.mat']);
    
    phi=Beam_Phi_theta(1,VecPosition)
    theta=Beam_Phi_theta(2,VecPosition)
    phi_RFI=Phi_theta(1,VecPosition)
    theta_RFI=Phi_theta(2,VecPosition)
    
    NumberOfAntennas=length(px);
    LookingDirection  = steerv(px,py,phi,theta);  % Direction of looking
    
    fprintf('\tACM\n');
    
    if EVD_Bool
             
             fprintf('\t\tEVD\n');
             %Applying ACM
             [ EvectorEVD, EvalueEVD ] = acm(xq,NumberOfAntennas,Snapshot_Buffer, Beam_Phi_theta(1,Snapshot_Buffer),Beam_Phi_theta(2,Snapshot_Buffer), px, py, 1, 1,Forgetting);
             
             Evalue=EvalueEVD ;
             Evector=EvectorEVD;
             
             if plotEVD
             figure(9)
             plot(-sort(-diag(EvalueEVD)),'*b');
             end
                % else
          %      [ EvectorEVD, EvalueEVD ] = acm(AntennaSignals, NumberOfAntennas, snapshot_number,  ...
           %         rfi_phi, rfi_theta, px, py, rfi_number, testRFI);
                %   end
          %  if (rfi_strat==2)
          
            %Number of sources detection
             if Threshold_radio==1
             fprintf('\t\tThresholding Eigen Values EVD - Estimation of the number of interfering sources\n');
                 rfi_sources=length(find(-sort(diag(-Evalue))>Threshold_value));
             fprintf(['\t\tThreshold :' num2str(Threshold_value) ' - Sources detected : ' num2str(rfi_sources) '\n']);
             else    
             fprintf('\t\tMDL - Estimating the number of interfering sources\n');
             for i = 1:size(EvectorEVD,3);
             rfi_sources(i,:)=mdl(-sort(diag(-squeeze(Evalue(:,:,i)))),NumberOfAntennas,Snapshot_Buffer);
             end  
             end
             rfi_sources=20;
             save([dirpath 'data/temp.mat'], 'Evalue','Evector','rfi_sources');
       end;
        
        
        
       if SVD_Bool
            fprintf('\t\tSVD\n');
            %[W,d]=svd(AntennaSignals);
            [W,d]=svd(xq);
            EvectorSVD=W;
            EvalueSVD=d;
            if (rfi_strat==2)
                sz=size(diag(EvalueSVD),1);
                d=diag(EvalueSVD)/EvalueSVD(sz/2,sz/2);
            Evalue=EvalueSVD ;
            Evector=EvectorSVD;  
            
            %Number of sources detection
             if Threshold_radio==1
             fprintf('\t\tThresholding Eigen Values SVD - Estimation of the number of interfering sources\n');
             rfi_sources=length(find(-sort(diag(-Evalue))>Threshold_value));
             fprintf(['\t\tThreshold :' num2str(Threshold_value) ' - Sources detected : ' num2str(rfi_sources) '\n']);
             else 
             fprintf('\t\tMDL using SVD - Estimating the number of interfering sources\n'); 
             rfi_sources=mdl(-sort(diag(-EvalueSVD)),NumberOfAntennas,snnr);
             end
             rfi_sources=20
            end;
           
       
            if plotSVD
            figure(9)
            plot(-sort(-diag(EvalueSVD)),'*b');
            end
            
            save([dirpath 'data/temp.mat'],'Evector','Evalue','rfi_sources');
        end
        
         %Declaration a changer
 
        
         if PASTd_Bool
            fprintf('\t\tPASTd\n');
            if indicator==1
            load([dirpath 'data/temp.mat'],'Evector','Evalue','rfi_sources');
            else 
            load([dirpath 'data/temp2.mat'],'Evector','Evalue','rfi_sources');
            end
            % transform the antennasignals to correspond to the format PASTd expects         
      
            [ W, d, numsnaps, vecconv] = pastdev(xq.',Evalue,Evector,size(xq,2),1, BetaPASTd, NumberOfAntennas);
            EvectorPASTd=W;
            EvaluePASTd=d;
           
            %if (rfi_strat==2)
                %fprintf('\t\tMDL - Estimating the number of interfering sources\n');
                %rfi_sources=mdl((diag(d)),NumberOfAntennas,snnr); %abs of the eigenvalue....
            %end;
            
            useACM=1;
            %Plot the weigth convergence
            if plotPASTd==1
            figure(10)
            subplot(1,2+useACM,1)
            title('Eigen Vector convergence of PASTd')
            plot(vecconv);
            end
            
%             if (signal_eigen_acm)
%                 s = figure(9);
%                 cur_scr = get(s,'Position');
%                 root_scr = get(0,'Screensize');
%                 set(s,'Position',[root_scr(1), cur_scr(2), root_scr(3), cur_scr(4)]);
%                 subplot(1,2+useACM,2)
%                 plot( (log10(-sort(-abs(diag(EvaluePASTd))))), '+')% Attention au moins
%                 xlabel('index eigenvalue')
%                 ylabel('^{10}log(eigenvalue)')
%                 title('Eigen values of ACM from PASTd')
%                 axis([0 100 -5 4]); 
%                 hold off
%             end; 
            
               
            
            if (match ~= 0)
                B=EvectorPASTd(:,1:rfi_sources);
                fprintf('\t\t\tMatching algorithms\n');
                 switch match
                 case 1  % match steer vector
                    fprintf('\t\t\t\tMatch steer vector.\n');
                    patend=180;
                    patstep=4;
                    a=[-1*patend:patstep:patend]*pi/180;
                     patend=90;
                    b=[-1*patend:patstep/2:patend]*pi/180;
                    for ki=1:rfis
                        cmax=0;
                        imax(ki)=-90*pi/180;
                        imay(ki)=-90*pi/180;
                        for i=1:length(a)
                            for j=1:length(b)
                                c=abs(B(:,ki)'*steerv(px,py,a(i),b(j)));
                                if c > cmax;
                                    imax(ki)=a(i);
                                    imay(ki)=b(j);
                                    cmax=c;
                                end
                            end
                        end
                        intx(ki)=imax(ki);
                        inty(ki)=imay(ki);
                        fprintf('\t\t\t\tRFI source found at (%d,%d)...\n',intx,inty);
                    end
                    end
                end;
                
                if (match > 0)
                    save([dirpath '/temp2.mat'], 'EvaluePASTd','EvectorPASTd','intx','inty','B');
                else
                    Evalue=EvaluePASTd;
                    Evector=EvectorPASTd;
                    save([dirpath 'data/temp2.mat'], 'Evalue','Evector','rfi_sources');
                end
                
         Evalue=EvaluePASTd;
         Evector=EvectorPASTd;
         save([dirpath 'data/temp2.mat'], 'Evalue','Evector','rfi_sources');       
        end
  
    
 
    
        

    % Plot the beampattern using which of the three Eigen systems?
    % please note that PASTd and SVD have the most significant eigen vector 
    % first, while the EVD algorithm produces it last.
     [EValue,index]=sort(-diag(Evalue));
     EValue=-EValue;
     EVector=Evector(:,index(1:rfi_sources));
   
     
     
     WeightVector=[];
     fprintf('\tAWE\n');
    % Get the number of interfering sources from data
    for i=1:size(rfi_sources,1)
        %WeightVector(:,i) = awe(squeeze(EVector(:,1:rfi_sources(i,:),i)),squeeze(EValue(:,1:rfi_sources(i,:),i)),rfi_sources, NumberOfAntennas,LookingDirection,1);
        WeightVector = awe(EVector, EValue,rfi_sources, length(px),LookingDirection,1);
    end
   %WeightVector = awe(EVector, EvalueEVD,rfi_sources, length(px),LookingDirection,1);
    % save these values to file
    save([dirpath 'data/rfi_eigen.mat'],'Evector','Evalue','LookingDirection','WeightVector', 'rfi_sources');
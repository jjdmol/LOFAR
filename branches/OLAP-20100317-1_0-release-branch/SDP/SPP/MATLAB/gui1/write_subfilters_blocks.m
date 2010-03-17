% write_subfilters_blocks(N,L,h,quant_h,B) creates intel .hex files in the folder
% of the application. These files are required for the hardware implementation
% of the subfilters.
function write_subfilters_blocks(N,L,h,quant_h,B)

blocks=B;
for i=1:blocks,                                    % for each block
    
    file_number=num2str(i);                        % get the number of the block
    file_name=strcat('sub',file_number,'.hex');    % create a file name including this number
    syst_file_number=fopen(file_name,'w');         % get the corresponding number given by the system for this file
    coefficients=downsample(h,blocks,i-1);         % get the coefficients to write in this file
        
    % here we write only half of the coeeficients, because the hardware design takes the
    % symmetry of the filter h into account
    for j=1:N/blocks*L/2,                          % for each coefficient index
        
        % the index is coded with four hexadecimal digit, and the checksum required bytes (2characters)
        % therefore it's necessary to divide the index into two parts
        index=j-1;                                   % current index
        index1 = floor(index/256);                 % part of the index corresponding to the higher byte
        index2 = index-index1*256;                 % part of the index corresponding to the lower byte
        
        
        scale_coeff(j)=abs(coefficients(j))*4;
        
        if coefficients(j)<0                            % if the coefficient is negative
            scale_coeff(j)= -scale_coeff(j)+2^(quant_h+2);  % add 2^quant_h to the coefficient, so that the MSB is 1
        end;                                            % then the coefficient will be negative in the design 
        
        
        
        % here each coefficient is coded over 6 hexadecimal digit (3 bytes)
        % for the checksum, it's necessary to extract these 3 bytes: coeff1, coeff2, coeff3        
        coeff1  = floor(scale_coeff(j)/65536);
        result1 = scale_coeff(j)-coeff1*65536;
        coeff2  = floor(result1/256);
        coeff3  = result1-coeff2*256;
        
        % the checksum is a two digit hexadecimal that is the two's compliment of the sum of all
        % bytes in the line, including the prefix, which is 3 in our case because the coefficient
        % is coded with 3 bytes.
        checksum=2+index1+index2+coeff1+coeff2+coeff3;
        while checksum>255
            checksum=checksum-256;
        end;
        if checksum>0
            checksum=256-checksum;
        end;
        
        % here we concatenate all the strings in this order:
        % ':' , 'prefix(02)' , 'index' , 'record type(00)' , 'coefficient' , 'checksum'. 
        % then the concatenation is written in a .hex file
        conv_hex=strcat(':02',dec2hex(index,4),'00',dec2hex(scale_coeff(j),4),dec2hex(checksum,2));
        fprintf(syst_file_number,'%s\n',conv_hex);

    end;
    
    % once all the coefficients have been written in a file, it is neccessary to close this file
    % before, we have to write the End Of File string:
    conv_hex=':00000001ff';
    fprintf(syst_file_number,'%s\n',conv_hex);     % write the end of file string
    fclose(syst_file_number);                      % close the file
    
    % here we copy the .hex files in the filters folder
    file_copied = copyfile(file_name,'/filters');
    delete(file_name);
    
end;
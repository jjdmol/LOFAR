function [px,py]=reader_array(Filename_array,NAntennas)
stop=0;
fid=fopen(Filename_array,'r');
while (stop<2)
    line=fgetl(fid);
    resp=strfind(line,'[');
    if length(resp)>0
        stop=stop+1;
        if stop==1
        px =fscanf(fid,'%g',[1 NAntennas]);
        else
        py =fscanf(fid,'%g',[1 NAntennas]);
        end
    end
end
fclose(fid)
function  [AntennaSignals] = reader_data(file)

    fid = fopen(file,'r');
    stop=0;
    while (stop==0)
    line=fgetl(fid);
    resp4=strfind(line,'x');
    if length(resp4)>0
    Npositions=str2num(line(1:resp4-1));
    Nantennas=str2num(line(resp4+1:length(line)));
    end
    resp0=strfind(line,'[');
    if length(resp0)~=0
        stop=1;
    end
    end
   fac=1;
   real= fscanf(fid,'%f',[Nantennas Npositions]);
   stop=0;
while (stop==0)
    line=fgetl(fid);
    resp0=strfind(line,'[');
    if length(resp0)~=0
        stop=1;
    end
end
   imag= fscanf(fid,'%f',[Nantennas Npositions]);
   
   AntennaSignals=real(:,1:floor(size(real,2)/fac))+sqrt(-1)*imag(:,1:floor(size(imag,2)/fac));
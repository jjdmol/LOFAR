function reader1_dsp(file_name)

global x1;
global x2;

global nb_spectre;
global size_spectre;

MEGA = 1e6;
%size_spectre is always 1024 for Kleewein's DSP
size_spectre = 8192;

%open .dsp file for reading in big-endian mode



%close file handler
fclose (FID);

nb_spectre =round( size_b/taille_spectre);

s = reshape(b, taille_spectre, nb_spectre);
s(1:16,:) = [];

if e.nb_voies == 3
    ch1(1:size_spectre,1:nb_spectre) = s(1:size_spectre,1:nb_spectre);
    ch2(1:size_spectre,1:nb_spectre) = s(1025:2048,1:nb_spectre);
    x1(1:size_spectre,1:nb_spectre) = s(2049:3072,1:nb_spectre);
    x2(1:size_spectre,1:nb_spectre) = s(3073:4096,1:nb_spectre);

else
    ch1(1:size_spectre,1:nb_spectre) = s(1:size_spectre,1:nb_spectre);
end

clear b; clear s;

clear ch1;
clear ch2;
clear x1;
clear x2;
clear e;
clear nb_spectre;
clear size_spectre;

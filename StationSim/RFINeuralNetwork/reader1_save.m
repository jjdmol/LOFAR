global ch1;
global ch2;
global x1;
global x2;
global e;
global nb_spectre;
global size_spectre;

str = datestr(date,0);

[newfile,newpath] = uiputfile('*.mat','Sauvegarde de l''environnement')

file_name = [newpath newfile]

save(file_name ,'ch1','ch2','x1','x2','e', 'nb_spectre', 'size_spectre');

close all;
clear all;
load -ascii el_nino_PC.mat
x=el_nino_pc;
ncol = 1;

D = x(:,ncol); ncol = ncol + 1;

N1 =  x(:,ncol); ncol = ncol + 1;
N2 =  x(:,ncol); ncol = ncol + 1;
N3 =  x(:,ncol); ncol = ncol + 1;
N4 =  x(:,ncol); ncol = ncol + 1;

Tx1 =  x(:,ncol); ncol = ncol + 1;
Tx2 =  x(:,ncol); ncol = ncol + 1;
Tx3 =  x(:,ncol); ncol = ncol + 1;
Tx4 =  x(:,ncol); ncol = ncol + 1;

Ty1 =  x(:,ncol); ncol = ncol + 1;
Ty2 =  x(:,ncol); ncol = ncol + 1;
Ty3 =  x(:,ncol); ncol = ncol + 1;
Ty4 =  x(:,ncol); ncol = ncol + 1;

epsi = 0.15;

Box1 = [ -90+epsi+360       -80-epsi+360    0-epsi   -5+epsi]; Coul1 = [1 0   0  ];
Box2 = [ -90+epsi+360       -80-epsi+360   -5-epsi  -10+epsi]; Coul2 = [1 0.5 0.5];
Box3 = [-150+epsi+360       -90-epsi+360    5-epsi   -5+epsi]; Coul3 = [0 0   1  ];
Box4 = [(160-360)+epsi+360 -150-epsi+360    5-epsi   -5+epsi]; Coul4 = [0 0.5 0.5];

Annee = floor(D/100);
Mois = D - (Annee * 100);

% Compte les Labels en absices
iLab = find(Mois == 1);
nLab = length(iLab);

XLabels = cell(length(iLab),1);
for ii = 1:length(iLab)
	iAnnee = Annee(iLab(ii));
	XLabels{ii,1} = sprintf('%02d', iAnnee);
end
XTicks = iLab;

figure
subplot(3,1,1)
plot(N1,'Color',Coul1)
hold on
plot(N2,'Color',Coul2)
plot(N3,'Color',Coul3)
plot(N4,'Color',Coul4)
axis([0 450 -2 4]);
title('Temp')
legend('NINO1','NINO2','NINO3','NINO4',2);
box on, grid on
set(gca,'XTick',XTicks, 'XTickLabel',XLabels);

subplot(3,1,2)
plot(Tx1,'Color',Coul1)
hold on
plot(Tx2,'Color',Coul2)
plot(Tx3,'Color',Coul3)
plot(Tx4,'Color',Coul4)
axis([0 450 -60 40]);
title('Tx')
box on, grid on
set(gca,'XTick',XTicks, 'XTickLabel',XLabels);


subplot(3,1,3)
plot(Ty1,'Color',Coul1)
hold on
plot(Ty2,'Color',Coul2)
plot(Ty3,'Color',Coul3)
plot(Ty4,'Color',Coul4)
axis([0 450 -60 40]);
title('Ty')
box on, grid on
set(gca,'XTick',XTicks, 'XTickLabel',XLabels);


figure
alpha = 1;
MinY = -4;
MaxY =  4;
StepY = 1;

Var = eval(['N' num2str(1)]);
iOk = find(Var ~= -9999);

Annee = floor(D(iOk)/100);
Mois = D(iOk) - (Annee * 100);

% Compte les Labels en absices
iLab = find(Mois == 1);
nLab = length(iLab);

XLabels = cell(length(iLab),1);
for ii = 1:length(iLab)
	iAnnee = Annee(iLab(ii));
	XLabels{ii,1} = sprintf('%02d', iAnnee);
end
XTicks = iLab;

for ii=1:4
	subplot(4,1,ii)
	
	Var = eval(['N' num2str(ii)]);
	iOk = find(Var ~= -9999);
	Var = Var(iOk);
	M = mean(Var(:));
	S = std(Var(:));
	plot((Var - M) / (alpha * S))
	hold on
	lax = axis;
	axis([lax(1) lax(2) MinY MaxY]); set(gca,'YTick',MinY:StepY:MaxY)
	title(['NINO ' num2str(ii) ', N = ' num2str(length(Var)) ' et Alpha = ' num2str(alpha)])
    box on, grid on
	set(gca,'XTick',XTicks, 'XTickLabel',XLabels);

end

%suptitle('Variables de Temperature NINOx Normalises [(S-m)/(alpha * e)]')

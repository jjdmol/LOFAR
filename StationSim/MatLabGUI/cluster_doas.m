function [clusters] = cluster_doas(doas,cut_off)

% pdist, linkage, clust

t=clusterdata(doas.',cut_off) ;

Beampattern_plot( doas(:,find(t==1))) ;
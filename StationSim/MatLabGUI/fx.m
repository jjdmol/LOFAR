function y=fx(data, N, NBINT)
y=zeros(1,N/2);
%h=ones(N,1);
h=hamming(N);
for i=1:1:NBINT
    temp=abs(fft(data((1+(i-1)*N):(N*i)).*h',N)); 
    y=temp(1:N/2)+y(1:N/2);
end

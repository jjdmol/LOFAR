function [Phi_theta]=interpolation(D,Nfft,freq,Total_Time)
freq
Nfft
D
Phi=[];
Time=[];
Theta=[];
Phi_theta=[];
for i=1:size(D,1)-1
    
    Delta_phi=D(i+1,1)-D(i,1);
    Delta_theta=D(i+1,2)-D(i,2);
    
    if (Delta_phi==0|Delta_theta==0)
    if Delta_phi==0
        Speed_phi=0;
        Speed_theta=D(i+1,3);
    end
    if Delta_theta==0
        Speed_theta=0;
        Speed_phi=D(i+1,3);
    end
    else
        alpha=atan(Delta_theta/Delta_phi);
        Speed_phi=cos(alpha)*D(i+1,3);
        Speed_theta=sin(alpha)*D(i+1,3);
    end

    if i==1
        phi_start=D(1,1);
        theta_start=D(1,2);
        time_start=D(1,4);
    else
        phi_start=phi_arrival;  
        theta_start=theta_arrival;
        time_start=time_end;
    end
    time_step=(D(i+1,4)-D(i,4))*Total_Time;
    interval=ceil(freq*time_step/Nfft);
    phi_arrival=phi_start+Speed_phi*time_step;
    theta_arrival=theta_start+Speed_theta*time_step;
    time_end=time_start+time_step;
    
 if (Delta_phi==0)
     phi=phi_start*ones(1,interval);
     theta=theta_start:(theta_arrival-theta_start)/(length(phi)-1):theta_arrival;
     time=time_start:(time_end-time_start)/(length(phi)-1):time_end;
 end
 if (Delta_theta==0)
     theta=theta_start*ones(1,interval);
      phi=phi_start:(phi_arrival-phi_start)/(length(theta)-1):phi_arrival;
       time=time_start:(time_end-time_start)/(length(theta)-1):time_end;
 if ((Delta_theta==0)&(Delta_phi==0))
     theta=theta_start*ones(1,interval);
     phi=phi_start*ones(1,interval);
     time=time_start:(time_end-time_start)/(length(theta)-1):time_end;
 end
 else
    if (abs(Speed_phi)>abs(Speed_theta))
        phi=phi_start:(phi_arrival-phi_start)/interval:phi_arrival;
        theta=theta_start:(theta_arrival-theta_start)/(length(phi)-1):theta_arrival;
        time=time_start:(time_end-time_start)/(length(phi)-1):time_end;
    else
        theta=theta_start:(theta_arrival-theta_start)/interval:theta_arrival;
        phi=phi_start:(phi_arrival-phi_start)/(length(theta)-1):phi_arrival;
        time=time_start:(time_end-time_start)/(length(theta)-1):time_end;
  end
end
    Phi=[Phi;phi.'];
    Theta=[ Theta; theta.'];
    Time=[Time;time.'];
    Phi_theta=[phi.' theta.' time.'];
end
Phi_theta=[Phi Theta Time/Total_Time];
figure
plot(Time,Phi_theta(:,2),'b')
hold on
plot(Time,Phi_theta(:,1),'r')
title(' Plot of the behaviour of Phi, theta according to input matrix');
xlabel('Time ')
ylabel('radian')
legend('Theta','Phi')
hold off
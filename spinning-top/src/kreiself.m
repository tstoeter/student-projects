% kreiself.m
% ordinary differential equations describing the behaviour of a spinning top
% its motions are visualized in kreisel.m

function dydt = topf(t,y)
    dydt = zeros(6,1);  % zero out return vector

    % required constants
    m = 45.81; % g
    g = 981; % cm * s^(-2)
    d = 7.5; % cm
    a = 214.73;
    c = 85.89;
    
    % some renaming for better readability of ode's
    phi = y(1); theta = y(2); psi = y(3);
    phid = y(4); thetad = y(5); psid = y(6);
    
    % the second order differential equations
    psidd = ((c-2*a)*psid*thetad) / (a*tan(theta)) + (c*phid*thetad) / (a*sin(theta));
    thetadd = (a*psid^2*cos(theta)-c*psid^2*cos(theta)-c*phid*psid+m*g*d)*sin(theta)/a;
    phidd = psid*thetad*sin(theta)-psidd*cos(theta);
    
    % assign first and second order derivatives to return vector
    dydt = [phid;thetad;psid;phidd;thetadd;psidd];
    
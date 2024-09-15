% kreisel.m
% simulation of a spinning top
% solves equations in kreiself.m which describe the top's motion
% and displays the results in an animation

clear all;

% set time interval and step size
t0 = 0; te = 2; s = 0.01;

% numerically integrate our given set of ode's from kreiself.m
% initial values: theta = 2.8°, phid = 30 rots per sec
[ts, ys] = ode45('kreiself', [t0:s:te], [0; pi/64; 0; 30*2*pi; 0; 0]);

% print out the results to the prompt
[ts, ys]

% making the axes and scaling fix
axis normal;
axis([-2 2 -2 2 0 2]);
hold on; grid on;

% create our cone's vertices
[X Y Z] = cylinder([0 0.5]);
% and get a handle for the cone surface for transformations
hcone = surf(X,Y,Z, 'FaceColor','red','EdgeColor','black');
camlight left; lighting phong;  % also make the graphics look nice ;)

% create hgtransform object and bind it to the cone surface
tcone = hgtransform;
set(hcone, 'Parent', tcone);

% animate for every time step, n refers to number of frames
[m, n] = size([t0:s:te]);

% iterate over solution array to generate each animation frame
for t = 1:n
    % create transformation matrix and apply it
    R = makehgtform('zrotate', ys(t,1), 'yrotate', ys(t,2), 'zrotate', ys(t,3));
    set(tcone, 'Matrix', R);
    % draw phase plot of top down view
    plot(sin(ys(1:t,2)) .* cos(ys(1:t,3)), sin(ys(1:t,2)) .* sin(ys(1:t,3)), 'b')
    % capture current frame
    M(t) = getframe;
end

% plot angles and their speed
%plot(ts, ys(:,1), ts, ys(:,2), ts, ys(:,3))
%plot(ts, ys(:,4), ts, ys(:,5), ts, ys(:,6))

% show the animation twice at real time (?)
movie(M, 1, 1/s);
movie2avi(M, 'kreisel.avi', 'compression', 'none');

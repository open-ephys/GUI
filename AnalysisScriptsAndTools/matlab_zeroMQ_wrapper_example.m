url = 'tcp://localhost:5556'; % or tcp://192.168.50.96:5556' , if GUI runs
% on another machine...

handle = zeroMQwrapper('StartConnectThread',url);

fndllZeroMQ_Wrapper('Send',handle ,'ClearDesign');


fndllZeroMQ_Wrapper('Send',handle ,'NewDesign Go_Left_Right');
fndllZeroMQ_Wrapper('Send',handle ,'AddCondition Name GoRight TrialTypes 1 2 3');
fndllZeroMQ_Wrapper('Send',handle ,'AddCondition Name GoLeft TrialTypes 4 5 6');

tic;while toc < 2; end;

for k=1:10
    % indicate trial type number #2 has started
    fndllZeroMQ_Wrapper('Send',handle ,'TrialStart 2');  
    tic;while toc < 0.2; end;
    % indicate that trial has ended with outcome "1"
    fndllZeroMQ_Wrapper('Send',handle ,'TrialEnd 1');
    tic;while toc < 1; end;
end

fndllZeroMQ_Wrapper('CloseThread',handle);

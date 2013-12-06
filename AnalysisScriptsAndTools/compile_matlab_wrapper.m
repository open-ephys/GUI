% compile 0MQ matlab wrapper 

% 1. make sure mex is setup properly and a compiler is available
mex -setup

GUIfolder = 'C:\Users\Shay\Documents\GitHub\GUI';

headerFolder = [GUIfolder, '/Resources/ZeroMQ/include'];
if strcmp(computer,'PCWIN')
    libFolder = [GUIfolder, '/Resources/ZeroMQ/lib_x86'];
elseif strcmp(computer,'PCWIN64')
    libFolder = [GUIfolder, '/Resources/ZeroMQ/lib_x64'];
elseif strcmp(computer,'GLNX86')
    error('to be implemented...');
elseif strcmp(computer,'GLNXA64')
    error('to be implemented...');
elseif strcmp(computer,'MACI64')
    error('to be implemented...');
end

libraryName = 'libzmq-v110-mt-3_2_2';
% 2. compile
eval(['mex zeroMQwrapper.cpp -I',headerFolder,' -L',libFolder,' -l',libraryName] )
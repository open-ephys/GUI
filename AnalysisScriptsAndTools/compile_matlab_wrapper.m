% compile 0MQ matlab wrapper 

% 1. make sure mex is setup properly and a compiler is available
mex -setup

%GUIfolder = 'C:\Users\Shay\Documents\GitHub\GUI';
GUIfolder = '/home/jsiegle/Programming/GUI/';

headerFolder = [GUIfolder, '/Resources/ZeroMQ/include'];
if strcmp(computer,'PCWIN')
    libFolder = [GUIfolder, '/Resources/ZeroMQ/lib_x86'];
    libraryName = 'libzmq-v110-mt-3_2_2';
    cppFile = 'zeroMQwrapper.cpp';
elseif strcmp(computer,'PCWIN64')
    libFolder = [GUIfolder, '/Resources/ZeroMQ/lib_x64'];
    libraryName = 'libzmq-v110-mt-3_2_2';
elseif strcmp(computer,'GLNX86') || strcmp(computer,'GLNXA64')
    libFolder = '/usr/local/lib';
    libraryName = 'zmq';
    cppFile = 'zeroMQNixwrapper.cpp';
elseif strcmp(computer,'MACI64')
    error('to be implemented...');
end


% 2. compile
eval(['mex ' cppFile ' -I',headerFolder,' -L',libFolder,' -l',libraryName] )
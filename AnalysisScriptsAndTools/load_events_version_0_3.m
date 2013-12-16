function [data, timestamps, info] = load_open_ephys_data(filename)

%
% [data, timestamps, info] = load_open_ephys_data(filename)
%
%   Loads continuous, event, or spike data files into Matlab.
%
%   Inputs:
%
%     filename: path to file
%
%
%   Outputs:
%
%     data: either an array continuous samples, a matrix of spike waveforms,
%           or an array of event channels
%
%     timestamps: in seconds
%
%     info: structure with header and other information
%
%
%
%   DISCLAIMER:
%
%   Both the Open Ephys data format and this m-file are works in progress.
%   There's no guarantee that they will preserve the integrity of your
%   data. They will both be updated rather frequently, so try to use the
%   most recent version of this file, if possible.
%
%

%
%     ------------------------------------------------------------------
%
%     Copyright (C) 2013 Open Ephys
%
%     ------------------------------------------------------------------
%
%     This program is free software: you can redistribute it and/or modify
%     it under the terms of the GNU General Public License as published by
%     the Free Software Foundation, either version 3 of the License, or
%     (at your option) any later version.
%
%     This program is distributed in the hope that it will be useful,
%     but WITHOUT ANY WARRANTY; without even the implied warranty of
%     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%     GNU General Public License for more details.
%
%     <http://www.gnu.org/licenses/>.
%
filename = 'C:\Users\shayo\Documents\GitHub\GUI\Builds\VisualStudio2012\Debug64\bin\2013-12-10_13-03-06\all_channels.events';
filetype = filename(max(strfind(filename,'.'))+1:end); % parse filetype

fid = fopen(filename);
%filesize = getfilesize(fid);

% constants
NUM_HEADER_BYTES = 1024;

% constants for pre-allocating matrices:
MAX_NUMBER_OF_SPIKES = 1e6;
MAX_NUMBER_OF_RECORDS = 1e4;
MAX_NUMBER_OF_CONTINUOUS_SAMPLES = 10e6;
MAX_NUMBER_OF_EVENTS = 1e6;
SPIKE_PREALLOC_INTERVAL = 1e6;

%-----------------------------------------------------------------------
%------------------------- EVENT DATA ----------------------------------
%-----------------------------------------------------------------------
index = 0;

hdr = fread(fid, NUM_HEADER_BYTES, 'char*1');
eval(char(hdr'));
info.header = header;

if (isfield(info.header, 'version'))
    version = info.header.version;
else
    version = 0.0;
end


SESSION = 10;
TIMESTAMP = 0;
TTL = 3;
SPIKE = 4;
NETWORK = 7;

timestampCounter = 1;
fseek(fid,NUM_HEADER_BYTES,-1)
afSpikeTimeHardware = [];
afSpikeTimeSoftware= [];

TimstampSoftware = [];
TimestampHardware = [];
SessionStartInd = [];
while (1)
    index=index+1;
    eventTy= fread(fid, 1, 'uint8=>uint8');
    if isempty(eventTy)
        break;
    end
    eventType(index)  = eventTy;
    eventSize(index) = fread(fid, 1, 'uint16=>uint16');
    switch eventType(index)
        case SESSION 
            startStop = fread(fid,1,'uint8=>bool'); % start = 1
            recordNumber = fread(fid,1,'uint16=>uint16');
            softwareTS = fread(fid,1,'int64=>int64');
           
            SessionStartInd = [SessionStartInd,length(TimstampSoftware)];
           
        case TIMESTAMP
            TimstampSoftware(timestampCounter) = fread(fid,1,'int64=>int64');
            TimestampHardware(timestampCounter) = fread(fid,1,'int64=>int64');
            timestampCounter=timestampCounter+1;
            
             
        case TTL
            risingEdge = fread(fid,1,'uint8=>bool'); % start = 1
            softwareTS = fread(fid,1,'int64=>int64');
            hardwareTS = fread(fid,1,'int64=>int64');
        case SPIKE
            
            softwareTS = fread(fid,1,'int64=>int64');
            hardwareTS = fread(fid,1,'int64=>int64');
            sortedID = fread(fid,1,'int16=>int16');
            electrodeID = fread(fid,1,'int16=>int16');
            numChannels = fread(fid,1,'int16=>int16');
            numPts = fread(fid,1,'int16=>int16');
            if (eventSize(index) > 8+8+2+2+2+2)
                fread(fid,numPts*numChannels,'uint16=>uint16');
            end
    
        case NETWORK
             str = fread(fid,eventSize(index)-8,'char=>char')'; % start = 1
             softwareTS = fread(fid,1,'int64=>int64');

        otherwise
            fprintf('Unknown event. Skipping %d\n',eventSize(index));
            fseek(fid,eventSize(index),0);
    end
    
end
fclose(fid);


figure;
plot(TimstampSoftware-TimstampSoftware(1),TimestampHardware-TimestampHardware(1),'.');
% robust fit has trouble with very large values.
% so we remove the offset and then regress
[SyncCoeff]=robustfit(TimstampSoftware(:)-TimstampSoftware(1),TimestampHardware(:)-TimestampHardware(1))

% Software to hardware mapping jitter:
JitterMS = 1e3*(((TimstampSoftware-TimstampSoftware(1))* SyncCoeff(2) + SyncCoeff(1)) -  (TimestampHardware-TimestampHardware(1))) / header.sampleRate;
mean(JitterMS)

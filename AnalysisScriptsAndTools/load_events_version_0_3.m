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
filename = 'E:\Data\Doris\Electrophys\Intan\140113_154158_Debug\all_channels.events';
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

fseek(fid,NUM_HEADER_BYTES,-1);


%% Event types definition
SESSION = 10;
TIMESTAMP = 0;
TTL = 3;
SPIKE = 4;
NETWORK = 7;
EYE_POSITION = 8;


%% main loop (non optimized)
timestampCounter = 1;
acNetworkEvents = cell(0);
networkCounter = 1;
ttlCounter = 1;
TimstampSoftware = [];
TimestampHardware = [];
SessionStartInd = [];
networkEventsTS = [];
spikeCounter = 1;
TTLs = zeros(0,4);
EyePos = zeros(0,7);
eyeCounter = 1;
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
            channel = fread(fid,1,'uint16=>uint16'); % start = 1
            softwareTS = fread(fid,1,'int64=>int64');
            hardwareTS = fread(fid,1,'int64=>int64');
            TTLs = [TTLs;double(channel),double(risingEdge),double(softwareTS),double(hardwareTS)];
            ttlCounter=ttlCounter+1;
        case SPIKE
            
            softwareTS = fread(fid,1,'int64=>int64');
            hardwareTS = fread(fid,1,'int64=>int64');
            sortedID = fread(fid,1,'int16=>int16');
            electrodeID = fread(fid,1,'int16=>int16');
            numChannels = fread(fid,1,'int16=>int16');
            numPts = fread(fid,1,'int16=>int16');
            if (eventSize(index) > 8+8+2+2+2+2)
                Waveform = fread(fid,numPts*numChannels,'uint16=>uint16');
            end
            if (spikeCounter == 1)
               Spikes = [softwareTS, hardwareTS, electrodeID, sortedID];
               AllWaveforms = [Waveform(:)'];
            else
               Spikes = [Spikes;double(softwareTS), double(hardwareTS), double(electrodeID), double(sortedID)];
               AllWaveforms = [AllWaveforms;Waveform(:)'];
               
            end
            spikeCounter=spikeCounter+1;
        case NETWORK
             str = fread(fid,eventSize(index)-8,'char=>char')'; % start = 1
             softwareTS = fread(fid,1,'int64=>int64');
             acNetworkEvents{networkCounter} = str;
             networkEventsTS(networkCounter) = softwareTS;
             networkCounter = networkCounter + 1;
        case EYE_POSITION
              x = fread(fid,1,'double=>double');
              y = fread(fid,1,'double=>double');
              xc = fread(fid,1,'double=>double');
              yc = fread(fid,1,'double=>double');
              p = fread(fid,1,'double=>double');
              softwareTS = fread(fid,1,'int64=>int64');
              hardwareTS = fread(fid,1,'int64=>int64');
              EyePos = [EyePos;x,y,xc,yc,p,double(softwareTS),double(hardwareTS)];
              eyeCounter = eyeCounter+1;

        otherwise
            fprintf('Unknown event. Skipping %d\n',eventSize(index));
            fseek(fid,eventSize(index),0);
    end
    
end
fclose(fid);

%% Hardware-Software Synchronization 
% Match software & hardware timestamp using robust linear regression.
% jitter is expected. software-hardware pairs are only an approximation
% (since we never know exactly what was the software TS when the hardware
% clock
% robust fit has trouble with very large values.
% so we remove the offset and then regress
[SyncCoeff]=robustfit(TimstampSoftware(:)-TimstampSoftware(1),TimestampHardware(:)-TimestampHardware(1));

figure(1);
clf;
plot(TimstampSoftware-TimstampSoftware(1),TimestampHardware-TimestampHardware(1),'.');
JitterMS = 1e3/ header.sampleRate*(((TimstampSoftware-TimstampSoftware(1))* SyncCoeff(2) + SyncCoeff(1)) -  (TimestampHardware-TimestampHardware(1))) ;
hold on;
plot(TimstampSoftware-TimstampSoftware(1),(((TimstampSoftware-TimstampSoftware(1))* SyncCoeff(2) + SyncCoeff(1))),'r');
title(sprintf('hardware-software synchronization. Avg jitter %.3f (ms)',mean(JitterMS)));

%% Software-Software Synchronization 
% Synchronize software timestamps between two computers.
% The remote computer sends messages of the form [keyword TS]
% in this case, the keyword is "KofikoSync".
syncKeyword = 'KofikoSync';
localComputerTS = [];
remoteComputerTS = [];
for k=1:length(acNetworkEvents)
    if strncmpi(acNetworkEvents{k},syncKeyword,length(syncKeyword))
        localComputerTS = [localComputerTS,networkEventsTS(k)];
        remoteComputerTS = [remoteComputerTS, str2double(acNetworkEvents{k}(length(syncKeyword)+1:end))];
    end
end
if ~isempty(localComputerTS)
    % Use robust linear regression to find optimal coefficients to match
    % local and remote timestamps
    [RemoteComputerSyncCoeff]=robustfit(localComputerTS(:)-localComputerTS(1),remoteComputerTS(:)-remoteComputerTS(1));
    JitterMS = 1e3*(((localComputerTS-localComputerTS(1))* RemoteComputerSyncCoeff(2) + RemoteComputerSyncCoeff(1)) -  (remoteComputerTS-remoteComputerTS(1))) ;
    figure(2);
    clf;
    plot(localComputerTS-localComputerTS(1),remoteComputerTS-remoteComputerTS(1),'b.');
    hold on;
    plot(localComputerTS-localComputerTS(1),(((localComputerTS-localComputerTS(1))* RemoteComputerSyncCoeff(2) + RemoteComputerSyncCoeff(1))),'r');
    title(sprintf('local-remote computer synchronization. Avg jitter %.3f (ms)',mean(JitterMS)));
end

%% Spikes
if ~isempty(Spikes)
    figure(3);
    clf;
    [sortedUnits,~,mapToUnits] = unique(Spikes(:,3:4),'rows');
    n=ceil(sqrt(size(sortedUnits,1)));
    for k=1:size(sortedUnits,1)
        subplot(n,n,k);
        unitWaveforms = double(AllWaveforms(mapToUnits==k,:));
        plot(-1*unitWaveforms','color',[0.5 0.5 0.5]);
        hold on;
        plot(-1*mean(unitWaveforms),'k','LineWidth',2);
        title(sprintf('Electrode %d, Unit %d',sortedUnits(k,1),sortedUnits(k,2)));
    end
end
%% Trial matrix
state = 0;
TRIAL_START = 'TrialStart';
TRIAL_TYPE = 'TrialType';
TRIAL_ALIGN = 'TrialAlign';
TRIAL_END = 'TrialEnd';
TRIAL_OUTCOME = 'TrialOutcome';
trialTable = nans(0,5); % [trial type, trial start TS, trial align TS, trial end TS, trial outcome]
trialCounter = 1;
for k=1:length(acNetworkEvents)
    if strncmpi(acNetworkEvents{k},TRIAL_START,length(TRIAL_START))
        trialTable(trialCounter, 2) = networkEventsTS(k);
        if length(acNetworkEvents{k}) > length(TRIAL_START)
            % Trial type information was passed 
            trialType = str2num(acNetworkEvents{k}(length(TRIAL_START)+1:end));
            if ~isempty(trialType)
                trialTable(trialCounter, 1) = trialType;
            end
        end
        state = 1;
    end
    if strncmpi(acNetworkEvents{k},TRIAL_TYPE,length(TRIAL_TYPE))
            trialType = str2num(acNetworkEvents{k}(length(TRIAL_TYPE)+1:end));
            if ~isempty(trialType)
                trialTable(trialCounter, 1) = trialType;
            end
    end
    if strncmpi(acNetworkEvents{k},TRIAL_ALIGN,length(TRIAL_ALIGN))
              trialTable(trialCounter, 3) = networkEventsTS(k);
    end    
    if strncmpi(acNetworkEvents{k},TRIAL_END,length(TRIAL_END))    
        if (state == 1)
            trialTable(trialCounter, 4) = networkEventsTS(k);
            if length(acNetworkEvents{k}) > length(TRIAL_END)
                % Trial outcome information was passed 
                trialOutcome = str2num(acNetworkEvents{k}(length(TRIAL_END)+1:end));
                if ~isempty(trialType)
                    trialTable(trialCounter, 5) = trialOutcome;
                end
            end
            state = 0;
            trialCounter=trialCounter+1;
            trialTable(trialCounter,:) = nans(1,5);
        end
    end
  if strncmpi(acNetworkEvents{k},TRIAL_OUTCOME,length(TRIAL_OUTCOME))    
       trialOutcome = str2num(acNetworkEvents{k}(length(TRIAL_OUTCOME)+1:end));
         if ~isempty(trialType)
                    trialTable(trialCounter, 5) = trialOutcome;
         end
  end
      
end
%% TTL
TTLhardwareOnsets = cell(1,8);
for ttlChannel=1:8
    TTLhardwareOnsets{ttlChannel} =  TTLs((TTLs(:,1) == ttlChannel & TTLs(:,2) == 1), 4);
end

%% Build Advancers matrix
advancerMatrix = [
% reconstruct the position of each advancer
NEW_ADVANCER_MSG = 'NewAdvancer';
NEW_ADVANCER_POS_MSG = 'NewAdvancerPosition';
NEW_ADVANCER_CONTAINER_MSG = 'NewAdvancerContainer';
for k=1:length(acNetworkEvents)
    
end

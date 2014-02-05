function [TTLs, Spikes,AllWaveforms,acNetworkEvents,...
                networkEventsTS,trialTable,...
                strctHardwareSoftwareSync,strctLocalRemoteSync,EyePos] = load_events_version_0_3(filename, verbose)

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
fid = fopen(filename);
%filesize = getfilesize(fid);

% constants
NUM_HEADER_BYTES = 1024;

% constants for pre-allocating matrices:
MAX_NUMBER_OF_SPIKES = 1e6;
MAX_NUMBER_OF_RECORDS = 1e4;
MAX_NUMBER_OF_CONTINUOUS_SAMPLES = 10e6;
MAX_NUMBER_OF_TIMESTAMPS = 1e6;
MAX_NUMBER_OF_EYE_POSITION = 200 * 60 * 60 * 10; % 10 hours worth of data
MAX_NUMBER_OF_TTLS = 1e6;
MAX_NUMBER_OF_EVENTS = 1e6;
SPIKE_PREALLOC_INTERVAL = 1e6;
MAX_NUMBER_OF_NETWORK_EVENTS = 1e6;

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
AllWaveforms = [];
timestampCounter = 1;
acNetworkEvents = cell(1,MAX_NUMBER_OF_NETWORK_EVENTS);
networkEventsTS = zeros(1,MAX_NUMBER_OF_NETWORK_EVENTS);
networkCounter = 1;
ttlCounter = 1;
TimstampSoftware = zeros(1,MAX_NUMBER_OF_TIMESTAMPS);
TimestampHardware = zeros(1,MAX_NUMBER_OF_TIMESTAMPS);
SessionStartInd = [];
spikeCounter = 1;
TTLs = zeros(MAX_NUMBER_OF_TTLS,4);
EyePos = zeros(MAX_NUMBER_OF_EYE_POSITION,7);
Spikes = zeros(MAX_NUMBER_OF_SPIKES,4);
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
            TTLs(ttlCounter,:) = [double(channel),double(risingEdge),double(softwareTS),double(hardwareTS)];
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
               AllWaveforms = zeros(MAX_NUMBER_OF_SPIKES, length(Waveform(:)));
            end
            Spikes(spikeCounter,:) = [double(softwareTS), double(hardwareTS), double(electrodeID), double(sortedID)];
            AllWaveforms(spikeCounter,:) = Waveform(:)';
            
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
              EyePos(eyeCounter,:) = [x,y,xc,yc,p,double(softwareTS),double(hardwareTS)];
              eyeCounter = eyeCounter+1;

        otherwise
            fprintf('Unknown event. Skipping %d\n',eventSize(index));
            fseek(fid,eventSize(index),0);
    end
    
end
fclose(fid);

Spikes = Spikes(1:spikeCounter-1,:);
AllWaveforms = AllWaveforms(1:spikeCounter-1,:);
TTLs=TTLs(1:ttlCounter-1,:);
EyePos=EyePos(1:eyeCounter-1,:);
acNetworkEvents = acNetworkEvents(1:networkCounter-1);
networkEventsTS= networkEventsTS(1:networkCounter-1);

TimstampSoftware = TimstampSoftware(1:timestampCounter-1);
TimestampHardware = TimestampHardware(1:timestampCounter-1);

%% Hardware-Software Synchronization 
% Match software & hardware timestamp using robust linear regression.
% jitter is expected. software-hardware pairs are only an approximation
% (since we never know exactly what was the software TS when the hardware
% clock
% robust fit has trouble with very large values.
% so we remove the offset and then regress
[SyncCoeff]=robustfit(TimstampSoftware(:)-TimstampSoftware(1),TimestampHardware(:)-TimestampHardware(1));
strctHardwareSoftwareSync.coeff = SyncCoeff;
strctHardwareSoftwareSync.sourceT0 = TimstampSoftware(1);
strctHardwareSoftwareSync.targetT0 = TimestampHardware(1);

if (verbose)
    figure(1);
    clf;
    plot(TimstampSoftware-TimstampSoftware(1),TimestampHardware-TimestampHardware(1),'.');
    JitterMS = 1e3/ header.sampleRate*(((TimstampSoftware-TimstampSoftware(1))* SyncCoeff(2) + SyncCoeff(1)) -  (TimestampHardware-TimestampHardware(1))) ;
    hold on;
    plot(TimstampSoftware-TimstampSoftware(1),(((TimstampSoftware-TimstampSoftware(1))* SyncCoeff(2) + SyncCoeff(1))),'r');
    title(sprintf('hardware-software synchronization. Avg jitter %.3f (ms)',mean(JitterMS)));
    xlabel('Software timestamp');
    ylabel('Hardware timestamp');
end

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
    
    strctLocalRemoteSync.coeff = SyncCoeff;
    strctLocalRemoteSync.sourceT0 = localComputerTS(1);
    strctLocalRemoteSync.targetT0 = remoteComputerTS(1);
    
    if (verbose)
        JitterMS = 1e3*(((localComputerTS-localComputerTS(1))* RemoteComputerSyncCoeff(2) + RemoteComputerSyncCoeff(1)) -  (remoteComputerTS-remoteComputerTS(1))) ;
        figure(2);
        clf;
        plot(localComputerTS-localComputerTS(1),remoteComputerTS-remoteComputerTS(1),'b.');
        hold on;
        plot(localComputerTS-localComputerTS(1),(((localComputerTS-localComputerTS(1))* RemoteComputerSyncCoeff(2) + RemoteComputerSyncCoeff(1))),'r');
        title(sprintf('local-remote computer synchronization. Avg jitter %.3f (ms)',mean(JitterMS)));
    end
else
    strctLocalRemoteSync = [];
end

%% Spikes
if ~isempty(Spikes)
    if (verbose)
        figure(3);
        clf;
        [sortedUnits,~,mapToUnits] = unique(Spikes(:,3:4),'rows');
        n=ceil(sqrt(size(sortedUnits,1)));
        tm = [-8:31]/30000*1e3;
        for k=1:size(sortedUnits,1)
            subplot(n,n,k);
            unitWaveforms = double(AllWaveforms(mapToUnits==k,:));
            unitWaveforms_uV = ((unitWaveforms-32768)* 0.00975000020);
            plot(tm,unitWaveforms_uV','color',[0.5 0.5 0.5]);
            hold on;
            plot(tm,mean(unitWaveforms_uV),'k','LineWidth',2);
            title(sprintf('Electrode %d, Unit %d',sortedUnits(k,1),sortedUnits(k,2)));
            xlabel('Time (ms)');
            ylabel('Amplitude (uV)');
            set(gca,'ylim',[-120 100]);
        end
    end
else
    AllWaveforms = [];
    sortedUnits = [];
end
%% Trial matrix
state = 0;
TRIAL_START = 'TrialStart';
TRIAL_TYPE = 'TrialType';
TRIAL_ALIGN = 'TrialAlign';
TRIAL_END = 'TrialEnd';
TRIAL_OUTCOME = 'TrialOutcome';
trialTable = NaN*ones(0,5); % [trial type, trial start TS, trial align TS, trial end TS, trial outcome] (all TS are in software)
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
            trialTable(trialCounter,:) = NaN*ones(1,5);
        end
    end
  if strncmpi(acNetworkEvents{k},TRIAL_OUTCOME,length(TRIAL_OUTCOME))    
       trialOutcome = str2num(acNetworkEvents{k}(length(TRIAL_OUTCOME)+1:end));
         if ~isempty(trialType)
                    trialTable(trialCounter, 5) = trialOutcome;
         end
  end
      
end


%% Build trial aligned responses.
% Build a raster [-200..500] ms relative to trial alignment 
iBeforeMS = -200;
iAfterMS = 500;
if ~isempty(sortedUnits) && ~isempty(trialTable) && ~isempty(strctHardwareSoftwareSync)
    trialAlignTShardware = timeZoneChange(strctHardwareSoftwareSync, trialTable(:,3),0); % software->hardware
    
    numUnits = size(sortedUnits,1);
    for unitIter=1:numUnits
        relevantSpikeTimesHardware = Spikes( mapToUnits == unitIter,2);
        
        % Ignore trial outcome. Just aggregate according to trial type
        [a2bRaster,periStimulusRangeMS] = buildRaster(relevantSpikeTimesHardware/header.sampleRate, trialAlignTShardware/header.sampleRate, iBeforeMS, iAfterMS);
        
        % First, build a large 1-ms resolution raster matrix
        
        
        [uniqueTrialTypes,~,mapToUniqueTrialType]=unique(trialTable(:,1));
        numUniqueTrials = length(uniqueTrialTypes);
        averageRaster = zeros(numUniqueTrials, length(periStimulusRangeMS));
        for k=1:numUniqueTrials
            relevantTrials = find(mapToUniqueTrialType == k);
            if ~isempty(relevantTrials)
                averageRaster(k,:)=mean(a2bRaster(relevantTrials,:),1);
            end
        end
    end
end

% figure(11);
% clf;
% imagesc(periStimulusRangeMS,1:numUniqueTrials,conv2(averageRaster,fspecial('gaussian',[1 50],3),'same'));
% xlabel('Time (relative to image onset), in ms');
% ylabel('Image number');
% clear psth
% psth(1,:) = mean(averageRaster(1:16,:),1);
% psth(2,:) = mean(averageRaster(17:32,:),1);
% psth(3,:) = mean(averageRaster(33:48,:),1);
% psth(4,:) = mean(averageRaster(49:64,:),1);
% psth(5,:) = mean(averageRaster(65:80,:),1);
% psth(6,:) = mean(averageRaster(81:96,:),1);
% psth=conv2(psth,fspecial('gaussian',[1 50],3),'same');
% figure(12);
% clf;
% plot(periStimulusRangeMS,1e3*psth');
% ylabel('Average firing rate (Hz)');
% xlabel('Time (ms)');
% legend('Faces','Bodies','Fruits','Object','Hands','Scrambled Images','Location','NorthEastOutside');

%% TTL
TTLhardwareOnsets = cell(1,8);
for ttlChannel=1:8
    TTLhardwareOnsets{ttlChannel} =  TTLs((TTLs(:,1) == ttlChannel & TTLs(:,2) == 1), 4);
end

%% Build Advancers matrix

% reconstruct the position of each advancer
NEW_ADVANCER_MSG = 'NewAdvancer';
NEW_ADVANCER_POS_MSG = 'NewAdvancerPosition';
NEW_ADVANCER_CONTAINER_MSG = 'NewAdvancerContainer';
for k=1:length(acNetworkEvents)
      if strncmpi(acNetworkEvents{k},NEW_ADVANCER_POS_MSG,length(NEW_ADVANCER_POS_MSG))
          
      else
          if strncmpi(acNetworkEvents{k},NEW_ADVANCER_CONTAINER_MSG,length(NEW_ADVANCER_CONTAINER_MSG))
          acNetworkEvents{k}
          else 
                if strncmpi(acNetworkEvents{k},NEW_ADVANCER_MSG,length(NEW_ADVANCER_MSG))
            acNetworkEvents{k}
                end 
          end
      end      
end



return


function [a2bRaster,aiPeriStimulusRangeMS] = buildRaster(spikeTimes, trialAlignTimes, iBeforeMS, iAfterMS)
%
aiPeriStimulusRangeMS = iBeforeMS:iAfterMS;
iNumTrials = length(trialAlignTimes);
a2bRaster = zeros(iNumTrials, length(aiPeriStimulusRangeMS));
warning off
for iTrialIter=1:iNumTrials
    aiSpikesInd = find(...
        spikeTimes >= trialAlignTimes(iTrialIter) + iBeforeMS/1e3 & ...
        spikeTimes <= trialAlignTimes(iTrialIter) + iAfterMS/1e3);
    
    if ~isempty(aiSpikesInd)
        aiSpikeTimesBins = 1+floor( (spikeTimes(aiSpikesInd) - trialAlignTimes(iTrialIter) -iBeforeMS/1e3)*1e3);
        aiIncreaseCount = hist(aiSpikeTimesBins, 1:length(aiPeriStimulusRangeMS));
        a2bRaster(iTrialIter, :) = a2bRaster(iTrialIter, :) + aiIncreaseCount;
        
    end;
end;
%a2bRaster = a2bRaster > 0; % Ignore multiple spikes falling inside same bin ?
return;


function outTS = timeZoneChange(strctSync, inTS, direction)
% Convert timestamps from source to target using a sync structure.
% for example, convert software timestamps to hardware timestamps.
% To convert from target to source, use the same sync structure, but use
% direction = 1.
%
% transformation is defined as:
% target = targetT0 + (sourceTS-sourceT0)*coeff(2) + coeff(1)
if direction > 0
    % target->source
    outTS = strctSync.sourceT0 + (inTS-strctSync.targetT0-strctSync.coeff(1))/strctSync.coeff(2);
else
    outTS = strctSync.targetT0 + (inTS-strctSync.sourceT0)*strctSync.coeff(2) +strctSync.coeff(1);
    % source->target
end



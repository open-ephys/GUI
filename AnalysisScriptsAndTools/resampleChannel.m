function [resampledRealigned,rangeSec] = resampleChannel(channelFileName, intervalRange,alignTS, hpf)
% resample to 1 ms resolution
[data, timestamps, info] = load_continuous_data(channelFileName);
sampleRate = info.header.sampleRate;

if (hpf)
    [b,a]=ellip(2,0.1,40,[300 6000]*2/sampleRate);
    data=filtfilt(b,a,data);
end

%intervalRange = 0.3; % 200 ms
upSample = 10;
nEvents = length(alignTS);
nSamples = 2 * intervalRange * sampleRate + 1;
range = linspace(-intervalRange *sampleRate,intervalRange*sampleRate,upSample*2*intervalRange * sampleRate+1);
% Generate sample times.
a2fSampleTimes = zeros(nEvents, length(range));
for k=1:nEvents
    a2fSampleTimes(k,:) = alignTS(k) + range;
end
resampledRealigned = reshape(interp1(timestamps, data,a2fSampleTimes(:),'linear','extrap'),size(a2fSampleTimes));
rangeSec = range / sampleRate;
return
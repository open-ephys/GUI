% Simple analysis pipeline 
% 1. Read continuous channel
% 2. Read events and parse them into TTLs, Spikes, and Trial Information
% 3. build raster and PSTHs for different trial types.

%folder = 'C:\Users\shayo\Documents\GitHub\GUI\Builds\VisualStudio2012\Release64\bin\140115_083959_Debug';

%folder = 'C:\Users\shayo\Documents\GitHub\GUI\Builds\VisualStudio2012\Release64\bin\140115_115528_Debug'
%folder = 'C:\Users\shayo\Documents\GitHub\GUI\Builds\VisualStudio2012\Release64\bin\140115_133728_Debug';
%folder = 'C:\Users\shayo\Documents\GitHub\GUI\Builds\VisualStudio2012\Release64\bin\2014-01-15_14-12-29'
folder = 'C:\Users\shayo\Documents\GitHub\GUI\Builds\VisualStudio2012\Release64\bin\140117_143151_Spiderman';
eventsFilename = [folder,filesep,'all_channels.events'];
ch1 = [folder,filesep,'100_CH1.continuous'];
stim = [folder,filesep,'100_STIM.continuous'];
gate = [folder,filesep,'100_GATE.continuous'];
%channelFileName = [folder,filesep,'100_STIM.continuous'];

[TTLs, Spikes,AllWaveforms,acNetworkEvents,...
                networkEventsTS,trialTable,...
                strctHardwareSoftwareSync,strctLocalRemoteSync] = load_events_version_0_3(eventsFilename,1);

% look at CH1 aligned to ttl1 rise events at [-200..200] ms
ttl1RiseHardwareTS = TTLs( (TTLs(:,1) == 5 & TTLs(:,2) == 1),4);
intervalRange = 0.2;
[ch1_data,range] = resampleChannel(ch1, intervalRange,ttl1RiseHardwareTS,false);
stim_data= resampleChannel(stim, intervalRange,ttl1RiseHardwareTS,false);
gate_data = resampleChannel(gate, intervalRange,ttl1RiseHardwareTS,false);

figure(11);
clf;hold on;
h1=plot(1e3*range,0.1*(stim_data'-9000),'b');
h2=plot(1e3*range,ch1_data','r');
h3=plot(1e3*range,0.1*(gate_data'-9000),'g');
xlabel('ms');
legend([h1(1),h2(1),h3(1)],{'stimulation','CH1','fast settle'});
set(gca,'xlim',[-2 6])
set(gca,'ylim',[-1000 1000]);

function [data, timestamps, info] = load_continuous_data(filename)
if ~exist(filename,'file')
    fprintf('File does not exit!\n');
    data = [];
    timestamps = [];
    info = [];
    return
end
fid = fopen(filename,'rb+');
filesize = getfilesize(fid);

index = 0;
NUM_HEADER_BYTES = 1024;
SAMPLES_PER_RECORD = 1024;
RECORD_SIZE = 8 + 16 + SAMPLES_PER_RECORD*2 + 10; % size of each continuous record in bytes
MAX_NUMBER_OF_RECORDS = 1e4;
MAX_NUMBER_OF_CONTINUOUS_SAMPLES = 10e6;
RECORD_MARKER = [0 1 2 3 4 5 6 7 8 255]';
RECORD_MARKER_V0 = [0 0 0 0 0 0 0 0 0 255]';

hdr = fread(fid, NUM_HEADER_BYTES, 'char*1');
eval(char(hdr'));
info.header = header;

if (isfield(info.header, 'version'))
    version = info.header.version;
else
    version = 0.0;
end

% pre-allocate space for continuous data
data = zeros(MAX_NUMBER_OF_CONTINUOUS_SAMPLES, 1);
info.ts = zeros(1, MAX_NUMBER_OF_RECORDS);
info.nsamples = zeros(1, MAX_NUMBER_OF_RECORDS);

if version >= 0.2
    info.recNum = zeros(1, MAX_NUMBER_OF_RECORDS);
end

current_sample = 0;
fprintf('Reading %s\n',filename);
while ftell(fid) + RECORD_SIZE < filesize % at least one record remains
    if mod(index,10000) == 0
        fprintf('Passed record %d\n',index);
    end;
    go_back_to_start_of_loop = 0;
    
    index = index + 1;
    
    if (version >= 0.1)
        timestamp = fread(fid, 1, 'int64', 0, 'l');
        nsamples = fread(fid, 1, 'uint16',0,'l');
        
        
        if version >= 0.2
            recNum = fread(fid, 1, 'uint16');
        end
        
    else
        timestamp = fread(fid, 1, 'uint64', 0, 'l');
        nsamples = fread(fid, 1, 'int16',0,'l');
    end
    
    
    if nsamples ~= SAMPLES_PER_RECORD && version >= 0.1
        
        disp(['  Found corrupted record...searching for record marker.']);
        
        % switch to searching for record markers
        
        last_ten_bytes = zeros(size(RECORD_MARKER));
        
        for bytenum = 1:RECORD_SIZE*5
            
            byte = fread(fid, 1, 'uint8');
            
            last_ten_bytes = circshift(last_ten_bytes,-1);
            
            last_ten_bytes(10) = double(byte);
            
            if last_ten_bytes(10) == RECORD_MARKER(end);
                
                sq_err = sum((last_ten_bytes - RECORD_MARKER).^2);
                
                if (sq_err == 0)
                    disp(['   Found a record marker after ' int2str(bytenum) ' bytes!']);
                    go_back_to_start_of_loop = 1;
                    break; % from 'for' loop
                end
            end
        end
        
        % if we made it through the approximate length of 5 records without
        % finding a marker, abandon ship.
        if bytenum == RECORD_SIZE*5
            
            disp(['Loading failed at block number ' int2str(index) '. Found ' ...
                int2str(nsamples) ' samples.'])
            
            break; % from 'while' loop
            
        end
        
        
    end
    
    if ~go_back_to_start_of_loop
        
        block = fread(fid, nsamples, 'int16', 0, 'b'); % read in data
        
        fread(fid, 10, 'char*1'); % read in record marker and discard
        
        data(current_sample+1:current_sample+nsamples) = block;
        
        current_sample = current_sample + nsamples;
        
        info.ts(index) = timestamp;
        info.nsamples(index) = nsamples;
        
        if version >= 0.2
            info.recNum(index) = recNum;
        end
        
    end
    
end

% crop data to the correct size
data(current_sample+1:end) = [ ];
info.ts(index+1:end) = [ ];
info.nsamples(index+1:end) = [ ];

if version >= 0.2
    info.recNum(index+1:end) = [ ];
end

timestamps = nan(size(data));

current_sample = 0;

if version >= 0.1
    
    for record = 1:length(info.ts)
        
        ts_interp = info.ts(record):info.ts(record)+info.nsamples(record);
        
        timestamps(current_sample+1:current_sample+info.nsamples(record)) = ts_interp(1:end-1);
        
        current_sample = current_sample + info.nsamples(record);
    end
else % v0.0; NOTE: the timestamps for the last record will not be interpolated
    
    for record = 1:length(info.ts)-1
        
        ts_interp = linspace(info.ts(record), info.ts(record+1), info.nsamples(record)+1);
        
        timestamps(current_sample+1:current_sample+info.nsamples(record)) = ts_interp(1:end-1);
        
        current_sample = current_sample + info.nsamples(record);
    end
    
end
fprintf('Done\n');
return


function filesize = getfilesize(fid)

fseek(fid,0,'eof');
filesize = ftell(fid);
fseek(fid,0,'bof');


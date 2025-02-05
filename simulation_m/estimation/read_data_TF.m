% UAV-ART // Aerot�c - N�cleo de estudantes de Engenharia Aeroespacial
% Authors: - Hugo Pereira
%          - Pedro Martins
%          - Sim�o Caeiro

% Program that reads the ".txt" file data to the structures array "data"

fid = fopen('test_flight_log5.txt','rt');

if fid < 0
    disp('Error opening the file ".txt"');
else
    i = -1;
    while ~feof(fid)
        line = fgetl(fid);
        i = i + 1;             
        if  i > 0
            [value, remain]  =  strtok(line);
            data(i).time     =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).theta    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).psi      =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).phi      =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).q        =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).r        =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).p        =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).lat      =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).long     =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).alt      =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).vnorth   =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).veast    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).vdown    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).xacc     =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).yacc     =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).zacc     =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).IAS      =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).alpha    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).beta     =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).wx       =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).wy       =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).wz       =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).RCch1    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).RCch2    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).RCch3    =  str2num(value);
            [value, remain]  =  strtok(remain);
            data(i).RCch4    =  str2num(value);
        end
    end
end

fclose(fid);

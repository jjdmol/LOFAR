function SelectedSubbandSignals = ReadSelectedSubbandSignals(path, NumberOfSubbands, DataLength, ...
                                  SubbandFilterLength, NumberOfAntennas)

InputFile = load(path);

%********************* For two antennas ***********************************************************

% Input file (NumberOfAntennas x NumberofSubbands repeated Tn times):
%       A1---------------->#subbands
%       A2---------------->#subbands (Sn)
%       A1
%       A2
%        |
%        Tn

% Tn = DataLength / NumberOfSubbands - SubbandFilterLength + 1;
% % 
% % for t = 1 : Tn
% %     for s = 1 : NumberOfSubbands
% %         for a = 1 : NumberOfAntennas
% %             SelectedSubbandSignals(a, t, s) = InputFile(t, (((s - 1) * 2 * NumberOfAntennas) + (a * 2) - 1)) + i * InputFile(t, (((s - 1) * 2 * NumberOfAntennas) + (a * 2)));
% %         end
% %     end
% % end
% tt = 0;
% for t = 1 : 2 : Tn * 2
%     tt = tt + 1;
%     ss = 0;
%     for s = 1 : NumberOfAntennas * 2 : NumberOfSubbands * NumberOfAntennas
%         ss = ss + 1;
%         for a = 1 : NumberOfAntennas
%             SelectedSubbandSignalsA(a, tt, ss) = InputFile(t, s + (a - 1) * 2) + ...
%                                               i * InputFile(t, s + (a - 1) * 2 + 1);
%         end
%     end
% end
% 
% tt = 0;
% for t = 2 : 2 : Tn * 2
%     tt = tt + 1;
%     ss = 0;
%     for s = 1 : NumberOfAntennas * 2 : NumberOfSubbands * NumberOfAntennas
%         ss = ss + 1;
%         for a = 1 : NumberOfAntennas
%             SelectedSubbandSignalsB(a, tt, ss) = InputFile(t, s + (a - 1) * 2) + ...
%                                               i * InputFile(t, s + (a - 1) * 2 + 1);
%         end
%     end
% end
% 
% 
% for t = 1 : Tn
%     for a = 1 : NumberOfAntennas
%         ss = 1;
%         for s = 1 : NumberOfSubbands
%             if s < 65
%                 SelectedSubbandSignals(a, t, s) = SelectedSubbandSignalsA(a, t, s);
%             else
%                 SelectedSubbandSignals(a, t, s) = SelectedSubbandSignalsB(a, t, ss);
%                 ss = ss + 1;
%             end
%         end
%     end
% end
% 
% i=0;
%     %     *----------*
%     %    /Sn        /|
%     %   /.         / |      Tn = n-th time sample
%     %  /.         /  |
%     % *----------*   |      An = n-th Antenna
%     % |A1......Tn|   *
%     % |.         |  /       Sn = n-th Subband
%     % |.         | /
%     % |An        |/         How to index: (An, Tn, Sn)
%     % *----------*


%********************* For one antennas ***********************************************************

Tn = DataLength / NumberOfSubbands - SubbandFilterLength + 1;
% 
% for t = 1 : Tn
%     for s = 1 : NumberOfSubbands
%         for a = 1 : NumberOfAntennas
%             SelectedSubbandSignals(a, t, s) = InputFile(t, (((s - 1) * 2 * NumberOfAntennas) + (a * 2) - 1)) + i * InputFile(t, (((s - 1) * 2 * NumberOfAntennas) + (a * 2)));
%         end
%     end
% end
for t = 1 : Tn
    ss = 0;
    for s = 1 : 2 : NumberOfSubbands * NumberOfAntennas * 2
        ss = ss + 1;
        SelectedSubbandSignals(1, t, ss) = InputFile(t, s) + i * InputFile(t, s + 1);
        Antenna1(t, ss) = SelectedSubbandSignals(1, t, ss);
    end
end

i = 0;
% for t = 1 : Tn
%     ss = 1;
%     for s = 1 : NumberOfSubbands
%         if s < 65
%             SelectedSubbandSignals(a, t, s) = SelectedSubbandSignalsA(a, t, s);
%         else
%             SelectedSubbandSignals(a, t, s) = SelectedSubbandSignalsB(a, t, ss);
%             ss = ss + 1;
%         end
%     end
% end

    %     *----------*
    %    /Sn        /|
    %   /.         / |      Tn = n-th time sample
    %  /.         /  |
    % *----------*   |      An = n-th Antenna
    % |A1......Tn|   *
    % |.         |  /       Sn = n-th Subband
    % |.         | /
    % |An        |/         How to index: (An, Tn, Sn)
    % *----------*

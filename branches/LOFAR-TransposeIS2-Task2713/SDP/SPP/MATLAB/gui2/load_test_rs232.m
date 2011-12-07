%Function to send a test file to the prototyping board via RS232
% Input parameters are:
        %output bin number
        %name of the test that is loaded
% Output:
        %file sent via RS232
        
function load_test_rs232(test_number)

cd RS232_C_code;
cd ..;

%cmd_str='copy InputFile.txt totofile.txt';
%dos(cmd_str);
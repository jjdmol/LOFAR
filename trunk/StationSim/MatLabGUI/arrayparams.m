function [params,chain] = arrayparams (array_type)

     switch array_type
         case 1
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=[' Compact THEA'];
         case 2     
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=[' Compact THEA, but now with triangular lattice structure'];
         case 3     
            ringsize  = [13 0 0 0 15];
            ringangle = [0 0 0 0 0];
            chain=[' 16 element array in one ring '];
         case 4     
            ringsize  = [10 4.4 0 0 0];
            ringangle = [0 36 0 0 0];
            chain=[' 16 element array with two rings of 11 and 5'];
         case 5     
            ringsize  = [10.8 6.5 0 0 7];
            ringangle = [28 0 0 0 0];
         case 6     
            ringsize  = [11 6.1 0 0 0];
            ringangle = [10 12 0 0 0];
         case 7     
            ringsize  = [14.5 10 5.3 0 0];
            ringangle = [30 10 0 0 0];
         
         case 8     
            ringsize  = [9.9 4.6 0 0 0];
            ringangle = [17 12 0 0 0];
            chain=[' 16 element array with three rings of 11, 4, 1'];
         case 9     
            ringsize  = [25 20 15 10 5];
            ringangle = [0 0 0 0 0];
            chain=[' Y shaped array [VLA]']
         case 10
            ringsize  = [25 20 15 10 5];
            ringangle = [0 10 20 30 40];
            chain=[' Spiral shaped array [3 arms]'];
         case 11
            ringsize  = [18.5 13.5 8.5 3.5 0];
            ringangle = [0 20 40 60 0];
            chain=[' Spiral shaped array [4 arms]']
         case 12
            ringsize  = [4.5 8 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=['logarithmic array [4x4]'];
         case 13
            ringsize  = [16 0 0 0 16];
            ringangle = [20 25 0 0 0];
            chain=['logarithmic circle'];
         case 14
            ringsize  = [0.5 1.0 5.5 0 0];
            ringangle = [46 0 0 0 0];
            chain=['logarithmic spiral'];
         case 15
            ringsize  = [11 6  0 0 0];
            ringangle = [30 50 47 58 104];
             chain=[' two logarithmic circle '];
         case 16
            ringsize  = [10 5.5  0 0 0];
            ringangle = [30 50 55 65 107];
            chain=[' three logarithmic circle'];
         case 17
            ringsize  = [4.5 8 0 0 0];
            ringangle = [0 10 20 0 0];
             chain=[' three pentagones']; 
         case 18
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=[' 8*8 THEA Tile '];
         case 19
            ringsize  = [1 4 0 0 15];
            ringangle = [0 0 0 0 0];            
            chain=[' LOFAR array of 5 circles with logarithmic radius '];
         case 20
            ringsize  = [1/2 3/2 0 0 13];
            ringangle = [20 0 0 0 0];         
            chain=['LOFAR array of 7 circles with logarithmic radius'];
         case 21
            ringsize  = [16 21 0 16 16];
            ringangle = [20 25 20 25 11];
             chain=['logarithmic circle'];
         case 22
            ringsize  = [22.5 17 11.8 6.1 0];
            ringangle = [180 0 180 0 0];
            chain=['Triangular lattice in middle']
         case 23
	        ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=['single antenna, slightly offset from center']
        case 24 % regular, 2 antennas
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=['regular array with two antennas, spaced one lambda apart']
        case 25 % regular, 6 antennas
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
            chain=['regular array with six antennas, spaces two lambda apart']
        case 26 % regular, 9 antennas
            ringsize  = [0];
            ringangle = [0];
            chain=['regular array with nine antennas, spaces four lambda apart']
        case 27 % regular, 6 antennas
            ringsize  = [0];
            ringangle = [0];
            chain=['square array, four elements, spaced four lambda apart']
        case 28
            ringsize  = [0];
            ringangle = [0];
            ringangle = [0];
            chain=['square array, four elements, spaced four lambda apart']
        case 29
            ringsize  = [0];
            ringangle = [0];
            ringangle = [0];
            chain=['rectangular array 4x8 elements, spaced 2 lambda apart'];
        case 30
            ringsize  = [0];
            ringangle = [0];
            chain=['rectangular array 4x8 elements, spaced 2 lambda apart'];
        case 31
            ringsize  = [5 0 0 0 15];
            ringangle = [0 0 0 0 0];
            chain=['rectangular array 4x8 elements, spaced 8 lambda apart'];
        case 32
            ringsize  = [0];
            ringangle = [0];
             chain=[' 16 element array with two rings of 11 and 5'];
        case 33
            ringsize  = [0];
            ringangle = [0];
             chain=['regular array two antennas'];
      end
params = [ringsize; ringangle];

function params = arrayparams (array_type)

     switch array_type
         case 1
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
         case 2     
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
         case 3     
            ringsize  = [13 0 0 0 15];
            ringangle = [0 0 0 0 0];
         case 4     
            ringsize  = [10 4.4 0 0 0];
            ringangle = [0 36 0 0 0];
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
         case 9     
            ringsize  = [25 20 15 10 5];
            ringangle = [0 0 0 0 0];
         case 10
            ringsize  = [25 20 15 10 5];
            ringangle = [0 10 20 30 40];
         case 11
            ringsize  = [18.5 13.5 8.5 3.5 0];
            ringangle = [0 20 40 60 0];
         case 12
            ringsize  = [4.5 8 0 0 0];
            ringangle = [0 0 0 0 0];
         case 13
            ringsize  = [16 0 0 0 16];
            ringangle = [20 25 0 0 0];
         case 14
            ringsize  = [0.5 1.0 5.5 0 0];
            ringangle = [46 0 0 0 0];
         case 15
            ringsize  = [11 6  0 0 0];
            ringangle = [30 50 47 58 104];
         case 16
            ringsize  = [10 5.5  0 0 0];
            ringangle = [30 50 55 65 107];
         case 17
            ringsize  = [4.5 8 0 0 0];
            ringangle = [0 10 20 0 0];
         case 18
            ringsize  = [0 0 0 0 0];
            ringangle = [0 0 0 0 0];
         case 19
            ringsize  = [1 4 0 0 15];
            ringangle = [0 0 0 0 0];            
         case 20
            ringsize  = [1/2 3/2 0 0 13];
            ringangle = [20 0 0 0 0];            
         case 21
            ringsize  = [16 21 0 16 16];
            ringangle = [20 25 20 25 11];
         case 22
            ringsize  = [22.5 17 11.8 6.1 0];
            ringangle = [180 0 180 0 0];
         case 23
	    ringsize  = [0];
            ringangle = [0];
      end
params = [ringsize; ringangle];

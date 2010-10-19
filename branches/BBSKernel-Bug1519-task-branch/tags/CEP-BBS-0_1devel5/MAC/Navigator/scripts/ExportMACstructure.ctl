main()
{
 	DebugTN("Export State: Start...");

	dyn_string PVSSTYPES;
	PVSSTYPES[1] = "structure";
	PVSSTYPES[3] = "character-array";
	PVSSTYPES[4] = "integer-array";
	PVSSTYPES[5] = "unsigned-array";
	PVSSTYPES[6] = "float-array";
	PVSSTYPES[7] = "bit-array";
	PVSSTYPES[8] = "bit-pattern-array";
	PVSSTYPES[9] = "text-array";
	PVSSTYPES[10] = "time-array";
	PVSSTYPES[11] = "character-structure";
	PVSSTYPES[12] = "integer-structure";
	PVSSTYPES[13] = "unsigned-structure ";
	PVSSTYPES[14] = "float-structure";
	PVSSTYPES[15] = "bit32";
	PVSSTYPES[16] = "bit32-structure";
	PVSSTYPES[17] = "text-structure";
	PVSSTYPES[18] = "time-structure";
	PVSSTYPES[19] = "character";
	PVSSTYPES[20] = "integer";
	PVSSTYPES[21] = "unsigned";
	PVSSTYPES[22] = "float";
	PVSSTYPES[23] = "bit";
	PVSSTYPES[24] = "bit-pattern";
	PVSSTYPES[25] = "text";
	PVSSTYPES[26] = "time";
	PVSSTYPES[27] = "identifier";
	PVSSTYPES[29] = "identifier-array";
	PVSSTYPES[39] = "identifier-array";
	PVSSTYPES[41] = "type-reference";
	PVSSTYPES[42] = "multilingual-text";
	PVSSTYPES[43] = "multilingual-text-structure";
	PVSSTYPES[44] = "description-array";
	PVSSTYPES[46] = "blob";
	PVSSTYPES[47] = "blob-structure";


 	DebugTN("Export State: Open file...");
	dyn_dyn_anytype tab;
	dyn_string ds;
	string fromSysNum = getSystemId();
	/*string fromSysNum = getSystemId(tfFromSys.text);*/

	slStatusLogging.deleteAllItems;
	if (tfFile.text == "")
	{
		DebugTN("No filename specified!");
	  	return;
	}
	file f = fopen("dpexport.out", "w");
 
	DebugTN(ferror(f));

 	DebugTN("Export State: Query types...");
	ds=dpTypes("T*", fromSysNum);
	DebugTN(ds); /* Returns all DPTs from system 2, which begin with "T" */
  
	fprintfUL(f, "%d\n", dynlen(ds)); /* number of types */
 	for(int i=1;i<=dynlen(ds);i++)
 	{
		dyn_dyn_string names;
		dyn_dyn_int types;
		int struc;
		string typename;
		struc=dpTypeGet(ds[i],names,types);

		DebugTN(ds[i],names,types);
		
 		fprintf(f, "%s\n",names[1][1]); /* type name */
 		for(int t=2;t<=dynlen(names);t++)
 		{
 			typename = PVSSTYPES[types[t][2]];
 			if(types[t][2] == 41 && dynlen(names[t]) >= 3) // typeref
 			{
 				typename = names[t][3];
 			}
	 		fprintf(f, "%s %s\n",names[t][2],typename); /* struct-item-name struct-item-type */
		}
		fprintf(f,"\n");
 	}
 	
 	DebugTN("Export State: Perform query...");

	/*ds = dpNames(tfFromSys.text + tfQuery.text);*/
 	ds = dpNames("P?C*");
 	for(int i=1;i<=dynlen(ds);i++)
 	{
 		string typename;
 		typename = dpTypeName(ds[i]);
 		// filter unknown types
 		if(typename[0] == 'T')
 		{
	 		fprintf(f, "%s %s\n",ds[i],typename);
	 	}
 	}

	fclose(f);
 	DebugTN("Export State: Ready/Idle");
}

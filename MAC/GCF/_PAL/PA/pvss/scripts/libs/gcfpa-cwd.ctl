#uses "gcfpa-com.ctl"

main()
{
	addGlobal("gConnManList", DYN_DYN_STRING_VAR);	
	addGlobal("gDistSysList", DYN_UINT_VAR);	
	
	startThread("gcfConnectionWatchDog");
}

// watchdog section start

void gcfConnectionWatchDog()
{
	DebugN("GCF: Starting GCF connection watch-dog");
	retrieveManNums(getSystemId());
 	dpGet("_DistManager.State.SystemNums", gDistSysList);
 	for (int i = 1; i <= dynlen(gDistSysList); i++)
 	{
 		retrieveManNums(gDistSysList[i]);	 		
 	}
 	dpConnect("distSystemChanged", FALSE, "_DistManager.State.SystemNums");
 	if (!dpExists("DPA_WDGoneSys"))
 	{
 		dpCreate("DPA_WDGoneSys", "LPT_UNSIGNED");
 	}
	DebugN("GCF: Watch-dog started");
}

void retrieveManNums(unsigned sysNr)
{
	DebugN("GCF: Add managers for (new) system " + getSystemName(sysNr) + ".");
	string sysName = getSystemName(sysNr);
	dyn_anytype manNums;
 	dpGet(sysName + "_Connections.Ui.ManNums", manNums);
 	addManagers(sysNr, manNums, "Ui");
 	dpConnect("uiConnectionsChanged", FALSE, sysName + "_Connections.Ui.ManNums");
 	dpGet(sysName + "_Connections.Api.ManNums", manNums);
 	addManagers(sysNr, manNums, "Api");
 	dpConnect("apiConnectionsChanged", FALSE, sysName + "_Connections.Api.ManNums");
}

void addManagers(unsigned sysNr, dyn_anytype manNums, string manType)
{
	string systemId = (string) sysNr;
	dyn_string manItem;

	for (int i = 1; i <= dynlen(manNums); i++)
	{
		manItem = makeDynString(sysNr, manType, manNums[i]);	
		DebugN("GCF: Add mananger: " + getSystemName(sysNr) + manType + ":" + manNums[i]);
		gConnManList[dynlen(gConnManList) + 1] = manItem;
	}
}

void distSystemChanged(string dp, dyn_uint value)
{
	dyn_uint curRegSysNrs = getDynUInt(gConnManList, 1);
	for (int i = 1; i <= dynlen(gDistSysList); i++)
	{
		if (!dynContains(value, gDistSysList[i]))
		{	
			remoteSystemGone(gDistSysList[i]);
		}
	}
	for (int i = 1; i <= dynlen(value); i++)
	{
		if (!dynContains(gDistSysList, value[i]))
		{
			retrieveManNums(value[i]);
		}
	}
	gDistSysList = value;
}

void remoteSystemGone(unsigned sysNr)
{
	DebugN("GCF: System " + getSystemName(sysNr) + " gone.");
	string msg = "d" + sysNr + ":";
	sendEventToPA(msg, getSystemName());
	dyn_anytype manNums;
	dpGet("_Connections.Api.ManNums", manNums);
	dyn_string apiDPNames;
	msg = "u|" + getSystemId() + ":Ctrl:" + myManNum() + ":|" + 
				getSystemName() + "DPA_server|gone|" + getSystemName() + "|";	
	for (int i = 1; i <= dynlen(manNums); i++)
	{
		apiDPNames = dpNames("DPA_client_API" + i + "_*", "LPT_BLOB");		
		for (int j = 1; j <= dynlen(apiDPNames); j++)
		{
			sendEvent(apiDPNames[j] + ".", msg);
		}
	}
	dyn_string goneManList;
	for (int i = 1; i <= dynlen(gConnManList); i++)
	{
		if (gConnManList[i][1] == sysNr)
		{						
			dynRemove(gConnManList, i);
		}
	}
	dpSet("DPA_WDGoneSys.", sysNr);
}

void uiConnectionsChanged(string dp, dyn_uint value)
{
	connectionsChanged(dp, value, "Ui");
}

void apiConnectionsChanged(string dp, dyn_uint value)
{
	connectionsChanged(dp, value, "Api");
}

void connectionsChanged(string dp, dyn_uint value, string manType)
{
	string sysNr = getSystemId(dpSubStr(dp, DPSUB_SYS));
	
	dyn_string msg;
	string manString;
	dyn_uint deletedLocalManNums;
	int i, j;
	bool manNumFound = false;
	dyn_string newItem;
	for (i = 1; i <= dynlen(value); i++)
	{
		for (j = 1; j <= dynlen(gConnManList); j++)
		{
			if (gConnManList[j][1] == sysNr && gConnManList[j][2] == manType && value[i] == gConnManList[j][3]) 
			{
				manNumFound = true;
				break;	
			}
		}
		if (!manNumFound)
		{
			newItem = makeDynString(sysNr, manType, value[i]);
			DebugN("GCF: Add mananger: " + getSystemName(sysNr) + manType + ":" + value[i]);
			gConnManList[dynlen(gConnManList) + 1] = newItem;
		}
		else
		{
			manNumFound = false;
		}
	}
	for (i = 1; i <= dynlen(gConnManList); i++)
	{
		if (gConnManList[i][1] == sysNr && gConnManList[i][2] == manType && !dynContains(value, gConnManList[i][3])) 
		{
			// a (remote) manager is disconnected from PVSS so inform the local property agent
			msg = "d" + sysNr + ":" + manType + ":" + gConnManList[i][3] + ":";
			DebugN("GCF: Remove mananger: " + msg);
			sendEventToPA(msg, getSystemName());

			dynRemove(gConnManList, i);
		}
	}
}
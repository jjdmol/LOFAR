void sendEventToPA(string msg, string destSysName)
{
	sendEvent(destSysName + "__gcf_DPA_server.", msg);
}

void sendEvent(string dest, string msg)
{
	DebugTN("Msg: " + msg);
	blob event;
	blobZero(event, strlen(msg) + 1); // "+ 1" workaround for known bug in CTRL impl.
	blobSetValue(event, 0, msg, strlen(msg));
	dpSet(dest, event);
}


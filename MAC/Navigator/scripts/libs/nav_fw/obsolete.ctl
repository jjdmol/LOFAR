Originaly in gcfnav-functions.ctl:

///////////////////////////////////////////////////////////////////////////
//
// Function addColorTags
//
// adds HTML color tags to the specified string
//
///////////////////////////////////////////////////////////////////////////
string addColorTags(string text, string fgcolor = "", string bgcolor = "")
{
  LOG_TRACE("COLOR TAGS DISABLED");
  string taggedText = text;

/*
  if (strlen(fgcolor) > 0) {
    taggedText = "<color=" + fgcolor + ">" + taggedText + "</color>";
  }
  if (strlen(bgcolor) > 0) {
    taggedText = "<bgcolor=" + bgcolor + ">" + taggedText + "</bgcolor>";
  }
  LOG_TRACE(taggedText);
*/
  return taggedText;
}

///////////////////////////////////////////////////////////////////////////
//
// Function stripColorTags
//
// strips HTML color and bgcolor tags from the specified string
//
///////////////////////////////////////////////////////////////////////////
string stripColorTags(string text)
{
  LOG_TRACE("COLOR TAGS DISABLED");
  string untaggedText = text;
/*
  dyn_string tags = makeDynString("<color=", "</color", "<bgcolor=", "</bgcolor");
  int tagBegin, tagEnd, i;
  for (i = 1; i <= dynlen(tags); i++) {
    tagBegin = strpos(untaggedText, tags[i]);
    if (tagBegin >= 0) {
      int tagEnd = strpos(untaggedText, ">");
      if (tagEnd > tagBegin) {
        string beginPart, endPart;
        beginPart = "";
        endPart = "";
        if (tagBegin > 0) {
          beginPart = substr(untaggedText, 0, tagBegin);
        }
        endPart = substr(untaggedText, tagEnd + 1);
        untaggedText = beginPart + endPart;
      }
    }
  }
*/
 return untaggedText;
}

//////////////////////////////////////////////////////////////////////////////////
//
// Function navConfigMessageWarning (message)
//
// Used to display a message to the end-user
// 
///////////////////////////////////////////////////////////////////////////////////
void navConfigMessageWarning(string message)
{
	ChildPanelOnCentralModal(navConfigGetViewsPath() + "MessageWarning.pnl", 
							 "Warning", makeDynString("$1:" + message));
}
<?php
// Date in the past
header("Expires: Fri, 30 Oct 1998 14:19:41 GMT");
// always modified
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
// HTTP/1.1
header("cache-Control: no-store, no-cache, must-revalidate");
header("cache-Control: post-check=0, pre-check=0", false);
// HTTP/1.0
header("Pragma: no-cache"); 

?>

<!DOCTYPE HTML>
<?php
include('calendar.inc.php');

$current_year = date('Y');
$current_month = date('n');
$current_week = date('W');
$month_names = array("January","February","March","April","May","June","July","August","September","October","November","December");

if (isset($_GET["week"]) & !empty($_GET["week"])) {
  if ($_GET["week"] >= 1 & $_GET["week"] <= 53) {
    $URL_week = $_GET["week"];
    }
}
if (isset($_GET["month"]) & !empty($_GET["month"])) {
  if ($_GET["month"] >= 1 & $_GET["month"] <= 12) {
    $URL_month = $_GET["month"];
  }
}
if (isset($_GET["year"]) & !empty($_GET["year"])) {
  $URL_year = $_GET["year"];
}

if (isset($URL_year)) { $selected_year = $URL_year; } else { $selected_year = $current_year; }
if (isset($URL_month)) { $selected_month = $URL_month; } else { $selected_month = $current_month; }
if (isset($URL_week)) {
  $selected_week = $URL_week;
    // create the calendar
      $cal = new CALENDAR($selected_year, $selected_month);
      $cal->offset = 2;
      $cal->weekNumbers = 1;
      $cal->tFontSize = 11;
      $cal->hFontSize = 9;
      $cal->dFontSize = 9;
      $cal->wFontSize = 9;
      $cal->selectedWeek = $selected_week;
      $html_calendar_code = $cal->create();
} 
else if (isset($URL_month)) { // no week specified but month is specified -> select first week of this month (later on)
  if ($selected_month > 0 & $selected_month < 13) {
    // create the calendar
      $cal = new CALENDAR($selected_year, $selected_month);
      $cal->offset = 2;
      $cal->weekNumbers = 1;
      $cal->tFontSize = 11;
      $cal->hFontSize = 9;
      $cal->dFontSize = 9;
      $cal->wFontSize = 9;
      $cal->selectedWeek = -1;
      $html_calendar_code = $cal->create(); 

      $selected_week = $cal->selectedWeek; // make selected week equal to first week of the selected month
  }
}
else { // select current week
  $selected_week = intval($current_week);
    // create the calendar
      $cal = new CALENDAR($selected_year, $selected_month);
      $cal->offset = 2;
      $cal->weekNumbers = 1;
      $cal->tFontSize = 11;
      $cal->hFontSize = 9;
      $cal->dFontSize = 9;
      $cal->wFontSize = 9;
      $cal->selectedWeek = $selected_week;
      $html_calendar_code = $cal->create();
}
?>

<HTML>
<HEAD>
<META HTTP-EQUIV="CACHE-CONTROL" CONTENT="NO-CACHE; NO-STORE">
<META HTTP-EQUIV="PRAGMA" CONTENT="NO-CACHE">
<META HTTP-EQUIV="EXPIRES" CONTENT="-1">
<META HTTP-EQUIV="CONTENT-TYPE" CONTENT="text/html; charset=UTF-8">
<META NAME="KEYWORDS" CONTENT="LOFAR, LOFAR schedule, LOFAR Scheduler, ASTRON, LOFAR observations, Astronomy">
<META NAME="ROBOTS" CONTENT="INDEX,NOFOLLOW,NOARCHIVE">

<?php
echo "<LINK rel=\"stylesheet\" href=\"../css/mainstyle.css\" type=\"text/css\">\n";
echo $cal->set_styles();
echo "\n<TITLE>\nLOFAR Schedule for week " . $selected_week . " (" . $selected_year . ")\n</TITLE>\n";
?>
</HEAD>
<BODY>
	
<div id="dhtmltooltip"></div>

<script type="text/javascript">

var offsetxpoint=-60 //Customize x offset of tooltip
var offsetypoint=20 //Customize y offset of tooltip
var ie=document.all
var ns6=document.getElementById && !document.all
var enabletip=false
if (ie||ns6)
var tipobj=document.all? document.all["dhtmltooltip"] : document.getElementById? document.getElementById("dhtmltooltip") : ""

function ietruebody(){
return (document.compatMode && document.compatMode!="BackCompat")? document.documentElement : document.body
}

function ddrivetip(thetext, thecolor, thewidth){
if (ns6||ie){
if (typeof thewidth!="undefined") tipobj.style.width=thewidth+"px"
if (typeof thecolor!="undefined" && thecolor!="") tipobj.style.backgroundColor=thecolor
tipobj.innerHTML=thetext
enabletip=true
return false
}
}

function positiontip(e){
if (enabletip){
var curX=(ns6)?e.pageX : event.clientX+ietruebody().scrollLeft;
var curY=(ns6)?e.pageY : event.clientY+ietruebody().scrollTop;
//Find out how close the mouse is to the corner of the window
var rightedge=ie&&!window.opera? ietruebody().clientWidth-event.clientX-offsetxpoint : window.innerWidth-e.clientX-offsetxpoint-20
var bottomedge=ie&&!window.opera? ietruebody().clientHeight-event.clientY-offsetypoint : window.innerHeight-e.clientY-offsetypoint-20

var leftedge=(offsetxpoint<0)? offsetxpoint*(-1) : -1000

//if the horizontal distance isn't enough to accomodate the width of the context menu
if (rightedge<tipobj.offsetWidth)
//move the horizontal position of the menu to the left by it's width
tipobj.style.left=ie? ietruebody().scrollLeft+event.clientX-tipobj.offsetWidth+"px" : window.pageXOffset+e.clientX-tipobj.offsetWidth+"px"
else if (curX<leftedge)
tipobj.style.left="5px"
else
//position the horizontal position of the menu where the mouse is positioned
tipobj.style.left=curX+offsetxpoint+"px"

//same concept with the vertical position
if (bottomedge<tipobj.offsetHeight)
tipobj.style.top=ie? ietruebody().scrollTop+event.clientY-tipobj.offsetHeight-offsetypoint+"px" : window.pageYOffset+e.clientY-tipobj.offsetHeight-offsetypoint+"px"
else
tipobj.style.top=curY+offsetypoint+"px"
tipobj.style.visibility="visible"
}
}

function hideddrivetip(){
if (ns6||ie){
enabletip=false
tipobj.style.visibility="hidden"
tipobj.style.left="-1000px"
tipobj.style.backgroundColor=''
tipobj.style.width=''
}
}

document.onmousemove=positiontip

</script>

<DIV class="style_container">
<DIV id="MainHeader"></DIV>


<?php
echo "<DIV style='position: absolute; left: 400px; top: 125px; font-weight: bold; font-size: 18px;'>";
echo "LOFAR Schedule for week " . $selected_week . " (" . $selected_year . ")";
    
echo "</DIV>\n<DIV ID='ScheduleArea'>\n<DIV id='calendar' style='position: absolute; left: 925px; top: 170px; width: 170px;'>\n";

// first find out which years are available by examining available year directories
$dir = './';
$year_dirs = scandir($dir);
foreach ($year_dirs as $i => $value) {
  if (!(is_dir($value)) | (strlen($value) != 4)) { // only accept 4 digits year directory
    unset($year_dirs[$i]);
  }
  }
  
  if (!empty($year_dirs)) {
      // month dropdown list
      echo "<FORM STYLE='float: left;' METHOD='get' ACTION='";
      echo $_SERVER['PHP_SELF']; 
      echo "'>\n<SELECT NAME='month' onchange='this.form.submit()'>\n";
      foreach ($month_names as $i => $month_name) {
	echo "<OPTION VALUE='";
	echo $i+1;
	echo "'";
	if ($i+1 == $selected_month) {
	  echo " SELECTED";
	}
	echo ">";
	echo $month_name;
	echo "</OPTION>\n";
      }
      echo "</SELECT>\n";
      // year pulldown list
      echo "<SELECT NAME='year' onchange='this.form.submit()'>\n";
      foreach ($year_dirs as $i => $year_nr) {
	echo "<OPTION VALUE='";
	echo $year_nr;
	echo "'";
	if ($year_nr == $selected_year) {
	  echo " SELECTED";
	}
	echo ">";
	echo $year_nr;
	echo "</OPTION>\n";
      }
      echo "</SELECT>\n";
  echo "</FORM><BR><BR>\n";

    $sel_week = sprintf("%02d", $selected_week); // make sure we have two digits for the weeknumber otherwise strtotime fails
    $prev_week_time = strtotime($selected_year . "W" . $sel_week . " -1 week");
    $next_week_time = strtotime($selected_year . "W" . $sel_week . " +1 week");
    // next: the intval is done to make sure we don't pad a single digit weeknumber with a 0
    $prev_week = intval(date("W", $prev_week_time)); 
    $next_week = intval(date("W", $next_week_time));
    $prev_week_month = date("n" , $prev_week_time);
    $next_week_month = date("n" , $next_week_time);
    $prev_week_year = date("Y" , $prev_week_time);
    $next_week_year = date("Y" , $next_week_time);

    echo "<A href='" . $_SERVER['PHP_SELF'] . "?week=" . $prev_week . "&amp;year=" . $prev_week_year . "&amp;month=" . $prev_week_month . "'>&lt;&lt;</A>\n&nbsp;&nbsp;\n";
    echo "<A href='" . $_SERVER['PHP_SELF'] . "?week=" . intval(date('W')) . "&amp;year=" . $current_year . "&amp;month=" . $current_month . "'>today</A>\n&nbsp;&nbsp;\n";
    echo "<A href='" . $_SERVER['PHP_SELF'] . "?week=" . $next_week . "&amp;year=" . $next_week_year . "&amp;month=" . $next_week_month . "'>&gt;&gt;</A>\n";
    }
    else { // no data available
      echo "No data available\n";
      }
      
    echo $html_calendar_code;

      ?>
      </DIV>
      <?php
      // add the selected week schedule
   //   echo "<DIV ID='schedule' style='position: absolute; top: 150px; width: 920px;'>";
      echo "<BR>";
      $schedule = $selected_year . "/week_" . $selected_week . ".html";
  /*    DEBUG CODE
      echo "selected week:  " . $selected_week . "<BR>" .
           "selected month: " . $selected_month . "<BR>" . 
           "selected year: " . $selected_year . "<BR>" . 
	   "URL_week: " . $URL_week . "<BR>" . 
	   "URL_month: " . $URL_month . "<BR>" . 
	   "URL_year: " . $URL_year . "<BR>" . 
	   "previous week: " . $prev_week . "<BR>" . 
	   "next week: " . $next_week . "<BR>";	   
      */
      if (is_file($schedule)) {
	include ($schedule);
      } 
      else {
	echo "<BR><p style='padding-left:20px; font-weight:bold; color:red;'>Week " . $selected_week . " (" . $selected_year . ") has not been published yet</p>";
      }
      echo "</DIV>";
      ?>
      </DIV>
      </BODY>
      <HEAD>
			<META HTTP-EQUIV="CACHE-CONTROL" CONTENT="NO-CACHE; NO-STORE">
			<META HTTP-EQUIV="PRAGMA" CONTENT="NO-CACHE">
			<META HTTP-EQUIV="EXPIRES" CONTENT="-1">
			</HEAD>
      </HTML>
      

<?
/*
 +-------------------------------------------------------------------+
 |                  H T M L - C A L E N D A R   (v2.11)              |
 |                                                                   |
 | Copyright Gerd Tentler               www.gerd-tentler.de/tools    |
 | Created: May 27, 2003                Last modified: Nov. 29, 2009 |
 +-------------------------------------------------------------------+
 | This program may be used and hosted free of charge by anyone for  |
 | personal purpose as long as this copyright notice remains intact. |
 |                                                                   |
 | Obtain permission before selling the code for this program or     |
 | hosting this software on a commercial website or redistributing   |
 | this software over the Internet or in any other medium. In all    |
 | cases copyright must remain intact.                               |
 +-------------------------------------------------------------------+

 EXAMPLE #1:  $myCal = new CALENDAR();
              echo $myCal->create();

 EXAMPLE #2:  $myCal = new CALENDAR(2004, 12);
              echo $myCal->create();

 EXAMPLE #3:  $myCal = new CALENDAR();
              $myCal->year = 2004;
              $myCal->month = 12;
              echo $myCal->create();

 Returns HTML code
==========================================================================================================
*/
  error_reporting(E_WARNING);
//  $cal_ID = 0;

  class CALENDAR {
//========================================================================================================
// Configuration
//========================================================================================================
    var $tFontFace = 'Arial, Helvetica'; // title: font family (CSS-spec, e.g. "Arial, Helvetica")
    var $tFontSize = 14;                 // title: font size (pixels)
    var $tFontColor = '#FFFFFF';         // title: font color
    var $tBGColor = '#304B90';           // title: background color

    var $hFontFace = 'Arial, Helvetica'; // heading: font family (CSS-spec, e.g. "Arial, Helvetica")
    var $hFontSize = 12;                 // heading: font size (pixels)
    var $hFontColor = '#FFFFFF';         // heading: font color
    var $hBGColor = '#304B90';           // heading: background color

    var $dFontFace = 'Arial, Helvetica'; // days: font family (CSS-spec, e.g. "Arial, Helvetica")
    var $dFontSize = 14;                 // days: font size (pixels)
    var $dFontColor = '#000000';         // days: font color
    var $dBGColor = '#FFFFFF';           // days: background color

    var $wFontFace = 'Arial, Helvetica'; // weeks: font family (CSS-spec, e.g. "Arial, Helvetica")
    var $wFontSize = 12;                 // weeks: font size (pixels)
    var $wFontColor = '#FFFFFF';         // weeks: font color
    var $wBGColorSchedule = '#347C17';   // weeks: background color if has week schedule (green)
    var $wBGColor = '#C11B17';           // weeks: background color (red)

    var $saFontColor = '#0000D0';        // Saturdays: font color
    var $saBGColor = '#F6F6FF';          // Saturdays: background color

    var $suFontColor = '#D00000';        // Sundays: font color
    var $suBGColor = '#FFF0F0';          // Sundays: background color

    var $tdBorderColor = '#FF0000';      // today: border color

    var $borderColor = '#304B90';        // border color
    var $hilightColor = '#FFFF00';       // hilight color (works only in combination with link)

    var $link = '';                      // page to link to when day is clicked
    var $offset = 1;                     // week start: 0 - 6 (0 = Saturday, 1 = Sunday, 2 = Monday ...)
    var $weekNumbers = true;             // view week numbers: true = yes, false = no

//--------------------------------------------------------------------------------------------------------
// You should change these variables only if you want to translate them into your language:
//--------------------------------------------------------------------------------------------------------
    // weekdays: must start with Saturday because January 1st of year 1 was a Saturday
    var $weekdays = array('Sa', 'Su', 'Mo', 'Tu', 'We', 'Th', 'Fr');

    // months: must start with January
    var $months = array('January', 'February', 'March', 'April', 'May', 'June',
                        'July', 'August', 'September', 'October', 'November', 'December');
    // error messages
    var $error = array('Year must be 1 - 3999!', 'Month must be 1 - 12!');

//--------------------------------------------------------------------------------------------------------
// Don't change from here:
//--------------------------------------------------------------------------------------------------------
    var $year, $month, $size;
    var $mDays = array(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
    var $specDays = array();

//========================================================================================================
// Functions
//========================================================================================================
    function CALENDAR($year = '', $month = '', $week = '') {
      if($year == '' && $month == '') {
        $year = date('Y');
        $month = date('n');
      }
      else if($year != '' && $month == '') $month = 1;
      $this->year = (int) $year;
      $this->month = (int) $month;
      $this->week = (int) $week;
    }

    function set_styles() {
//      global $cal_ID;

//      $cal_ID++;
      $html = '<style> .cssTitle { ';
      if($this->tFontFace) $html .= 'font-family: ' . $this->tFontFace . '; ';
      if($this->tFontSize) $html .= 'font-size: ' . $this->tFontSize . 'px; ';
      if($this->tFontColor) $html .= 'color: ' . $this->tFontColor . '; ';
      if($this->tBGColor) $html .= 'background-color: ' . $this->tBGColor . '; ';
      $html .= '} .cssHeading { ';
      if($this->hFontFace) $html .= 'font-family: ' . $this->hFontFace . '; ';
      if($this->hFontSize) $html .= 'font-size: ' . $this->hFontSize . 'px; ';
      if($this->hFontColor) $html .= 'color: ' . $this->hFontColor . '; ';
      if($this->hBGColor) $html .= 'background-color: ' . $this->hBGColor . '; ';
      $html .= '} .cssDays { ';
      if($this->dFontFace) $html .= 'font-family: ' . $this->dFontFace . '; ';
      if($this->dFontSize) $html .= 'font-size: ' . $this->dFontSize . 'px; ';
      if($this->dFontColor) $html .= 'color: ' . $this->dFontColor . '; ';
   //   if($this->dBGColor) $html .= 'background-color: ' . $this->dBGColor . '; ';
      $html .= '} .cssWeeks { ';
      if($this->wFontFace) $html .= 'font-family: ' . $this->wFontFace . '; ';
      if($this->wFontSize) $html .= 'font-size: ' . $this->wFontSize . 'px; ';
      if($this->wFontColor) $html .= 'color: ' . $this->wFontColor . '; ';
      if($this->wBGColor) $html .= 'background-color: ' . $this->wBGColor . '; ';
      $html .= '} .cssWeeksSchedule { ';
      if($this->wFontFace) $html .= 'font-family: ' . $this->wFontFace . '; ';
      if($this->wFontSize) $html .= 'font-size: ' . $this->wFontSize . 'px; ';
      if($this->wFontColor) $html .= 'color: ' . $this->wFontColor . '; ';
      if($this->wBGColorSchedule) $html .= 'background-color: ' . $this->wBGColorSchedule . '; ';
      $html .= '} .cssSaturdays { ';
      if($this->dFontFace) $html .= 'font-family: ' . $this->dFontFace . '; ';
      if($this->dFontSize) $html .= 'font-size: ' . $this->dFontSize . 'px; ';
      if($this->saFontColor) $html .= 'color: ' . $this->saFontColor . '; ';
   //   if($this->saBGColor) $html .= 'background-color: ' . $this->saBGColor . '; ';
      $html .= '} .cssSundays { ';
      if($this->dFontFace) $html .= 'font-family: ' . $this->dFontFace . '; ';
      if($this->dFontSize) $html .= 'font-size: ' . $this->dFontSize . 'px; ';
      if($this->suFontColor) $html .= 'color: ' . $this->suFontColor . '; ';
    //  if($this->suBGColor) $html .= 'background-color: ' . $this->suBGColor . '; ';
      $html .= '} .cssHilight { ';
      if($this->dFontFace) $html .= 'font-family: ' . $this->dFontFace . '; ';
      if($this->dFontSize) $html .= 'font-size: ' . $this->dFontSize . 'px; ';
      if($this->dFontColor) $html .= 'color: ' . $this->dFontColor . '; ';
      if($this->hilightColor) $html .= 'background-color: ' . $this->hilightColor . '; ';
      $html .= '} .cssHilightRow { ';
      if($this->dFontFace) $html .= 'font-family: ' . $this->dFontFace . '; ';
      if($this->dFontSize) $html .= 'font-size: ' . $this->dFontSize . 'px; ';
      if($this->dFontColor) $html .= 'color: ' . $this->dFontColor . '; ';
      if($this->hilightColor) $html .= 'background-color: ' . $this->hilightColor . '; ';
      $html .= 'cursor: default; ';
      $html .= '} </style>';

      return $html;
    }

    function leap_year($year) {
      return (!($year % 4) && ($year < 1582 || $year % 100 || !($year % 400))) ? true : false;
    }

    function get_weekday($year, $days) {
      $a = $days;
      if($year) $a += ($year - 1) * 365;
      for($i = 1; $i < $year; $i++) if($this->leap_year($i)) $a++;
      if($year > 1582 || ($year == 1582 && $days >= 277)) $a -= 10;
      if($a) $a = ($a - $this->offset) % 7;
      else if($this->offset) $a += 7 - $this->offset;

      return $a;
    }

    function get_week($year, $days) {
      $firstWDay = $this->get_weekday($year, 0);
      return floor(($days + $firstWDay) / 7) + ($firstWDay <= 3);
    }

    function table_cell($content, $class, $date = '', $style = '') {
//      global $cal_ID;

      $size = round($this->size * 1.5);
      $html = '<td align=center width=' . $size . ' class="' . $class . '"';

      if($content != '&nbsp;' && stristr($class, 'day')) {
        $link = $this->link;

        if($this->specDays[$content]) {
          if($this->specDays[$content][0]) {
            $style .= 'background-color:' . $this->specDays[$content][0] . ';';
          }
          if($this->specDays[$content][1]) {
            $html .= ' title="' . $this->specDays[$content][1] . '"';
          }
          if($this->specDays[$content][2]) $link = $this->specDays[$content][2];
        }
        if($link) {
          $link .= strstr($link, '?') ? "&date=$date" : "?date=$date";
          $html .= ' onMouseOver="this.className=\'cssHilight\'"';
          $html .= ' onMouseOut="this.className=\'' . $class . '\'"';
          $html .= ' onClick="document.location.href=\'' . $link . '\'"';
        }
      }
      if($style) $html .= ' style="' . $style . '"';
      $html .= '>' . $content . '</td>';

      return $html;
    }

    function table_head($content) {
//      global $cal_ID;

      $cols = $this->weekNumbers ? 8 : 7;
      $html = '<tr><td colspan=' . $cols . ' class="cssTitle" align=center><b>' .
              $content . '</b></td></tr><tr>';
      for($i = 0; $i < count($this->weekdays); $i++) {
        $ind = ($i + $this->offset) % 7;
        $wDay = $this->weekdays[$ind];
        $html .= $this->table_cell($wDay, 'cssHeading');
      }
      if($this->weekNumbers) $html .= $this->table_cell('&nbsp;', 'cssHeading');
      $html .= '</tr>';

      return $html;
    }

    function viewEvent($from, $to, $color, $title, $link = '') {
      if($from > $to) return;
      if($from < 1 || $from > 31) return;
      if($to < 1 || $to > 31) return;

      while($from <= $to) {
        $this->specDays[$from] = array($color, $title, $link);
        $from++;
      }
    }

    function create() {
//      global $cal_ID;

      $this->size = ($this->hFontSize > $this->dFontSize) ? $this->hFontSize : $this->dFontSize;
      if($this->wFontSize > $this->size) $this->size = $this->wFontSize;

      list($curYear, $curMonth, $curDay) = explode('-', date('Y-m-d'));

      if($this->year < 1 || $this->year > 3999) $html = '<b>' . $this->error[0] . '</b>';
      else if($this->month < 1 || $this->month > 12) $html = '<b>' . $this->error[1] . '</b>';
      else {
        $this->mDays[1] = $this->leap_year($this->year) ? 29 : 28;
        for($i = $days = 0; $i < $this->month - 1; $i++) $days += $this->mDays[$i];

        $start = $this->get_weekday($this->year, $days);
        $stop = $this->mDays[$this->month-1];

      //  $html = $this->set_styles();
        $html = '<table border="0" cellspacing="0" cellpadding="0"><tr>';
        $html .= '<td>';
        $html .= '<table border="0" cellspacing="1" cellpadding="3">';
        $title = htmlentities($this->months[$this->month-1]) . ' ' . $this->year;
        $html .= $this->table_head($title);
        $daycount = 1;

        if(($this->year == $curYear) && ($this->month == $curMonth)) $inThisMonth = true;
        else $inThisMonth = false;

        if($this->weekNumbers || $this->week) $weekNr = $this->get_week($this->year, $days);

        while($daycount <= $stop) {
          if($this->week && $this->week != $weekNr) {
            $daycount += 7;
            $weekNr++;
            continue;
          }
          $html .= '<tr'; // start of a week row
// 	  // check if week html file exists
          if ($this->selectedWeek == -1) { // if no week was specified set selected week equal to first week of this month
             $this->selectedWeek = $weekNr;
             $html .= ' style="background-color:#6698FF"'; // ;border-color:#C11B17;
          }
          else if ($weekNr == $this->selectedWeek) { // highlight the selected week
	     $html .= ' style="background-color:#6698FF"'; // ;border-color:#C11B17;
	  }
	  
	  $schedule_week_file = $this->year . "/week_" . $weekNr . ".html";	  
	  
	  if (is_file($schedule_week_file)) {
	    $has_week_schedule = 1;
	    $html .= ' onMouseOver="this.className=\'cssHilightRow\'"';
	    $html .= ' onMouseOut="this.className=\'cssDays\'"';
	    $html .= ' onClick="document.location.href=\'' . $_SERVER['PHP_SELF'] . '?week=' . $weekNr .
	     '&amp;year=' . $this->year . '&amp;month=' . $this->month . '\'"';
	  }
	  else {
	    $has_week_schedule = 0;
	  }
	  $html .= '>';
	  
          for($i = $wdays = 0; $i <= 6; $i++) {
            $ind = ($i + $this->offset) % 7;
            if($ind == 0) $class = 'cssSaturdays';
            else if($ind == 1) $class = 'cssSundays';
            else $class = 'cssDays';

            $style = '';
            $date = $this->year . '-' . $this->month . '-' . $daycount;

            if(($daycount == 1 && $i < $start) || $daycount > $stop) $content = '&nbsp;';
            else {
              $content = $daycount;
              if($inThisMonth && $daycount == $curDay) {
                $style = 'padding:0px;border:3px solid ' . $this->tdBorderColor . ';';
              }
              else if($this->year == 1582 && $this->month == 10 && $daycount == 4) $daycount = 14;
              $daycount++;
              $wdays++;
            }
            $html .= $this->table_cell($content, $class, $date, $style);
          }

          if($this->weekNumbers) {
            if(!$weekNr) {
              if($this->year == 1) $content = '&nbsp;';
              else if($this->year == 1583) $content = 52;
              else $content = $this->get_week($this->year - 1, 365);
            }
            else if($this->month == 12 && $weekNr >= 52 && $wdays < 4) $content = 1;
            else $content = $weekNr;

	    if ($has_week_schedule == 1) {
	      $cssWeekStyle = 'cssWeeksSchedule';
	    }
	    else {
	      $cssWeekStyle = 'cssWeeks';
	    }

            $html .= $this->table_cell($content, $cssWeekStyle);
            $weekNr++;
          }
          $html .= '</tr>';
        }
        $html .= '</table></td></tr></table>';
      }
      return $html;
    }
  }
?>

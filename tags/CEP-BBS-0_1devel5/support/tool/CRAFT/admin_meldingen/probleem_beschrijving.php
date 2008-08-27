<?php
	include_once("../includes/vars.php");

	echo("<HTML>");
	echo("<HEAD></HEAD>");
	echo("<BODY bgcolor=\"#B3CCE6\">");
	
	if(isset($_GET['c']))  {
		$query = "SELECT Stand_Beschrijving, Huidige_Status FROM melding_type WHERE Meld_Type_ID= '". $_GET['c'] ."'";
		$resultaat = mysql_query($query);
		$data = mysql_fetch_array($resultaat);
		echo("<input name=\"sStatus\" id=\"sStatus\" type=\"hidden\" value=\"".$data[1]."\">");
		echo("<textarea name=\"sProb_Beschrijving\" rows=\"4\" cols=\"35\">".$data[0]."</textarea>");
		
	}
	echo("</BODY>");
	echo("</HTML>");
?>
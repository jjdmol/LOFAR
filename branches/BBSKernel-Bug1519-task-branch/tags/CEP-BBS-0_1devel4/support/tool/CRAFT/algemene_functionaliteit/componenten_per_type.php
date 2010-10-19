<?php

	session_start();

	include_once("../includes/vars.php");

	echo("<HTML>");
	echo("<HEAD></HEAD>");
	echo("<BODY bgcolor=\"#B3CCE6\">");
	
	if(isset($_GET['c']))  {
		
		$query = "SELECT Comp_Lijst_ID, Comp_Naam, Comp_Locatie FROM comp_lijst WHERE Comp_Type_ID = '". $_GET['c'] ."'";
		$resultaat = mysql_query($query);

		echo("<table border =\"1\">");
			
		while ($data = mysql_fetch_array($resultaat)) { 

			$query = "SELECT Loc_Naam FROM comp_locatie WHERE Locatie_ID ='". $data['Comp_Locatie'] ."'";
			$res = mysql_query($query);
			$row = mysql_fetch_array($res);

			echo("<tr><td>" .substr($data['Comp_Naam'], 0, 40) . "...</td><td>".$row['Loc_Naam']."</td><td><a href=\"../". $_SESSION['huidige_pagina']."&o=1&c=". $data['Comp_Lijst_ID'] ."\" target=\"_top\">Meer</a></td></tr>");
		}
		echo("</table>");
	}
	echo("</BODY>");
	echo("</HTML>");
?>
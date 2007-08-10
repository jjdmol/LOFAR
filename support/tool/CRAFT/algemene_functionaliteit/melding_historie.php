<?php

	session_start();

	include_once("../includes/vars.php");

	echo("<HTML>");
	echo("<HEAD></HEAD>");
	echo("<BODY");

	if(!isset($_GET['q']))
		echo(" bgcolor=\"#B3CCE6\">");
	else echo(">");
	
	if(isset($_GET['c']))  {
		
		$query = "SELECT Laatste_Melding FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
		$resultaat = mysql_query($query);
		$data = mysql_fetch_array($resultaat);

		$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID ='". $data[0] ."'";
		$resultaat = mysql_query($query);
		$data = mysql_fetch_array($resultaat);

		
		//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
		$gedeeldveld=split(" ",$data['Meld_Datum']);
		//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
		$datum = split("-",$gedeeldveld[0]);


		echo("<table border =\"1\">");
		echo("<tr><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td>");
		$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID ='". $data['Meld_Type_ID'] ."'";
		$res = mysql_query($query);
		$row = mysql_fetch_array($res);
		echo(substr($row['Melding_Type_Naam'], 0, 30));
		echo("</td><td>" . substr($data[6], 0, 30) . "...</td><td><a href=\"../algemene_functionaliteit/melding_info.php?c=".$data['Meld_Lijst_ID']."\" target=\"_blank\">Meer</a></td></tr>");
			
		while ($data['Voorgaande_Melding'] != 1) { 
			$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID ='". $data['Voorgaande_Melding'] ."'";
			$resultaat = mysql_query($query);
			$data = mysql_fetch_array($resultaat);

			//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
			$gedeeldveld=split(" ",$data['Meld_Datum']);
			//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
			$datum = split("-",$gedeeldveld[0]);
		
			echo("<tr><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td>");
			$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID ='". $data['Meld_Type_ID'] ."'";
			$res = mysql_query($query);
			$row = mysql_fetch_array($res);
			echo(substr($row['Melding_Type_Naam'], 0, 30));
			echo("</td><td>" .substr($data[6], 0, 30) . "...</td><td><a href=\"../algemene_functionaliteit/melding_info.php?c=".$data['Meld_Lijst_ID']."\" target=\"_blank\">Meer</a></td></tr>");
		}
		echo("</table>");
	}
	echo("</BODY>");
	echo("</HTML>");
?>
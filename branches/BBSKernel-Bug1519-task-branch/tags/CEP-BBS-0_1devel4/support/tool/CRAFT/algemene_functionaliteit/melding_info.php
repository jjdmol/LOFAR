<html>
	<head></head>
	<body>
		<?php
			include_once("../includes/vars.php");			
			
			if (isset($_GET['c'])) {
				$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID= '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				
				echo("<h3>Specificaties Melding ".$data['Meld_Lijst_ID']." </h3>");
				echo("<table>");
				echo("<tr><td>Melding ID:</td><td>" . $data['Meld_Lijst_ID'] . "</td></tr>");
				echo("<tr><td>Type melding:</td><td>");
				$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID= '". $data['Meld_Type_ID'] ."'";
				$resultaat = mysql_query($query);
				$data2 = mysql_fetch_array($resultaat);
				echo($data2[0] . "</td></tr>");
				echo("<tr><td>Melding hoort bij component:</td><td>");
				$query = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID= '". $data['Comp_Lijst_ID'] ."'";
				$resultaat = mysql_query($query);
				$data2 = mysql_fetch_array($resultaat);
				echo($data2[0] . "</td></tr>");

				//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
				$gedeeldveld=split(" ",$data['Meld_Datum']);
				//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
				$datum = split("-",$gedeeldveld[0]);
				//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
				$tijd = split(":",$gedeeldveld[1]);
				echo("<tr><td>Datum gemeld:</td><td>" . $datum[2] ."-". $datum[1] ."-". $datum[0] ." ". $tijd[0] .":". $tijd[1] . "</td></tr>");
				
				echo("<tr><td>Melding geplaatst door:</td><td>");
				$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID= '". $data['Gemeld_Door'] ."'";
				$resultaat = mysql_query($query);
				$data2 = mysql_fetch_array($resultaat);
				echo($data2[0] . "</td></tr>");
				
				echo("<tr><td>Status component na plaatsen melding:</td><td>");
				$query = "SELECT Status FROM status WHERE Status_ID= '". $data['Huidige_Status'] ."'";
				$resultaat = mysql_query($query);
				$data2 = mysql_fetch_array($resultaat);
				echo($data2[0] . "</td></tr>");
				
				echo("<tr><td>Beschrijving van het probleem:</td><td>" . $data['Prob_Beschrijving'] . "</td></tr>");
				echo("<tr><td>Oplossing van het probleem:</td><td>" . $data['Prob_Oplossing'] . "</td></tr>");
				echo("<tr><td>Melding behandeld door:</td><td>");
				$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID= '". $data['Behandeld_Door'] ."'";
				$resultaat = mysql_query($query);
				$data2 = mysql_fetch_array($resultaat);
				echo($data2[0] . "</td></tr>");
				echo("<tr><td>Melding afgehandeld:</td><td>");
				if ($data['Afgehandeld'] == 1) echo('Ja');
				else echo ('Nee');
				echo("</td></tr>");
				echo("<tr><td>Voorgaande melding:</td><td>" . $data['Voorgaande_Melding'] . "</td></tr>");
				echo("</table>");
			}
		
		
		?>
	</body>
</html>
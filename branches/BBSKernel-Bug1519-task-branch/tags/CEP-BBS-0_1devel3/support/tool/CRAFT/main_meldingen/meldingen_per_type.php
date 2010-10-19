<?php

	session_start();
	if (isset($_SESSION['laatste_inlog'])) {
		include_once("../includes/vars.php");
?>

		<html>
			<head></head>
			<body bgcolor="#B3CCE6">
	
			<?php
				$query = "SELECT * FROM melding_lijst WHERE Meld_Type_ID = '".$_GET['c']."' AND Meld_Lijst_ID > '1' ORDER BY Meld_Datum DESC";
			  $resultaat = mysql_query($query);

				echo("<table border=\"1\">");
				while ($data = mysql_fetch_array($resultaat)) {
					echo("<tr><td>".$data['Meld_Lijst_ID']."</td>");
					
  				//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
					$gedeeldveld=split(" ",$data['Meld_Datum']);
					//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
					$datum = split("-",$gedeeldveld[0]);
				
					echo("<td>". $datum[2] ."-". $datum[1] ."-". $datum[0] ."</td>");
					echo("<td>". substr($data['Prob_Beschrijving'], 0, 40) ."</td><td>");
					$query = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '".$data['Comp_Lijst_ID']."'";
				  $res = mysql_query($query);
				  $row = mysql_fetch_array($res);
					echo(substr($row['Comp_Naam'], 0, 40). "</td>");
					echo("<td><a href=\"../algemene_functionaliteit/melding_info.php?c=". $data['Meld_Lijst_ID']."\" target=\"_blank\">Info</a></td></tr>");
				}
				echo("</table>");
			?>

			</body>
		</html>

<?php
	}
?>
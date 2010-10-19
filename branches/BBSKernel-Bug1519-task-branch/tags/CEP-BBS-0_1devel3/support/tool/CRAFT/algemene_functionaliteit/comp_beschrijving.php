<?php
	session_start();
?>

<html>
	<head></head>
	<body>
		<?php
			include_once("../includes/vars.php");
			
			if (isset($_GET['c'])) {
				$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
			  $res = mysql_query($query);
				$data = mysql_fetch_array($res);
				echo("<h3>". $data['Comp_Naam']."</h3>");

				echo("<table border=\"0\">");
				echo("<tr><td>Status component:</td><td>");
				$query = "SELECT Status FROM status WHERE Status_ID IN (SELECT Huidige_Status FROM melding_lijst WHERE Meld_Lijst_ID = '".$data['Laatste_Melding']."')";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);

				echo($row[0]."</td></tr>");

				echo("<tr><td>Parent component:</td><td>");
				$query = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID ='".$data['Comp_Parent']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Comp_Naam'] ."</td><td>");
				if ($data['Comp_Parent'] != 1)
					echo("<a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/comp_beschrijving.php?c=". $data['Comp_Parent']."\" target=\"_blank\">Meer info</a></td></tr>");
				else echo("&nbsp</td></tr>");

				echo("<tr><td>Type component:</td><td>");
				$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type ='".$data['Comp_Type_ID']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Type_Naam'] ."</td><td><a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/comp_type.php?c=". $data['Comp_Type_ID']."\" target=\"_blank\">Meer info</a></td></tr>");
				
				echo("<tr><td>Locatie component:</td><td>");
				$query = "SELECT Loc_Naam FROM comp_locatie WHERE Locatie_ID ='".$data['Comp_Locatie']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Loc_Naam'] ."</td><td><a href=\"../".$_SESSION['pagina'] ."algemene_functionaliteit/locatie.php?c=".$data['Comp_Locatie']."\" target=\"_blank\">Meer info</a></td></tr>");
				
				echo("<tr><td>Verantwoordelijke:</td><td>");
				$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID ='".$data['Comp_Verantwoordelijke']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['inlognaam'] ."</td><td><a href=\"../".$_SESSION['pagina'] ."algemene_functionaliteit/gebruiker.php?c=".$data['Comp_Verantwoordelijke']."\" target=\"_blank\">Meer info</a></td></tr>");

				echo("<tr><td>Fabricant:</td><td>");
				$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Contact_Fabricant']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Contact_Naam'] ."</td><td><a href=\"../".$_SESSION['pagina'] ."algemene_functionaliteit/contact.php?c=".$data['Contact_Fabricant']."\" target=\"_blank\">Meer info</a></td></tr>");

				//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
				$gedeeldveld=split(" ",$data['Fabricatie_Datum']);
				//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
				$datum = split("-",$gedeeldveld[0]);
				
				echo("<tr><td>Fabricatiedatum:</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. " (".$gedeeldveld[1].")</td><td>&nbsp</td></tr>");
				echo("<tr><td>Leverancier:</td><td>");
				$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Contact_Leverancier']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Contact_Naam']  ."</td><td><a href=\"../".$_SESSION['pagina'] ."algemene_functionaliteit/contact.php?c=".$data['Contact_Leverancier']."\" target=\"_blank\">Meer info</a></td></tr>");

				//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
				$gedeeldveld=split(" ",$data['Lever_Datum']);
				//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
				$datum = split("-",$gedeeldveld[0]);

				echo("<tr><td>Leverdatum:</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. " (".$gedeeldveld[1].")</td><td>&nbsp</td></tr>");
				echo("</table>");

				$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				if ($row[0] != 0){
   				echo("<br>Meldingen behorende bij dit component:<br>");
   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"../". $_SESSION['pagina'] ."algemene_functionaliteit/melding_historie.php?c=".$_GET['c']. "&q=1\" width=\"450\" height=\"175\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
   			}
				else 
 					echo("<br>Er zijn bij dit component geen meldingen gevonden.<br>");
			}
		?>
	</body>
</html>
<?php

	session_start();
	if (isset($_SESSION['laatste_inlog'])) {
		include_once("../includes/vars.php");
?>

		<html>
			<head></head>
			<body bgcolor="#B3CCE6">
	
			<?php
				$query = "SELECT * FROM melding_lijst WHERE Meld_Datum > '".$_SESSION['laatste_inlog']."' ORDER BY Meld_Datum desc";
			  $resultaat = mysql_query($query);

				echo("<table border=\"1\">");
				while ($data = mysql_fetch_array($resultaat)) {
					echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>");
					$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID ='".$data['Meld_Type_ID']."'";
				  $res = mysql_query($query);
				  $row = mysql_fetch_array($res);
					echo(substr($row['Melding_Type_Naam'], 0, 40));
					echo("...</td><td>");
					$query = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '".$data['Comp_Lijst_ID']."'";
				  $res = mysql_query($query);
				  $row = mysql_fetch_array($res);
					echo(substr($row['Comp_Naam'], 0, 40). "</td>");
					
					
					echo("<td>");
					$id = substr($_SESSION['huidige_pagina'], (strpos ($_SESSION['huidige_pagina'], "p=") + 2 ), 1);
					$temp = $_SESSION['huidige_pagina'];
					$temp = str_replace("p=".$id  , "p=3", $temp);
					$_SESSION['type_overzicht'] = 1;

					//p=1 > p=3
					echo("  <a href=\"../". $temp."&bypass=1&c=".$data['Comp_Lijst_ID'] . "&m=" . $data['Meld_Lijst_ID']."\" target=\"_top\">Info</a></td></tr>");
				}
				echo("</table>");
			?>

			</body>
		</html>

<?php
	}
?>
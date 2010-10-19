<?php

	session_start();
	if (isset($_SESSION['laatste_inlog'])) {
		include_once("../includes/vars.php");
?>
		<html>
			<head></head>
			<body bgcolor="#B3CCE6">
	
			<?php
				$query = "SELECT * FROM comp_lijst WHERE Laatste_Melding in";
				$query = $query . "(SELECT Meld_Lijst_ID FROM melding_lijst WHERE Meld_Datum > ";
				$query = $query . "'".$_SESSION['laatste_inlog']."' AND Voorgaande_Melding = 1 ORDER BY Meld_Datum DESC)";
			  $resultaat = mysql_query($query);
				echo("<table border=\"1\">");
				while ($data = mysql_fetch_array($resultaat)) {
					$query2 = "SELECT Loc_Naam FROM comp_locatie WHERE Locatie_ID ='". $data['Comp_Locatie']."'";
				  $res = mysql_query($query2);
					$row = mysql_fetch_array($res);
					
					echo("<tr><td>".$data['Comp_Lijst_ID']."</td><td>".substr($data['Comp_Naam'], 0, 40)."...</td><td>". $row['Loc_Naam'] ."</td>");

					$id = substr($_SESSION['huidige_pagina'], (strpos ($_SESSION['huidige_pagina'], "p=") + 2 ), 1);
					$temp = $_SESSION['huidige_pagina'];
					$temp = str_replace("p=".$id  , "p=2", $temp);

					//p=1 > p=2
					echo("<td><a href=\"../". $temp."&bypass=1&c=". $data['Comp_Lijst_ID'] ."\" target=\"_top\">Info</a></td></tr>");
				}
				echo("</table>");
			?>
				
			</body>
		</html>
<?php
	}
?>
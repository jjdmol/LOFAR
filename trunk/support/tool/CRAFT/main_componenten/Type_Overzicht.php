<?php

	session_start();
	if (isset($_SESSION['laatste_inlog'])) {
		include_once("../includes/vars.php");
?>

		<html>
			<head></head>
			<body bgcolor="#B3CCE6">
	
			<?php
			
				$query = "SELECT * FROM comp_type WHERE Aanmaak_Datum > '".$_SESSION['laatste_inlog']."'  ORDER BY Aanmaak_Datum desc ";
			  $resultaat = mysql_query($query);

				echo("<table border=\"1\">");
				while ($data = mysql_fetch_array($resultaat)) {
					echo("<tr><td>". $data['Comp_Type'] ."</td><td>".substr($data['Type_Naam'], 0, 40)."...</td>");

					$id = substr($_SESSION['huidige_pagina'], (strpos ($_SESSION['huidige_pagina'], "p=") + 2 ), 1);
					$temp = $_SESSION['huidige_pagina'];
					$temp = str_replace("p=".$id  , "p=2", $temp);
					//p=1 > p=2
					echo("<td><a href=\"../". $temp."&bypass=2&o=2&c=". $data['Comp_Type']."\" target=\"_top\">Info</a></td></tr>");
				}
				echo("</table>");

			?>

			</body>
		</html>

<?php
	}
?>
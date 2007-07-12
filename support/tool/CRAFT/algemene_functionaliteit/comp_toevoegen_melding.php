<html>
	<head></head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");

			if (isset($_GET['c'])) {
				$query = "SELECT Stand_Beschrijving, Huidige_Status FROM melding_type WHERE Meld_Type_ID = '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				echo ("<textarea id=\"sMelding\" name=\"sMelding\" rows=\"4\" cols=\"35\">" .$data[0]."</textarea>");
				echo ("<input id=\"sStatus\" name=\"sStatus\" type=\"hidden\" value=\"". $data[1] ."\">");
			}
		?>
	</body>
</html>
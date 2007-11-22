<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");

			//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
			date_default_timezone_set ("Europe/Amsterdam");
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
			  $query = "SELECT Geleverd_Door FROM comp_type WHERE Comp_Type = '".$_GET['c']."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				if (isset($_GET['n']) && $_GET['n'] != -1)
					$leverancier = $_GET['n'];
				else
					$leverancier = $data['Geleverd_Door'];
					
			  $query = "SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1";
				$resultaat = mysql_query($query);
				echo("<form name=\"fTesttt\">\r\n");
				echo("<select id=\"sComp_Leverancier\" name=\"sComp_Leverancier\">\r\n");
				while ($data = mysql_fetch_array($resultaat)) {
					echo("<option value=\"". $data['Contact_ID'] ."\"");
					if ($leverancier == $data['Contact_ID']) echo('SELECTED');
					echo(">". $data['Contact_Naam'] ."</option>\r\n");
				}
				echo("</select>");

				echo("&nbspLeverdatum:&nbsp<input name=\"sLeverdatum\" id=\"sLeverdatum\" type=\"text\" size=\"8\" maxlength=\"10\" value=\"". $_GET['d'] ."\">");
				echo("&nbsp<input name=\"sLevertijd\" id=\"sLevertijd\" type=\"text\" size=\"2\" maxlength=\"5\" value=\"". $_GET['t'] ."\">");
				echo("</form>");
			}
		?>
	</body>
</html>
<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
			  $query = "SELECT Gefabriceerd_Door FROM comp_type WHERE Comp_Type = '".$_GET['c']."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				$fabricant = $data['Gefabriceerd_Door'];
			  $query = "SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1";
				$resultaat = mysql_query($query);
				echo("<form name=\"fTest\">\r\n");
				echo("<select id=\"sComp_Fabricant\" name=\"sComp_Fabricant\">\r\n");
				while ($data = mysql_fetch_array($resultaat)) {
					echo("<option value=\"". $data['Contact_ID'] ."\"");
					if ($fabricant == $data['Contact_ID']) echo('SELECTED');
					echo(">". $data['Contact_Naam'] ."</option>\r\n");
				}
				echo("</select></form>");
			}
	
		?>
	</body>
</html>
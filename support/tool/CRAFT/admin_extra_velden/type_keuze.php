<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			
			//mogelijkheden "c": 1 = Component type, 2 = melding type
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
				if($_GET['c'] == 1) 
					$query = "SELECT Comp_Type, Type_Naam FROM comp_type WHERE Comp_Type >1";
				else if ($_GET['c'] == 2)
					$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
				$resultaat = mysql_query($query);
				echo("<form name=\"fTest\">\r\n");
				echo("<select id=\"sType\" name=\"sType\">\r\n");
				while ($data = mysql_fetch_array($resultaat)) {
					echo("<option value=\"". $data[0] ."\"");
					if (isset($_GET['p']) && $_GET['p'] == $data[0]) echo('SELECTED');
					echo(">". $data[1] ."</option>\r\n");
				}
				echo("</select></form>");
			}	
		?>
	</body>
</html>
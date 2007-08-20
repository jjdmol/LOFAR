<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
			  $query = "SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Type_ID in (SELECT Type_Parent FROM comp_type WHERE Comp_Type = '".$_GET['c']."')";

		  	$num_rows = mysql_num_rows(mysql_query($query));		
		  	if ($num_rows == 0)
					echo("No parent components available");
		  	else {
					$resultaat = mysql_query($query);

					echo("<form name=\"fTest\">\r\n");
					if (isset($_GET['n']) && $_GET['n'] > 0)
						$selectie = $_GET['n'];
					else 
						$selectie = 'SELECTED';

					echo("<select id=\"sComp_Parent\" name=\"sComp_Parent\">\r\n");
					while ($data = mysql_fetch_array($resultaat)) {
						echo("<option value=\"". $data['Comp_Lijst_ID'] ."\"");
						if ($data['Comp_Lijst_ID'] == $selectie || $selectie == 'SELECTED') {
							echo(' SELECTED');
							$selectie = -1;
						}
						echo(">". $data['Comp_Naam'] ."</option>\r\n");
					}
					echo("</select></form>");
				}
			}
		?>
	</body>
</html>
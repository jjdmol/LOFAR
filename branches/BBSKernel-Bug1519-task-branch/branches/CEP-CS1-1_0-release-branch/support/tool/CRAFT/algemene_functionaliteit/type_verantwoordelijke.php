<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			include_once("../algemene_functionaliteit/globale_functies.php");
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
				$Collectie = Check_groepen($_GET['c']);
				echo("<form name=\"fTest\">\r\n");
				
				if ($_GET['s'] == -1) {
					$query = "SELECT Type_verantwoordelijke FROM comp_type WHERE Comp_Type = '". $_GET['c'] ."'";
					$resultaat = mysql_query($query);
					$data = mysql_fetch_array($resultaat);
					$selectie = $data[0];
				}
				else $selectie = $_GET['s'];
				
				echo("<select id=\"sVerantwoordelijke\" name=\"sComp_Fabricant\">\r\n");

				for ($i = 0; $i < count($Collectie); $i++){
				  $query = "SELECT Werknem_ID, inlognaam FROM gebruiker WHERE Groep_ID = '". $Collectie[$i] ."'";
					$resultaat = mysql_query($query);
					while ($data = mysql_fetch_array($resultaat)) {
						echo("<option value=\"". $data['Werknem_ID'] ."\"");
						if ($selectie == $data['Werknem_ID']) echo(" SELECTED");
						echo(">". $data['inlognaam'] ."</option>\r\n");
					}
				}
				echo("</select></form>");			
			}
		?>	
		</select>
	</body>
</html>
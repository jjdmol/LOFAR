<html>
	<head></head>
	<body>
		<?php
			include_once("../includes/vars.php");			
			
			if (isset($_GET['c'])) {
				$query = "SELECT * FROM comp_type WHERE Comp_Type='". $_GET['c'] ."'";
			  $resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
	
				echo("<h3>".$data['Type_Naam']."</h3>");
	
				echo("<table border=\"0\">");
				echo("<tr><td>Aangemaakt door:</td><td>");
				$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID ='".$data['Aangemaakt_Door']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['inlognaam'] ."</td></tr>");
	
				//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
				$gedeeldveld=split(" ",$data['Aanmaak_Datum']);
				//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
				$datum = split("-",$gedeeldveld[0]);
	
				echo("<tr><td>Aangemaakt op:</td><td>".$datum[2] ."-". $datum[1] ."-". $datum[0]. " (".$gedeeldveld[1].")</td><td>&nbsp</td></tr>");
				echo("<tr><td>Structuur entry:</td><td>");
				if ($data['Structuur_Entry'] == 1) echo("Ja");
				else echo("Nee");
				echo("</td></tr>");
	
				echo("<tr><td>Fabricant:</td><td>");
				$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Gefabriceerd_Door']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Contact_Naam'] ."</td></tr>");
	
				echo("<tr><td>Leverancier:</td><td>");
				$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Geleverd_Door']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['Contact_Naam']  ."</td></tr>");
	
				echo("<tr><td>Minimum aantal:</td><td>".$data['Min_Aantal']."</td></tr>");
				echo("<tr><td>Momenteel aangemaakt:</td><td>");
				$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Type_ID = '". $data['Comp_Type'] ."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row[0] ."</td></tr>");
				echo("<tr><td>Maximum aantal:</td><td>".$data['Max_Aantal']."</td></tr>");
				echo("<tr><td>Reserve minimum:</td><td>".$data['Reserve_Minimum']." (20% drempel: " . (round($data['Reserve_Minimum'] * 1.2)) .")</td></tr>");
				echo("<tr><td>Type verantwoordelijke:</td><td>");
				$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID ='".$data['Type_Verantwoordelijke']."'";
			  $res = mysql_query($query);
				$row = mysql_fetch_array($res);
				echo($row['inlognaam'] ."</td><td></tr>");
				echo("</table>");
				
				$type_array = array();
	 	  	array_push($type_array, $data['Type_Naam']);
				while ($data['Type_Parent']!= 1) {
					$query = "SELECT Type_Parent, Type_Naam FROM comp_type WHERE Comp_Type = '".$data['Type_Parent']."'";			
					$resultaat = mysql_query($query);
					$data = mysql_fetch_array($resultaat);
		 	  	array_push($type_array, $data['Type_Naam']);
				}
				
				echo("<h3>Componten type structuur:</h3>");
				for ($i = (count($type_array)-1); $i >=0;  $i--) {
					if ($i < (count($type_array)-1)) echo("&nbsp&nbsp<<&nbsp&nbsp");
					echo($type_array[$i]);
				}				
			}
		?>
	</body>
</html>
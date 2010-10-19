<html>
	<head></head>
	<body>
		<?php
			include_once("../includes/vars.php");			
			
			if (isset($_GET['c'])) {
				$query = "SELECT * FROM contact WHERE Contact_ID= '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				
				echo("<h3>Gegevens van \"".$data['Contact_Naam']."\" </h3>");
				echo("<table>");
				echo("<tr><td>Contact ID:</td><td>" . $data['Contact_ID'] . "</td></tr>");
				echo("<tr><td>Naam:</td><td>". $data['Contact_Naam'] . "</td></tr>");
				echo("<tr><td>Functie:</td><td>" . $data['Contact_Functie'] . "</td></tr>");
				echo("<tr><td>E-mail:</td><td>" . $data['Contact_Email'] . "</td></tr>");
				echo("<tr><td>Telefoon (vast):</td><td>" . $data['Contact_Telefoon_Vast'] . "</td></tr>");
				echo("<tr><td>Telefoon (mobiel):</td><td>" . $data['Contact_Telefoon_Mobiel'] . "</td></tr>");
				echo("<tr><td>Telefoon (fax):</td><td>" . $data['Contact_Fax'] . "</td></tr>");
				echo("<tr><td>Contact adres 1:</td><td>". $data['Contact_Adres1'] . "</td></tr>");
				echo("<tr><td>Contact adres 2:</td><td>". $data['Contact_Adres2'] . "</td></tr>");
				echo("<tr><td>Postcode:</td><td>" . $data['Contact_Postcode'] . "</td></tr>");
				echo("<tr><td>Plaats:</td><td>" . $data['Contact_Woonplaats'] . "</td></tr>");
				echo("</table>");
				
				
				$contacten_array = array();
	 	  	array_push($contacten_array, $data['Contact_Naam']);
				while ($data['Contact_Parent']!= 1) {
					$query = "SELECT Contact_Parent, Contact_Naam FROM contact WHERE Contact_ID = '".$data['Contact_Parent']."'";			
					$resultaat = mysql_query($query);
					$data = mysql_fetch_array($resultaat);
		 	  	array_push($contacten_array, $data['Contact_Naam']);
				}
				
				echo("<h3>Contact structuur:</h3>");
				for ($i = (count($contacten_array)-1); $i >=0;  $i--) {
					if ($i < (count($contacten_array)-1)) echo("&nbsp&nbsp<<&nbsp&nbsp");
					echo($contacten_array[$i]);
				}				
			}
		
		
		?>
	</body>
</html>
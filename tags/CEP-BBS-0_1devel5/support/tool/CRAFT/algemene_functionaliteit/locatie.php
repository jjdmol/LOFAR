<html>
	<head></head>
	<body>
		<?php
			include_once("../includes/vars.php");			
			
			if (isset($_GET['c'])) {
				$query = "SELECT * FROM comp_locatie WHERE Locatie_ID= '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				
				echo("<h3>Locatie gegevens van \"".$data['Loc_Naam']."\" </h3>");
				echo("<table>");
				echo("<tr><td>Locatie ID:</td><td>" . $data['Locatie_ID'] . "</td></tr>");
				echo("<tr><td>Locatie naam:</td><td>". $data['Loc_Naam'] . "</td></tr>");
				echo("<tr><td>Locatie adres 1:</td><td>". $data['Loc_Adres1'] . "</td></tr>");
				echo("<tr><td>Locatie adres 2:</td><td>". $data['Loc_Adres2'] . "</td></tr>");
				echo("<tr><td>Postcode:</td><td>" . $data['Loc_Postcode'] . "</td></tr>");
				echo("<tr><td>Plaats:</td><td>" . $data['Loc_Plaats'] . "</td></tr>");
				
				echo("<tr><td>Longitude graden:</td><td>". $data['Long_Graden'] . "</td></tr>");
				echo("<tr><td>Longitude minuten:</td><td>". $data['Long_Min'] . "</td></tr>");
				echo("<tr><td>Longitude seconden:</td><td>". $data['Long_Sec'] . "</td></tr>");
				echo("<tr><td>Latitude graden:</td><td>". $data['Lat_Graden'] . "</td></tr>");
				echo("<tr><td>Latitude minuten:</td><td>". $data['Lat_Min'] . "</td></tr>");
				echo("<tr><td>Latitude seconden:</td><td>". $data['Lat_Sec'] . "</td></tr>");

				echo("</table>");
			}
		?>
	</body>
</html>
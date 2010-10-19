<html>
	<head></head>
	<body>
		<?php
			include_once("../includes/vars.php");			
			
			if (isset($_GET['c'])) {
				$query = "SELECT * FROM gebruiker WHERE Werknem_ID= '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				
				echo("<h3>Gegevens van \"".$data['inlognaam']."\" </h3>");
				echo("<table>");
				echo("<tr><td>Werknemer ID:</td><td>" . $data['Werknem_ID'] . "</td></tr>");
				echo("<tr><td>Naam:</td><td>". $data['inlognaam'] . "</td></tr>");
				echo("<tr><td>E-mail:</td><td>". $data['Emailadres'] . "</td></tr>");
				echo("<tr><td>Groep:</td><td>");
				
				$query = "SELECT Groeps_Naam FROM gebruikers_groepen WHERE Groep_ID= '". $data['Groep_ID'] ."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				echo($data['Groeps_Naam'] . "</td></tr>");
				echo("</table>");
			}
		?>
	</body>
</html>
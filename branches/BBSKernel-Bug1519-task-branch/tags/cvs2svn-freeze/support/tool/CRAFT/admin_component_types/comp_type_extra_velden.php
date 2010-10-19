<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
				$query = "SELECT Kolom_ID FROM Type_Comp_Koppel_Extra WHERE Comp_Type_ID = '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				echo("<table border=\"1\">\r\n");
				//header
				echo("<tr><td>Naam:</td><td>Datatype:</td><td>Verplicht:</td></tr>");
				while ($data = mysql_fetch_array($resultaat)) {
					$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data[0] ."'";
					$result = mysql_query($query);
					$row = mysql_fetch_array($result);
					//naam
					echo("<tr><td>". $row['Veld_Naam'] ."</td>");
					//datatype
					if($row['DataType'] == 1) 		 echo("<td>Geheel getal (integer)</td>");
					else if($row['DataType'] == 2) echo("<td>Getal met decimalen (double)</td>");
					else if($row['DataType'] == 3) echo("<td>Text veld</td>");
					else if($row['DataType'] == 4) echo("<td>Datum/tijd veld (datetime)</td>");
					else if($row['DataType'] == 5) echo("<td>Bestandsverwijzing</td>");
					//verplicht
					if ($row['Is_verplicht'] == 1) 			echo("<td>Ja</td></tr>\r\n");
					else if ($row['Is_verplicht'] == 0) echo("<td>Nee</td></tr>\r\n");
				}
				echo("</table>\r\n");
			}	
		?>
	</body>
</html>
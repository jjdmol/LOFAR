<?php
	
	session_start();

?>

<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {		

				$query = "SELECT Kolom_ID FROM type_comp_koppel_extra WHERE Comp_Type_ID = '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				echo("<form>\n");
				echo("<table>\n");
				$aantal_velden = 0;
				while ($data = mysql_fetch_array($resultaat)) {
					$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data['Kolom_ID']  ."'";
					$result = mysql_query($query);
					$row = mysql_fetch_array($result);
					echo("<tr><td>". $row['Veld_Naam']);
					if ($row['Is_verplicht'] == 1)
						echo("*");
					echo("</td>"); 

					//standaard waardes ophalen
 					$query = "SELECT * FROM datatabel WHERE Data_Kolom_ID = '". $row['Data_Kolom_ID']  ."'"; 					
 					$res = mysql_query($query);
					$uitkomst = mysql_fetch_array($res);
					$waarde1 = "";
					$waarde2 = "";

					if($row['DataType'] == 1) $waarde1 = $uitkomst['Type_Integer'];
					else if($row['DataType'] == 2) $waarde1 = $uitkomst['Type_Double'];
					else if($row['DataType'] == 3) $waarde1 = $uitkomst['Type_Text'];
					else if($row['DataType'] == 4) {
  					$gedeeldveld=split(" ",$uitkomst['Type_DateTime']);
						//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
						$datum = split("-",$gedeeldveld[0]);
						//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
						$tijd = split(":",$gedeeldveld[1]);

						$waarde1 = $datum[2] ."-". $datum[1] ."-". $datum[0];
						$waarde2 = $tijd[0] .":". $tijd[1];
					}
					else if($row['DataType'] == 5) $waarde1 = $uitkomst['Type_TinyText'];

    			//1 = integer, 2 = double, 3 = text, 4 = datumtijd, 5 = bestandsverwijzing
					if ($row['DataType'] == 3) {
						echo("<td><textarea name=\"".$aantal_velden."\" id=\"".$aantal_velden."\" rows=\"3\" cols=\"30\">".$waarde1."</textarea>");
						echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
						echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
						echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
						echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						echo("</td>");
					}
					else if ($row['DataType'] == 4) {
						echo("<td><input name=\"".$aantal_velden."\" id=\"".$aantal_velden."\" type=\"text\" size=\"8\" maxlength=\"10\" value=\"".$waarde1."\"><input name=\"".($aantal_velden +1)."\" id=\"".($aantal_velden +1)."\" type=\"text\" size=\"2\" maxlength=\"5\" value=\"".$waarde2."\">");
						echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."a\">");
						echo("<input type=\"hidden\" name=\"t". ($aantal_velden +1) ."\" id=\"t". ($aantal_velden +1) ."\" value=\"". $row['DataType'] ."b\">");
						echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
						echo("<input type=\"hidden\" name=\"v". ($aantal_velden +1) ."\" id=\"v". ($aantal_velden +1) ."\" value=\"". $row['Is_verplicht'] ."\">");
						echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
						echo("<input type=\"hidden\" name=\"i". ($aantal_velden +1) ."\" id=\"i". ($aantal_velden +1) ."\" value=\"". $row['Kolom_ID'] ."\">");
						echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						echo("<input type=\"hidden\" name=\"n". ($aantal_velden +1) ."\" id=\"n". ($aantal_velden +1) ."\" value=\"". $row['Veld_Naam'] ."\">");
						echo("</td>");
						$aantal_velden++;
					}
					else if($row['DataType'] == 5){
						echo("<td>");
						if($waarde1 == -1) {
							echo("<a href=\"bestand_uploaden.php?c=".$aantal_velden."\" target=\"_blank\">Upload een bestand</a>");
							echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
							echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						}
						else {
							echo("<a href=\"".$waarde1."\" target=\"_blank\">Openen</a>");
							echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
							echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						}
						$_SESSION['bestand' . $aantal_velden] = $waarde1;
						echo("</td>");
					}
					else {
						echo("<td><input name=\"".$aantal_velden."\" id=\"".$aantal_velden."\" type=\"text\" value=\"".$waarde1."\">"); 
						echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
						echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
						echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
						echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						echo("</td>");
					}
					$aantal_velden++;

					echo("</tr>\n");
				}
				//aantal velden nodig, types velden nodig.
				echo("</table>\n");
				echo("<input type=\"hidden\" name=\"aantal\" id=\"aantal\" value=\"". $aantal_velden ."\">\n");
				echo("</form>\n");
			}
		?>
	</body>
</html>
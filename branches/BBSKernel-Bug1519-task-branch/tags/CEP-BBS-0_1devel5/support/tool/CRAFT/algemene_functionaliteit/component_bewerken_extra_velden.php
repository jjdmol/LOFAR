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

				$query = "SELECT Kolom_ID FROM comp_koppel_extra WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
				$resultaat = mysql_query($query);
				echo("<form>\n");
				echo("<table>\n");
				$aantal_velden = 0;
				$aangemaakte_velden = array();
				while ($data = mysql_fetch_array($resultaat)) {
					$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data['Kolom_ID']  ."'";
					$result = mysql_query($query);
					$row = mysql_fetch_array($result);

 	  			array_push($aangemaakte_velden, $row['Type_Beschrijving']);

					echo("<tr><td>". $row['Veld_Naam']);
					if ($row['Is_verplicht'] == 1)
						echo("*");
					echo("</td>"); 
					
					$query = "SELECT * FROM datatabel WHERE Data_Kolom_ID = '". $row['Data_Kolom_ID']  ."'";
					$rest = mysql_query($query);
				  $uitkomst = mysql_fetch_array($rest);

    			//1 = integer, 2 = double, 3 = text, 4 = datumtijd, 5 = bestandsverwijzing
					if ($row['DataType'] == 3) {
						echo("<td><textarea name=\"".$aantal_velden."\" id=\"".$aantal_velden."\" rows=\"3\" cols=\"30\">".$uitkomst['Type_Text']."</textarea>");
						echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
						echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
						echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
						echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						echo("</td>");
					}
					else if ($row['DataType'] == 4) {
  					$gedeeldveld=split(" ",$uitkomst['Type_DateTime']);
						//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
						$datum = split("-",$gedeeldveld[0]);
						//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
						$tijd = split(":",$gedeeldveld[1]);

						$waarde1 = $datum[2] ."-". $datum[1] ."-". $datum[0];
						$waarde2 = $tijd[0] .":". $tijd[1];						
						
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
						if($uitkomst['Type_TinyText'] == -1) {
							echo("<a href=\"bestand_uploaden.php?c=\" target=\"_blank\">Upload een bestand</a>");
							echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
							echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						}
						else {
							echo("<a href=\"".$uitkomst['Type_TinyText']."\" target=\"_blank\">Openen</a>");
							echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
							echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						}
						$_SESSION['bestand' . $aantal_velden] = $uitkomst['Type_TinyText'];
						echo("</td>");
					}

					else {
						if ($row['DataType'] == 1) $waarde = $uitkomst['Type_Integer'];
						else if ($row['DataType'] == 2) $waarde = $uitkomst['Type_Double'];
						else  $waarde = '';
						
						echo("<td><input name=\"".$aantal_velden."\" id=\"".$aantal_velden."\" type=\"text\" value=\"".$waarde."\">"); 
						echo("<input type=\"hidden\" name=\"t". $aantal_velden ."\" id=\"t". $aantal_velden ."\" value=\"". $row['DataType'] ."\">");
						echo("<input type=\"hidden\" name=\"v". $aantal_velden ."\" id=\"v". $aantal_velden ."\" value=\"". $row['Is_verplicht'] ."\">");
						echo("<input type=\"hidden\" name=\"i". $aantal_velden ."\" id=\"i". $aantal_velden ."\" value=\"". $row['Kolom_ID'] ."\">");
						echo("<input type=\"hidden\" name=\"n". $aantal_velden ."\" id=\"n". $aantal_velden ."\" value=\"". $row['Veld_Naam'] ."\">");
						echo("</td>");
					}
					$aantal_velden++;

					echo("</tr>\n");
				}
				
				$aan_te_maken = 0;
				//nog niet aangemaakte extra velden (die later toegevoegd zijn) toevoegen
				$query = "SELECT Kolom_ID FROM type_comp_koppel_extra WHERE Comp_Type_ID in (SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."')";
				$rest = mysql_query($query);
				while ($data = mysql_fetch_array($rest)) {
					$gevonden = false;
					//bekijken of dit record al is aangemaakt, dus array met aangemaakte waarden door itereren
					foreach ($aangemaakte_velden as $waarde) {
   					if ($data['Kolom_ID'] == $waarde) $gevonden = true;
					}
					
					//niet aangemaakt, dan hidden fields hiervoor aan maken					
					if($gevonden == false) {
						$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data['Kolom_ID']  ."'";
						$result = mysql_query($query);
						$row = mysql_fetch_array($result);
	
						echo("<tr><td>". $row['Veld_Naam']);
						if ($row['Is_verplicht'] == 1)
							echo("*");
						echo("</td>"); 
						
						$query = "SELECT * FROM datatabel WHERE Data_Kolom_ID = '". $row['Data_Kolom_ID']  ."'";
						$restt = mysql_query($query);
					  $uitkomst = mysql_fetch_array($restt);
	
	    			//1 = integer, 2 = double, 3 = text, 4 = datumtijd, 5 = bestandsverwijzing
						if ($row['DataType'] == 3) {
							echo("<td><textarea name=\"a".$aan_te_maken."\" id=\"a".$aan_te_maken."\" rows=\"3\" cols=\"30\">".$uitkomst['Type_Text']."</textarea>");
							echo("<input type=\"hidden\" name=\"at". $aan_te_maken ."\" id=\"at". $aan_te_maken ."\" value=\"". $row['DataType'] ."\">");
							echo("<input type=\"hidden\" name=\"av". $aan_te_maken ."\" id=\"av". $aan_te_maken ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"ai". $aan_te_maken ."\" id=\"ai". $aan_te_maken ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"an". $aan_te_maken ."\" id=\"an". $aan_te_maken ."\" value=\"". $row['Veld_Naam'] ."\">");
							echo("</td>");
						}
						else if ($row['DataType'] == 4) {
	  					$gedeeldveld=split(" ",$uitkomst['Type_DateTime']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);
							//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
							$tijd = split(":",$gedeeldveld[1]);
	
							$waarde1 = $datum[2] ."-". $datum[1] ."-". $datum[0];
							$waarde2 = $tijd[0] .":". $tijd[1];						
							
							echo("<td><input name=\"a".$aan_te_maken."\" id=\"a".$aan_te_maken."\" type=\"text\" size=\"8\" maxlength=\"10\" value=\"".$waarde1."\"><input name=\"a".($aan_te_maken +1)."\" id=\"a".($aan_te_maken +1)."\" type=\"text\" size=\"2\" maxlength=\"5\" value=\"".$waarde2."\">");
							echo("<input type=\"hidden\" name=\"at". $aan_te_maken ."\" id=\"at". $aan_te_maken ."\" value=\"". $row['DataType'] ."a\">");
							echo("<input type=\"hidden\" name=\"at". ($aan_te_maken +1) ."\" id=\"at". ($aan_te_maken +1) ."\" value=\"". $row['DataType'] ."b\">");
							echo("<input type=\"hidden\" name=\"av". $aan_te_maken ."\" id=\"av". $aan_te_maken ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"av". ($aan_te_maken +1) ."\" id=\"av". ($aan_te_maken +1) ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"ai". $aan_te_maken ."\" id=\"ai". $aan_te_maken ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"ai". ($aan_te_maken +1) ."\" id=\"ai". ($aan_te_maken +1) ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"an". $aan_te_maken ."\" id=\"an". $aan_te_maken ."\" value=\"". $row['Veld_Naam'] ."\">");
							echo("<input type=\"hidden\" name=\"an". ($aan_te_maken +1) ."\" id=\"an". ($aan_te_maken +1) ."\" value=\"". $row['Veld_Naam'] ."\">");
							echo("</td>");
							$aan_te_maken ++;
						}
						else if($row['DataType'] == 5){

							echo("<td>");
							if($uitkomst['Type_TinyText'] == -1) {
								echo("<a href=\"bestand_uploaden.php?a=".$aan_te_maken."\" target=\"_blank\">Upload een bestand</a>");
								echo("<input type=\"hidden\" name=\"at". $aan_te_maken ."\" id=\"at". $aan_te_maken ."\" value=\"". $row['DataType'] ."\">");
								echo("<input type=\"hidden\" name=\"av". $aan_te_maken ."\" id=\"av". $aan_te_maken ."\" value=\"". $row['Is_verplicht'] ."\">");
								echo("<input type=\"hidden\" name=\"ai". $aan_te_maken ."\" id=\"ai". $aan_te_maken ."\" value=\"". $row['Kolom_ID'] ."\">");
								echo("<input type=\"hidden\" name=\"an". $aan_te_maken ."\" id=\"an". $aan_te_maken ."\" value=\"". $row['Veld_Naam'] ."\">");
							}
							else {
								echo("<a href=\"".$uitkomst['Type_TinyText']."\" target=\"_blank\">Openen</a>");
								echo("<input type=\"hidden\" name=\"at". $aan_te_maken ."\" id=\"at". $aan_te_maken ."\" value=\"". $row['DataType'] ."\">");
								echo("<input type=\"hidden\" name=\"av". $aan_te_maken ."\" id=\"av". $aan_te_maken ."\" value=\"". $row['Is_verplicht'] ."\">");
								echo("<input type=\"hidden\" name=\"ai". $aan_te_maken ."\" id=\"ai". $aan_te_maken ."\" value=\"". $row['Kolom_ID'] ."\">");
								echo("<input type=\"hidden\" name=\"an". $aan_te_maken ."\" id=\"an". $aan_te_maken ."\" value=\"". $row['Veld_Naam'] ."\">");
							}
							$_SESSION['abestand' . $aan_te_maken] = $uitkomst['Type_TinyText'];
							echo("</td>");
						}

						else {
							if ($row['DataType'] == 1) $waarde = $uitkomst['Type_Integer'];
							else if ($row['DataType'] == 2) $waarde = $uitkomst['Type_Double'];
							else  $waarde = '';
							
							echo("<td><input name=\"a".$aan_te_maken."\" id=\"a".$aan_te_maken."\" type=\"text\" value=\"".$waarde."\">"); 
							echo("<input type=\"hidden\" name=\"at". $aan_te_maken ."\" id=\"at". $aan_te_maken ."\" value=\"". $row['DataType'] ."\">");
							echo("<input type=\"hidden\" name=\"av". $aan_te_maken ."\" id=\"av". $aan_te_maken ."\" value=\"". $row['Is_verplicht'] ."\">");
							echo("<input type=\"hidden\" name=\"ai". $aan_te_maken ."\" id=\"ai". $aan_te_maken ."\" value=\"". $row['Kolom_ID'] ."\">");
							echo("<input type=\"hidden\" name=\"an". $aan_te_maken ."\" id=\"an". $aan_te_maken ."\" value=\"". $row['Veld_Naam'] ."\">");
							echo("</td>");
						}
						$aan_te_maken++;
	
						echo("</tr>\n");						
					}
				}

				echo("</table>\n");
				echo("<input type=\"hidden\" name=\"aantal\" id=\"aantal\" value=\"". $aantal_velden ."\">\n");
				echo("<input type=\"hidden\" name=\"aantemaken\" id=\"aantemaken\" value=\"". $aan_te_maken ."\">\n");
				echo("</form>\n");
			}
		?>
	</body>
</html>
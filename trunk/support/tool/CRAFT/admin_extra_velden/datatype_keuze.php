<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			/*
1	Geheel getal (integer)
2	Getal met decimalen (double)
3	Text veld
4	Datum/tijd veld (datetime)
5	Bestandsverwijzing
*/

			//mogelijkheden "c": 1 = Component type, 2 = melding type
			if (isset($_GET['c']) && $_GET['c'] != 0) {		
				if (isset($_GET['d1'])) $waarde1 = $_GET['d1'];
				else $waarde1 = '';
				if (isset($_GET['d2'])) $waarde2 = $_GET['d2'];
				else $waarde2 = '';
				
				echo("<form name=\"fTest\">\r\n");
				if($_GET['c'] == 3)
					echo("<textarea name=\"data\" id=\"data\" rows=\"2\" cols=\"30\">".$waarde1."</textarea>");
				else if($_GET['c'] == 4) {
					echo("<input name=\"data\" id=\"datum\" type=\"text\" size=\"8\" maxlength=\"10\" value=\"".$waarde1."\">");
					echo("<input name=\"tijd\" id=\"tijd\" type=\"text\" size=\"2\" maxlength=\"5\" value=\"".$waarde2."\">");
				}
				else if($_GET['c'] == 5)
					echo("Geen standaard waarde mogelijk!");
				else 
					echo("<input name=\"data\" id=\"data\" type=\"text\" value=\"".$waarde1."\">");
				echo("</select></form>");
			}	
		?>
	</body>
</html>
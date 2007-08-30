<html>
	<head>
	</head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");

			if (isset($_GET['c']) && $_GET['c'] != 0) {		
				$query = "SELECT Schaduw_Vlag FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
			  $resultaat = mysql_query($query);
		  	$data = mysql_fetch_array($resultaat);
		  	$Comp_Schaduw = $data['Schaduw_Vlag'];

			  $query = "SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Type_ID in (SELECT Type_Parent FROM comp_type WHERE Comp_Type IN (SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."' ))";
		  	$num_rows = mysql_num_rows(mysql_query($query));		

		  	if ($num_rows == 0)
					echo("No parent components available");
		  	else {
					$resultaat = mysql_query($query);

					echo("<form name=\"fTest\" method=\"post\">\r\n");
					if(isset($_POST['sComp_Parent'])) 
						$selectie = $_POST['sComp_Parent'];
					else if (isset($_GET['n']) && $_GET['n'] > 0)
						$selectie = $_GET['n'];
					else 
						$selectie = 'SELECTED';
			  	
			  	//het geselecteerde component is geen schaduw component, dus kan deze verplaatst worden
			  	if($Comp_Schaduw == 0) {

						echo("<select id=\"sComp_Parent\" name=\"sComp_Parent\" onchange=\"	document.fTest.submit();\">\r\n");
						while ($data = mysql_fetch_array($resultaat)) {
							echo("<option value=\"". $data['Comp_Lijst_ID'] ."\"");
							if ($data['Comp_Lijst_ID'] == $selectie || $selectie == 'SELECTED') {
								echo(' SELECTED');
								$selectie = $data['Comp_Lijst_ID'];
							}
							echo(">". $data['Comp_Naam'] ."</option>\r\n");
						}
						echo("</select>");

						//het aantal aangemaakte componenten per geselecteerde type, gedeelte
						if ($selectie != -1) {
							//kijken of $selectie een schaduw ding is: dit is de parent waarnaar verplaatst wordt
							$query = "SELECT Schaduw_Vlag FROM comp_lijst WHERE Comp_Lijst_ID = '". $selectie ."'";
	    			  $resultaat = mysql_query($query);
					  	$data = mysql_fetch_array($resultaat);
					  	if ($data['Schaduw_Vlag'] == '0') {

								//maximum aantal ophalen 
								$query = "SELECT Max_Aantal FROM comp_type WHERE Comp_Type IN (SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."')";
		    			  $resultaat = mysql_query($query);
						  	$data = mysql_fetch_array($resultaat);
		    			  $maximum = $data['Max_Aantal'];
		
								//aantal aangemaakte componten van dit type ophalen
								$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Parent = '".$selectie."' AND Comp_Type_ID IN (SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."')";
		    			  $resultaat = mysql_query($query);
						  	$data = mysql_fetch_array($resultaat);
		    			  $aantal = $data[0];
		
								echo("&nbspAangemaakt: ". $aantal ." van ". $maximum);
								if ($aantal == $maximum)
									echo("<font color=\"#FF0000\"><b>&nbsp*Maximum componenten bereikt!</b></font>");
							}
						}
					}
					//het geselecteerde component kan niet verplaatst worden, omdat dit een schaduw component is
					else {
						$query = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '".$selectie."'";
					  $resultaat = mysql_query($query);
				  	$data = mysql_fetch_array($resultaat);
						
						echo($data['Comp_Naam'] . "<input type=\"hidden\" name=\"sComp_Parent\" id=\"sComp_Parent\" value=\"".$selectie."\">");
					}
					echo("</form>");
				}
			}
		?>
	</body>
</html>
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
			  $query = "SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Type_ID in (SELECT Type_Parent FROM comp_type WHERE Comp_Type = '".$_GET['c']."')";

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

					//als de gekozen parent een schaduw component is, dan geld onderstaande niet meer
					$query = "SELECT Schaduw_Vlag FROM comp_lijst WHERE Comp_Lijst_ID = '".$selectie."'";
  			  $resultaat = mysql_query($query);
			  	$data = mysql_fetch_array($resultaat);
					
					//de meldingen voor het aantal aan te maken componenten maken 
					if ($data['Schaduw_Vlag'] == 0) {
						//het aantal aangemaakte componenten per geselecteerde type, gedeelte
						if ($selectie != -1) {
							//maximum aantal ophalen 
							$query = "SELECT Max_Aantal FROM comp_type WHERE Comp_Type = '".$_GET['c']."'";
		  			  $resultaat = mysql_query($query);
					  	$data = mysql_fetch_array($resultaat);
		  			  $maximum = $data['Max_Aantal'];
		
							//aantal aangemaakte componten van dit type ophalen
							$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Parent = '".$selectie."' AND Comp_Type_ID = '".$_GET['c']."'";
		  			  $resultaat = mysql_query($query);
					  	$data = mysql_fetch_array($resultaat);
		  			  $aantal = $data[0];
		
							echo("&nbspAangemaakt: ". $aantal ." van ". $maximum);
							if ($aantal == $maximum)
								echo("<font color=\"#FF0000\"><b>&nbsp*Maximum componenten bereikt!</b></font>");
						}
					}
					
					//het schaduwcomponentengedeelte
					if(isset($_SESSION['admin_deel']) && 	$_SESSION['admin_deel'] > 0) {

						//als er al 1 schaduw component van dit type bestaat, dan deze niet weergeven
						$query = "SELECT Comp_Lijst_ID FROM comp_lijst WHERE Comp_Type_ID = '".$_GET['c']."' AND Schaduw_Vlag='1'";
						$num_rows = mysql_num_rows(mysql_query($query));
						//echo($num_rows . " asdad  ");
						
						//er is nog geen schaduwcomponent, dus aanmaken kan
						if($num_rows == 0){

							//Parent Component = 1 -> dan weergeven
							if ($selectie == 1) {
								echo("<br><input id=\"schaduw\" name=\"schaduw\" type=\"checkbox\">Schaduw component");
							}
							else {
								//geselecteerde parent is een schaduw
								$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID = '".$selectie."' AND Schaduw_Vlag = '1'";
								$num_rows = mysql_num_rows(mysql_query($query));
								//parent is een schaduw
								if($num_rows == 1){
									$query = "SELECT * FROM comp_type WHERE Type_Parent = '".$_GET['c']."'";
									$num_rows = mysql_num_rows(mysql_query($query));		
		
									//Geen parents
							  	if ($num_rows == 0)
										echo("<br>Dit is een instantie. Dit wordt geen schaduw component.<input id=\"schaduw\" name=\"schaduw\" type=\"hidden\" value=\"0\">");
									//mappen worden ook schaduw, and zelf parent dan schaduw
				  				else 
										echo("<br>Dit component wordt een schaduwcomponent.<input id=\"schaduw\" name=\"schaduw\" type=\"hidden\" value=\"1\">");
								}
								//parent is geen schaduw, dus dan dit ook geen schaduw laten worden
								else								
									echo("<br>Dit component kan geen schaduw component worden.<input id=\"schaduw\" name=\"schaduw\" type=\"hidden\" value=\"0\">");
							}
						}
						else {
							//er is een schaduwcomponent... 
							//dus onderscheid maken of t schaduwcomponent geselecteerd is.
							$query = "SELECT Comp_Lijst_ID FROM comp_lijst WHERE Comp_Type_ID IN (SELECT Type_Parent FROM comp_type WHERE Comp_Type = '".$_GET['c']."') AND Schaduw_Vlag='1'";
		  			  $resultaat = mysql_query($query);
					  	$data = mysql_fetch_array($resultaat);
							
							if ($data['Comp_Lijst_ID'] == $selectie)
								echo("<br>Dit is het schaduwcomponent voor dit type.<input id=\"schaduw\" name=\"schaduw\" type=\"hidden\" value=\"0\">");
							else echo("<br>Een schaduwcomponent bestaat voor dit type.<input id=\"schaduw\" name=\"schaduw\" type=\"hidden\" value=\"0\">");
						}
						//als parent een schaduw is, en huidige type is een map -> schaduw voor deze wordt 1 
						//als parent een schaduw is, en huidige type is geen map -> schaduw wordt 0
					}

					echo("</form>");
				}
			}
		?>
	</body>
</html>
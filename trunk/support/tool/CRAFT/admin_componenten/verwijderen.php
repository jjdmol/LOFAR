<?php
	if (isset($_SESSION['admin_deel'])) {
	
		$_SESSION['admin_deel'] = 2;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=3';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/comp_functies.php\"></script>");
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree.js\"></script>");
					echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_items.php\"></script>");
					echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_tpl.js\"></script>");
	  		?>
				<script language="JavaScript">
				<!--//
		 			new tree (TREE_ITEMS, TREE_TPL);
	   		//-->
				</script> 
			
			</div>
	    <div id="rechterdeel">
	    	
	    	<h2>Component verwijderen</h2>
	
				<?php
	  			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
						//extra velden verwijzingen verwijderen!!!
						//eerst kijken of er extra velden verwijzingen zijn.
						$query = "SELECT * FROM comp_koppel_extra WHERE Comp_Lijst_ID = '".$_POST['component']."'";
						$num_rows = mysql_num_rows(mysql_query($query));
						$extra_velden = array();
						if($num_rows > 0) {
							//door de extra velden itereren en de ID's van de extra velden opslaan
							$resultaat = mysql_query($query);
					  	while ($data = mysql_fetch_array($resultaat)) {
		  		 	  	array_push($extra_velden, $data['Kolom_ID']);

					  	}
					  	//De koppeling tussen het component en het extra veld verwijderen
					  	$query = "DELETE FROM comp_koppel_extra WHERE Comp_Lijst_ID = '".$_POST['component']."'";
							if (mysql_query($query)) {
								$datatabel = array();
								//door de extra velden itereren om de datatabel verwijzing op te slaan en het extra veld  te verwijderen
								for($i = 0; $i < Count($extra_velden); $i++) {
									$query = "SELECT Data_Kolom_ID FROM extra_velden WHERE Kolom_ID = '".$extra_velden[$i]."'";
									$resultaat = mysql_query($query);
							  	$data = mysql_fetch_array($resultaat);
			  		 	  	array_push($datatabel, $data['Data_Kolom_ID']);
			  		 	  	
			  		 	  	$query = "DELETE FROM extra_velden WHERE Kolom_ID = '".$extra_velden[$i]."'";
									mysql_query($query);
								}
								
								//de entries in de datatabel verwijderen
								for($i = 0; $i < Count($datatabel); $i++) {			  		 	  	
			  		 	  	$query = "DELETE FROM datatabel WHERE Data_Kolom_ID = '".$datatabel[$i]."'";
									mysql_query($query);
								}								
							}
						}

						$query = "DELETE FROM comp_lijst WHERE Comp_Lijst_ID = " . $_POST['component'];
						
						if (mysql_query($query)) echo("Het door u geselecteerde component is uit het systeem verwijderd.<br>");
						else("Er is iets mis gegaan met het verwijderen van het geselecteerde component!! Het component is niet verwijderd!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een component uit de treeview.</a>');
	  			}
	  			else {
	
						if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$query = "SELECT a.Comp_Naam, b.Type_Naam FROM comp_lijst a, comp_type b WHERE Comp_Lijst_ID = '". $_GET['c'] ."' AND a.Comp_Type_ID = b.Comp_Type";
							$resultaat = mysql_query($query);
							$row = mysql_fetch_row($resultaat);
							
							echo('U heeft het type "'. $row[0] .'" geselecteerd.<br>Dit is een component van het type "' .$row[1]. '"');
							
							//kijken of er componenten onder dit component hangen
							$query = "SELECT COUNT(Comp_Type_ID) FROM comp_lijst WHERE Comp_Parent = '". $_GET['c'] ."' GROUP BY Comp_Type_ID";
							$resultaat = mysql_query($query);
							$row = mysql_fetch_row($resultaat);
							//er zijn geen componenten van dit type.
							if ($row[0] == NULL) {
								
								//kijken of er meldingen bij dit component behoren
								$query = "SELECT COUNT(Meld_Lijst_ID) FROM melding_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."' GROUP BY Comp_Lijst_ID";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								//er zijn geen meldingen bij dit component.
								if ($row[0] == NULL) {
								
									//FORMPJE MAKEN!!!!!!!!!!!!!!!!!!!!!
									?>
							    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
							    		<table>
							    			<tr><td><input type="hidden" name="component" value="<?php echo($_GET['c']);?>">Weet u zeker dat u dit component verwijderen wilt?</td></tr>
							    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil dit component verwijderen</td></tr>
							    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
							    		</table>
							    	</form>
									<?php
								}
								else echo("<br><br>Dit component heeft bijbehorende meldingen.<br>Hierdoor kan dit component niet verwijderd worden!");
							}
							else echo("<br><br>Dit component heeft onderliggende componenten, welke naar dit component verwijzen.<br>Hierdoor kan dit component niet verwijderd worden!");
						}
						else echo("Er is geen component geselecteerd om te verwijderen<br>Selecteer hiernaast een component.");
					}
				
				?>
	    	
	    </div>    	
	    	
		<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>
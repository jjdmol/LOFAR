<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 8;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=3';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
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
	    	<h2>Extern contact verwijderen</h2>
	
				<?php
	    			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
							$query = "DELETE FROM contact WHERE Contact_ID = " . $_POST['contact'];
							if (mysql_query($query)) echo("Het door u geselecteerde contact is uit het systeem verwijderd.<br>");
							else("Er is iets mis gegaan met het verwijderen van het contact!! Het contact is niet verwijderd!");
							echo('<a href="'.$_SESSION['huidige_pagina'].'&s=3">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een contact uit de treeview.</a>');
	    			}
	    			else {
			  			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
								$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID = '". $_GET['c'] ."'";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								
								echo('U heeft het type "'. $row[0] .'" geselecteerd:<br>');
								
								//kijken of er verwijzingen naar dit contact in de comp_lijst tabel aanwezig zijn.
								$query2 = "SELECT Count(Contact_Fabricant) FROM comp_lijst WHERE Contact_Fabricant = '".$_GET['c']."'";
								$resultaat2 = mysql_query($query2);
								$row1 = mysql_fetch_row($resultaat2);
								$contact_gevonden = false;
								if ($row1[0] != 0 ) $contact_gevonden = true;
								$query2 = "SELECT Count(Contact_Leverancier) FROM comp_lijst WHERE Contact_Leverancier = '".$_GET['c']."'";
								$resultaat2 = mysql_query($query2);
								$row1 = mysql_fetch_row($resultaat2);
								if ($row1[0] != 0 ) $contact_gevonden = true;
								
								//false wanneer er verwijzingen naar dit contact in de componenten lijst gevonden zijn
								if(!$contact_gevonden) {
									
									//kijken of er verwijzingen naar dit contact in de comp_type tabel aanwezig zijn.
									$query2 = "SELECT Count(Gefabriceerd_Door) FROM comp_type WHERE Gefabriceerd_Door = '".$_GET['c']."'";
									$resultaat2 = mysql_query($query2);
									$row1 = mysql_fetch_row($resultaat2);
									$contact_gevonden = false;
									if ($row1[0] != 0 ) $contact_gevonden = true;
									$query2 = "SELECT Count(Geleverd_Door) FROM comp_type WHERE Geleverd_Door = '".$_GET['c']."'";
									$resultaat2 = mysql_query($query2);
									$row1 = mysql_fetch_row($resultaat2);
									if ($row1[0] != 0 ) $contact_gevonden = true;

									//false wanneer er verwijzingen naar dit contact in de componenten type tabel gevonden zijn
									if(!$contact_gevonden) {
									
										//kijken of dit contact deel uitmaakt van een hierarchie
										$query2 = "SELECT Count(Contact_Parent) FROM contact WHERE Contact_Parent = ". $_GET['c'] ." GROUP BY Contact_Parent";
										$resultaat2 = mysql_query($query2);
										$row1 = mysql_fetch_row($resultaat2);
										//geen childs gevonden, dit is dus het onderste deel van een hierarchie
										//er mag verwijderd worden!!!!!!!!!!!!!!!
										if ($row1[0] == NULL) {
											//FORMPJE MAKEN!!!!!!!!!!!!!!!!!!!!!
											?>
									    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
									    		<table>
									    			<tr><td><input type="hidden" name="contact" value="<?php echo($_GET['c']);?>">Weet u zeker dat u dit contact verwijderen wilt?</td></tr>
									    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil dit contact verwijderen</td></tr>
									    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
									    		</table>
									    	</form>
											<?php
										}
										else echo("Dit contact heeft onderliggende contacten, welke naar dit contact verwijzen.<br>Hierdoor kan dit contact niet verwijderd worden!");
									}
									else echo("Dit contact wordt gebruikt bij een of meer type componenten.<br>Hierdoor kan dit contact niet verwijderd worden!");
								}
								else echo("Dit contact wordt gebruikt bij een of meer componenten.<br>Hierdoor kan dit contact niet verwijderd worden!");
							}
							else echo("Er is geen contact geselecteerd om te verwijderen<br>Selecteer hiernaast een contact.");
						}
					?>
		    </div>
			<?php  
				
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>    	
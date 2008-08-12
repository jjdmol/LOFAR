<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 5;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=3';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/controle_functies.php');
		
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
  	<h2>Extra velden verwijderen</h2>
	
		<?php
 			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
				$query = "SELECT Tabel_Type, Data_Kolom_ID, Veld_Naam FROM extra_velden WHERE Kolom_ID = '". $_POST['extra_veld'] ."'";
				$resultaat = mysql_query($query);
				$row = mysql_fetch_row($resultaat);
				
				//koppeling verwijderen
				if ($row[0] == 1) $query = "DELETE FROM type_comp_koppel_extra WHERE Kolom_ID = '". $_POST['extra_veld'] ."'";				
				else if ($row[0] == 2) $query = "DELETE FROM type_melding_koppel_extra WHERE Kolom_ID = '". $_POST['extra_veld'] ."'";
				
				$errorlevel = 0;
				if (mysql_query($query)) {
					$errorlevel = 1;

					//extra_veld verwijderen
					$query = "DELETE FROM extra_velden WHERE Kolom_ID = '". $_POST['extra_veld'] ."'";
					if (mysql_query($query)) {
						$errorlevel = 2;
						
						//datatabel verwijderen
						$query = "DELETE FROM datatabel WHERE Data_Kolom_ID = '". $row[1] ."'";
						if (mysql_query($query)) 
							$errorlevel = 3;
					}
				}

				//foutcode vertalen en een melding genereren voor de gebruiker
				if ($errorlevel == 3) echo("Het extra veld \"". $row[2] ."\" is succesvol uit het systeem verwijderd!<br>");
				else if ($errorlevel == 0) echo("Het extra veld \"". $row[2] ."\" kon niet uit het systeem worden verwijderd!<br>Er ging iets fout tijdens het verwijderen van de koppeling van het extra veld met de type.<br>");
				else if ($errorlevel == 1) echo("Het extra veld \"". $row[2] ."\" kon niet uit het systeem worden verwijderd!<br>Het dataveld en het extra veld konden niet verwijderd worden!<br>");
				else if ($errorlevel == 2) echo("Het extra veld \"". $row[2] ."\" is uit het systeem verwijderd!<br>Het dataveld kon echter niet verwijderd worden!<br>");
				echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een extra veld uit de treeview.</a>');
 			}
 			else {
				if (isset($_GET['c']) && $_GET['c'] != 0 ) {
					$query = "SELECT Veld_Naam, Tabel_Type FROM extra_velden WHERE Kolom_ID = '". $_GET['c'] ."'";
					$resultaat = mysql_query($query);
					$row = mysql_fetch_row($resultaat);
					
					echo('U heeft het extra veld "'. $row[0] .'" geselecteerd:<br>');
					
					//eerst kijken of er instanties van dit extra veld aangemaakt zijn
					$query = "SELECT COUNT(Kolom_ID) FROM extra_velden WHERE Type_Beschrijving = '".$_GET['c']."'";	
					$resultaat = mysql_query($query);
					$row = mysql_fetch_row($resultaat);
	
					//er zijn geen instanties van dit extra veld aangemaak, er mag dus verwijderd worden
					if ($row[0] == 0) {
					?>
			    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
			    		<table>
			    			<tr><td><input type="hidden" name="extra_veld" value="<?php echo($_GET['c']);?>">Weet u zeker dat u dit extra veld verwijderen wilt?</td></tr>
			    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil dit extra veld verwijderen</td></tr>
			    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
			    		</table>
			    	</form>
					<?php					
					}
					//er zijn instanties van dit extra veld aangemaakt
					else echo("Dit extra veld is in gebruik bij een of meerdere componenten/meldingen.<br>Hierdoor kan dit extra veld niet verwijderd worden!");
				}
				//er is geen extra veld geselecteerd
				else echo("Er is geen extra veld geselecteerd om te verwijderen<br>Selecteer hiernaast een extra veld.");
			}
		?>

  </div>
	
<?php
	  }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>
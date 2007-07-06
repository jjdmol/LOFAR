<?php
	
	$_SESSION['admin_deel'] = 8;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p=8&s=2';
  
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
    	
    	<h2>Locaties bewerken</h2>

			<?php

				function Valideer_Invoer() {
					if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
						return false;
					
					if (isset($_POST['Loc_Naam'])) {
						if ($_POST['Loc_Naam'] == '')
							return false;
					} else return false;
    		
    			return true;
				}
			
    		if (Valideer_Invoer()) {
					$query = "UPDATE comp_locatie SET Loc_Naam = '". $_POST['Loc_Naam'] . "', Loc_Adres1 = '". $_POST['Loc_Adres1'] . "', Loc_Adres2 = '". $_POST['Loc_Adres2'];
					$query = $query . "', Loc_Postcode = '". $_POST['Loc_Postcode'] ."', Loc_Plaats = '". $_POST['Loc_Plaats'] ."', Long_Graden = '". $_POST['Long_Grad'];
					$query = $query . "', Long_Min = '". $_POST['Long_Min'] ."', Long_Sec = '". $_POST['Long_Sec'] ."', Lat_Graden = '". $_POST['Lat_Grad'];
					$query = $query . "', Lat_Min = '". $_POST['Lat_Min'] ."', Lat_Sec = '". $_POST['Lat_Sec'] ."' WHERE Locatie_ID = '" . $_GET['c'] . "'";

					if (mysql_query($query)) echo("De gewijzigde locatie \"". $_POST['Loc_Naam'] ."\" is in het systeem bijgewerkt<br>");
					else("Er is iets mis gegaan met het opslaan van de locatie \"". $_POST['Loc_Naam'] ."\"!! De locatie is niet bijgewerkt!");
					echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar de vorige locatie of selecteer links een locatie uit de treeview.</a>');
    		}
    		else {
    		
	    		if (isset($_GET['c']) && $_GET['c'] != 0 ) {
						$query = "SELECT * FROM comp_locatie WHERE Locatie_ID ='". $_GET['c'] ."'";
						$resultaat = mysql_query($query);
				  	$row = mysql_fetch_array($resultaat);
			
			?>
	    	
	    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
	    		<table>
	    			<tr>
	    				<td>Locatie ID:</td>
	    				<td><?php echo($row['Locatie_ID']); ?></td>
	    			</tr>
	    			<tr>
	    				<td>Locatie naam:</td>
	    				<td><input type="text" name="Loc_Naam" value="<?php if (isset($_POST['Loc_Naam'])) echo($_POST['Loc_Naam']); else echo($row['Loc_Naam']); ?>">
	    					<?php if(isset($_POST['Loc_Naam']) && $_POST['Loc_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?>
	    				</td>
	    			</tr>
	    			<tr>
	    				<td>Locatie adres 1:</td>
	    				<td><input type="text" name="Loc_Adres1" value="<?php if (isset($_POST['Loc_Adres1'])) echo($_POST['Loc_Adres1']); else echo($row['Loc_Adres1']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Locatie adres 2:</td>
	    				<td><input type="text" name="Loc_Adres2" value="<?php if (isset($_POST['Loc_Adres2'])) echo($_POST['Loc_Adres2']); else echo($row['Loc_Adres2']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Locatie postcode:</td>
	    				<td><input type="text" name="Loc_Postcode" value="<?php if (isset($_POST['Loc_Postcode'])) echo($_POST['Loc_Postcode']); else echo($row['Loc_Postcode']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Locatie plaats:</td>
	    				<td><input type="text" name="Loc_Plaats" value="<?php if (isset($_POST['Loc_Plaats'])) echo($_POST['Loc_Plaats']); else echo($row['Loc_Plaats']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Graden (longitude):</td>
	    				<td><input type="text" name="Long_Grad" value="<?php if (isset($_POST['Long_Grad'])) echo($_POST['Long_Grad']); else echo($row['Long_Graden']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Minuten (longitude):</td>
	    				<td><input type="text" name="Long_Min" value="<?php if (isset($_POST['Long_Min'])) echo($_POST['Long_Min']); else echo($row['Long_Min']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Secondes (longitude):</td>
	    				<td><input type="text" name="Long_Sec" value="<?php if (isset($_POST['Long_Sec'])) echo($_POST['Long_Sec']); else echo($row['Long_Sec']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Graden (latitude):</td>
	    				<td><input type="text" name="Lat_Grad" value="<?php if (isset($_POST['Lat_Grad'])) echo($_POST['Lat_Grad']); else echo($row['Lat_Graden']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Minuten (latitude):</td>
	    				<td><input type="text" name="Lat_Min" value="<?php if (isset($_POST['Lat_Min'])) echo($_POST['Lat_Min']); else echo($row['Lat_Min']); ?>"></td>
	    			</tr>
	    			<tr>
	    				<td>Secondes (latitude):</td>
	    				<td><input type="text" name="Lat_Sec" value="<?php if (isset($_POST['Lat_Sec'])) echo($_POST['Lat_Sec']); else echo($row['Lat_Sec']); ?>"></td>
	    			</tr>
	    			<tr>
		    		  <td><input name="opslaan" type="hidden" value="1"></td>
		    		  <td><a href="javascript:document.theForm.submit();">Opslaan</a></td>
	    			</tr>
	    		</table>
    		</form>
				<?php
	    		}
					else echo('Er is geen locatie geselecteerd om te wijzigen.<br>Selecteer hiernaast een locatie.'); 
				}
    	?>
    </div>

<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>    	
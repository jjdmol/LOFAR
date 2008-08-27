<?php
	if(isset($_SESSION['admin_deel'])) {
		$_SESSION['admin_deel'] = 9;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
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
						$query = "UPDATE comp_locatie SET Loc_Naam = '". htmlentities($_POST['Loc_Naam'], ENT_QUOTES) . "', Loc_Adres1 = '". htmlentities($_POST['Loc_Adres1'], ENT_QUOTES) . "', Loc_Adres2 = '". htmlentities($_POST['Loc_Adres2'], ENT_QUOTES);
						$query = $query . "', Loc_Postcode = '". htmlentities($_POST['Loc_Postcode'], ENT_QUOTES) ."', Loc_Plaats = '". htmlentities($_POST['Loc_Plaats'], ENT_QUOTES) ."', Long_Graden = '". htmlentities($_POST['Long_Grad'], ENT_QUOTES);
						$query = $query . "', Long_Min = '". htmlentities($_POST['Long_Min'], ENT_QUOTES) ."', Long_Sec = '". htmlentities($_POST['Long_Sec'], ENT_QUOTES) ."', Lat_Graden = '". htmlentities($_POST['Lat_Grad'], ENT_QUOTES);
						$query = $query . "', Lat_Min = '". htmlentities($_POST['Lat_Min'], ENT_QUOTES) ."', Lat_Sec = '". htmlentities($_POST['Lat_Sec'], ENT_QUOTES) ."' WHERE Locatie_ID = '" . $_GET['c'] . "'";
	
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
		    				<td><input type="text" name="Loc_Naam" value="<?php if (isset($_POST['Loc_Naam'])) echo(htmlentities($_POST['Loc_Naam'], ENT_QUOTES)); else echo($row['Loc_Naam']); ?>">
		    					<?php if(isset($_POST['Loc_Naam']) && $_POST['Loc_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?>
		    				</td>
		    			</tr>
		    			<tr>
		    				<td>Locatie adres 1:</td>
		    				<td><input type="text" name="Loc_Adres1" value="<?php if (isset($_POST['Loc_Adres1'])) echo(htmlentities($_POST['Loc_Adres1'], ENT_QUOTES)); else echo($row['Loc_Adres1']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Locatie adres 2:</td>
		    				<td><input type="text" name="Loc_Adres2" value="<?php if (isset($_POST['Loc_Adres2'])) echo(htmlentities($_POST['Loc_Adres2'], ENT_QUOTES)); else echo($row['Loc_Adres2']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Locatie postcode:</td>
		    				<td><input type="text" name="Loc_Postcode" value="<?php if (isset($_POST['Loc_Postcode'])) echo(htmlentities($_POST['Loc_Postcode'], ENT_QUOTES)); else echo($row['Loc_Postcode']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Locatie plaats:</td>
		    				<td><input type="text" name="Loc_Plaats" value="<?php if (isset($_POST['Loc_Plaats'])) echo(htmlentities($_POST['Loc_Plaats'], ENT_QUOTES)); else echo($row['Loc_Plaats']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Graden (longitude):</td>
		    				<td><input type="text" name="Long_Grad" value="<?php if (isset($_POST['Long_Grad'])) echo(htmlentities($_POST['Long_Grad'], ENT_QUOTES)); else echo($row['Long_Graden']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Minuten (longitude):</td>
		    				<td><input type="text" name="Long_Min" value="<?php if (isset($_POST['Long_Min'])) echo(htmlentities($_POST['Long_Min'], ENT_QUOTES)); else echo($row['Long_Min']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Secondes (longitude):</td>
		    				<td><input type="text" name="Long_Sec" value="<?php if (isset($_POST['Long_Sec'])) echo(htmlentities($_POST['Long_Sec'], ENT_QUOTES)); else echo($row['Long_Sec']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Graden (latitude):</td>
		    				<td><input type="text" name="Lat_Grad" value="<?php if (isset($_POST['Lat_Grad'])) echo(htmlentities($_POST['Lat_Grad'], ENT_QUOTES)); else echo($row['Lat_Graden']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Minuten (latitude):</td>
		    				<td><input type="text" name="Lat_Min" value="<?php if (isset($_POST['Lat_Min'])) echo(htmlentities($_POST['Lat_Min'], ENT_QUOTES)); else echo($row['Lat_Min']); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Secondes (latitude):</td>
		    				<td><input type="text" name="Lat_Sec" value="<?php if (isset($_POST['Lat_Sec'])) echo(htmlentities($_POST['Lat_Sec'], ENT_QUOTES)); else echo($row['Lat_Sec']); ?>"></td>
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
	}
?>    	
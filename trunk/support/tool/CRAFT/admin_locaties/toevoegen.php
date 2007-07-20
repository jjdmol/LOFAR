<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 9;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
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
	    	
	    	<h2>Locaties toevoegen</h2>
	    	
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
		    		
		    		if(Valideer_Invoer()) {
							$query = "INSERT INTO comp_locatie (Loc_Naam, Long_Graden, Long_Min, Long_Sec, Lat_Graden, Lat_Min, Lat_Sec, ";
							$query = $query . "Loc_Adres1, Loc_Adres2, Loc_Postcode, Loc_Plaats)  VALUES ('". htmlentities($_POST['Loc_Naam'], ENT_QUOTES);
							$query = $query . "', '". htmlentities($_POST['Long_Grad'], ENT_QUOTES) ."', '". htmlentities($_POST['Long_Min'], ENT_QUOTES). "', '".htmlentities($_POST['Long_Sec'], ENT_QUOTES);
							$query = $query . "', '". htmlentities($_POST['Lat_Grad'], ENT_QUOTES) . "', '" .htmlentities($_POST['Lat_Min'], ENT_QUOTES). "', '" .htmlentities($_POST['Lat_Sec'], ENT_QUOTES);
							$query = $query . "', '". htmlentities($_POST['Loc_Adres1'], ENT_QUOTES) . "', '". htmlentities($_POST['Loc_Adres2'], ENT_QUOTES) . "', '". htmlentities($_POST['Loc_Postcode'], ENT_QUOTES). "', '". htmlentities($_POST['Loc_Plaats'], ENT_QUOTES). "')";
							if (mysql_query($query)) echo("De nieuwe locatie \"". $_POST['Loc_Naam'] ."\" is aan het systeem toegevoegd<br>");
							else echo("De nieuwe locatie \"". $_POST['Loc_Naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
							echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een locatie toe te voegen.</a>');
		    		}
		    		else {
		    		
		    	?>
		    	
		    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
		    		<table>
		    			<tr>
		    				<td>Locatie naam:</td>
		    				<td><input type="text" name="Loc_Naam" value="<?php if(isset($_POST['Loc_Naam'])) echo(htmlentities($_POST['Loc_Naam'], ENT_QUOTES)); ?>">
		    					<?php if(isset($_POST['Loc_Naam']) && $_POST['Loc_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?>
		    				</td>
		    			</tr>
		    			<tr>
		    				<td>Locatie adres 1:</td>
		    				<td><input type="text" name="Loc_Adres1" value="<?php if(isset($_POST['Loc_Adres1'])) echo(htmlentities($_POST['Loc_Adres2'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Locatie adres 2:</td>
		    				<td><input type="text" name="Loc_Adres2" value="<?php if(isset($_POST['Loc_Adres2'])) echo(htmlentities($_POST['Loc_Adres2'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Locatie postcode:</td>
		    				<td><input type="text" name="Loc_Postcode" value="<?php if(isset($_POST['Loc_Postcode'])) echo(htmlentities($_POST['Loc_Postcode'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Locatie plaats:</td>
		    				<td><input type="text" name="Loc_Plaats" value="<?php if(isset($_POST['Loc_Plaats'])) echo(htmlentities($_POST['Loc_Plaats'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Graden (longitude):</td>
		    				<td><input type="text" name="Long_Grad" value="<?php if(isset($_POST['Long_Grad'])) echo(htmlentities($_POST['Long_Grad'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Minuten (longitude):</td>
		    				<td><input type="text" name="Long_Min" value="<?php if(isset($_POST['Long_Min'])) echo(htmlentities($_POST['Long_Min'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Secondes (longitude):</td>
		    				<td><input type="text" name="Long_Sec" value="<?php if(isset($_POST['Long_Sec'])) echo(htmlentities($_POST['Long_Sec'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Graden (latitude):</td>
		    				<td><input type="text" name="Lat_Grad" value="<?php if(isset($_POST['Lat_Grad'])) echo(htmlentities($_POST['Lat_Grad'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Minuten (latitude):</td>
		    				<td><input type="text" name="Lat_Min" value="<?php if(isset($_POST['Lat_Min'])) echo(htmlentities($_POST['Lat_Min'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
		    				<td>Secondes (latitude):</td>
		    				<td><input type="text" name="Lat_Sec" value="<?php if(isset($_POST['Lat_Sec'])) echo(htmlentities($_POST['Lat_Sec'], ENT_QUOTES)); ?>"></td>
		    			</tr>
		    			<tr>
			    		  <td><input name="opslaan" type="hidden" value="1"></td>
			    		  <td><a href="javascript:document.theForm.submit();">Opslaan</a></td>
		    			</tr>
		    		</table>
	    		</form>
			<?php
				}
			?>
	    	
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>    	
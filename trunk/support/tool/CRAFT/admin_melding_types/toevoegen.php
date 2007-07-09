  <?php
	
	$_SESSION['admin_deel'] = 3;
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
    	<h2>Type melding toevoegen</h2>
    	
    	<?php
    	
    		function Valideer_Invoer() {
					if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
						return false;

					if (isset($_POST['type_naam'])) {
						if ($_POST['type_naam'] == '')
							return false;
					} else return false;

					if (isset($_POST['type_beschrijving'])) {
						if($_POST['type_beschrijving'] == '')
							return false;
					} else return false;

   				return true;
    		}
    	
    		//kijken of er opgeslagen mag worden
    		if (Valideer_Invoer()) {
					
					
					if (isset($_POST['type_algemeen']) && ($_POST['type_algemeen'] == 1 || $_POST['type_algemeen'] == 'on')) $algemeen = 1;
					else $algemeen = 0;
					
					$query = "INSERT INTO melding_type (Melding_Type_Naam, Huidige_Status, Algemene_Melding, Stand_Beschrijving";
					if (isset($_POST['type_oplossing']) && $_POST['type_oplossing'] != '')
						$query = $query . ", Stand_Oplossing";
					
					$query = $query . ") VALUES ('". $_POST['type_naam'] ."', '". $_POST['type_status'] ."', '". $algemeen ."', '". $_POST['type_beschrijving'];
					if (isset($_POST['type_oplossing']) && $_POST['type_oplossing'] != '')
						$query = $query. "', '" . $_POST['type_oplossing'];
					$query = $query . "')";

					if (mysql_query($query)) echo("Het nieuwe melding type \"". $_POST['type_naam'] ."\" is aan het systeem toegevoegd<br>");
					else echo("Het nieuwe type melding \"". $_POST['type_naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
					echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een meldingtype toe te voegen.</a>');
   			
    		}
    		else {
    	?>
    	
	    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>">
		    	<table>
		    		<tr>
		    			<td>Naam type:</td>
		    			<td><input name="type_naam" type="text" value="<?php if(isset($_POST['type_naam'])) echo($_POST['type_naam']); ?>">
		    				  <?php if(isset($_POST['type_naam']) && $_POST['type_naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?></td>
		    		</tr>
		    		<tr>
		    			<td>Status melding:</td>
		    			<td><select name="type_status"><option value="1" SELECTED>1</option></select></td>
		    		</tr>
		    		<tr>
		    			<td>Algemene melding:</td>
		    			<td><input name="type_algemeen" type="checkbox"></td>
		    		</tr>
		    		<tr>
		    			<td>Omschrijving:</td>
		    			<td><textarea name="type_beschrijving" rows="5" cols="35"><?php if(isset($_POST['type_beschrijving'])) echo($_POST['type_beschrijving']); ?></textarea>
		    				  <?php if(isset($_POST['type_beschrijving']) && $_POST['type_beschrijving'] == '') echo('<b>* Er is geen omschrijving ingevoerd!</b>'); ?></td>

		    		</tr>
		    		<tr>
		    			<td>Eventuele oplossing:</td>
		    			<td><textarea name="type_oplossing" rows="5" cols="35"><?php if(isset($_POST['type_oplossing'])) echo($_POST['type_oplossing']); ?></textarea></td>
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
?>    	
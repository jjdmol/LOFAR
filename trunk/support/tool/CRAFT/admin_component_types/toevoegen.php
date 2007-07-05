  	<?php
	
	$_SESSION['admin_deel'] = 1;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p=1&s=1';
  
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
    	
    	<h2>Type component toevoegen</h2>
    	<?php
    	
    		function Valideer_Invoer() {
					if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
						return false;
					
					if (isset($_POST['naam'])) {
						if ($_POST['naam'] == '')
							return false;
					} else return false;
					
					//Minimum niet negatief
					if (isset($_POST['minimum']) && $_POST['minimum'] < 0)
						return false;
					
					//maximum niet groter dan minimum
					if (isset($_POST['minimum']) && isset($_POST['maximum']) && $_POST['maximum'] < $_POST['minimum']) 
						return false;

					//Reserve tussen minimum en maximum										
					if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && 
						($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
							return false;
					
					return true;    			
    		}
    	
				//eerst de invoer valideren om te kijken of er opgeslagen mag worden...
				if (Valideer_Invoer())
				{
					if (isset($_POST['entry']) && ($_POST['entry'] == 1 || $_POST['entry'] == 'on')) $entry = 1;
					else $entry = 0;
					
					$query = "INSERT INTO comp_type (Type_Naam, Type_Parent, Aangemaakt_Door, Aanmaak_Datum, Structuur_Entry, Min_Aantal, Max_Aantal, Reserve_Minimum, Type_Verantwoordelijke) ";
					$query = $query . " VALUES ('". $_POST['naam'] ."', '". $_POST['parent'] ."', '". $_SESSION['gebr_id'] ."' ,NOW(), '". $entry ."', '". $_POST['minimum'] ."', '";
					$query = $query. $_POST['maximum'] ."', '". $_POST['reserve'] ."', '". $_POST['verantwoordelijke'] ."')";
					if (mysql_query($query)) echo("Het nieuwe type \"". $_POST['naam'] ."\" is aan het systeem toegevoegd<br>");
					else echo("Het nieuwe type \"". $_POST['naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
					echo('<a href="admin.php?s=1&p=1">Klik hier om nog een type toe te voegen.</a>');
				}
				else {
					
    	?>
    	
    	<form name="theForm" method="post" action="admin.php?p=1&s=1">
	    	<table>
	    		<tr><td>Naam van het type:</td><td><input type="text" name="naam" value="<?php if(isset($_POST['naam'])) echo($_POST['naam']); ?>"><?php if(isset($_POST['naam']) && $_POST['naam'] == '') echo('<b id="type_naam">* Er is geen naam ingevoerd!</b>'); ?></td></tr>
	    		<tr><td>Parent van het type:</td><td>
	    			<select name="parent">
	    			<?php 
	    				$query = 'SELECT Comp_Type, Type_Naam FROM comp_type';
	    			  $resultaat = mysql_query($query);
				  		if (isset($_POST['parent'])) $selectie = $_POST['parent'];
				  		else if(isset($_GET['c'])) $selectie = $_GET['c'];
							
					  	while ($data = mysql_fetch_array($resultaat)) {
					  		echo('<option value="'. $data['Comp_Type'] .'"');
					  		if(isset($selectie) && $data['Comp_Type'] == $selectie)
					  			echo('SELECTED');
					  		echo('>'. $data['Type_Naam'] .'</option>');
					  	}
	    			?>
	    			</select></td></tr>
	    		<tr><td>Structurele entry:</td><td><input name="entry" type="checkbox" value="1" <?php if(isset($_POST['entry']) && $_POST['entry'] == 1 ) echo("CHECKED"); ?>></td></tr>
	    		<tr><td>Gefabriceerd door:</td><td><select name="fabricant"></select></td></tr>
	    		<tr><td>Geleverd door:</td><td><select name="leverancier"></select></td></tr>
	    		<tr><td>Minimaal aan te maken aantal:</td><td><input name="minimum" type="text" value="<?php if(isset($_POST['minimum'])) echo($_POST['minimum']); else echo('1'); ?>"><?php if (isset($_POST['minimum']) && $_POST['minimum'] < 0 ) echo('<b id="type_minimum">* Het minimum aantal ('. $_POST['minimum'] .') mag niet negatief zijn!</b>');?></td></tr>
	    		<tr><td>Maximaal aan te maken aantal:</td><td><input name="maximum" type="text" value="<?php if(isset($_POST['maximum'])) echo($_POST['maximum']); ?>"><?php if (isset($_POST['minimum']) && isset($_POST['maximum']) && ($_POST['maximum'] < $_POST['minimum']) ) echo('<b id="type_maximum">* Het minimum aantal is hoger dan het maximum aantal!</b>'); ?> </td></tr>
	    		<tr><td>Aantal op reserve:</td><td><input name="reserve" type="text" value="<?php if(isset($_POST['reserve'])) echo($_POST['reserve']) ?>"><?php
						if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && 
							($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
	    					echo('<b id="type_reserve">* De invoer valt buiten de min. / max. waardes!</b>');
    				?></td></tr>
	    		<tr><td>Type verantwoordelijke:</td><td>
	    			<select name="verantwoordelijke">
	  				<?php
	    				$query = 'SELECT Werknem_ID, inlognaam FROM gebruiker';
	    			  $resultaat = mysql_query($query);
					  	if (isset($_POST['verantwoordelijke'])) $selectie = $_POST['verantwoordelijke'];
					  	else $selectie = $_SESSION['gebr_id'];
					  	
					  	while ($data = mysql_fetch_array($resultaat)) {
					  		echo('<option value="'. $data['Werknem_ID'] .'"');
					  		if (isset($selectie) && $selectie == $data['Werknem_ID']) echo(' SELECTED ');
					  		echo('>'.$data['inlognaam'] .'</option>');
					  	}
	   				?>
	    			</select></td></tr>
	    		<tr><td></td><td><input name="opslaan" type="hidden" value="1"><a href="javascript:document.theForm.submit();">Opslaan</a></td></tr>
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
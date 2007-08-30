<?php
	if (isset($_SESSION['admin_deel'])){
	
		$_SESSION['admin_deel'] = 1;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."admin_component_types/comp_type_functies.php\"></script>");

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
	    		$Geselecteerd_Type = '';


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
						if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && ($_POST['reserve'] != 0) && 
							($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
								return false;
						
						return true;    			
	    		}
	    	
					//eerst de invoer valideren om te kijken of er opgeslagen mag worden...
					if (Valideer_Invoer())
					{
						if (isset($_POST['entry']) && ($_POST['entry'] == 1 || $_POST['entry'] == 'on')) $entry = 1;
						else $entry = 0;
						
						$query = "INSERT INTO comp_type (Type_Naam, Type_Parent, Aangemaakt_Door, Aanmaak_Datum, Structuur_Entry, Min_Aantal, Max_Aantal, Reserve_Minimum, Type_Verantwoordelijke, Gefabriceerd_door, Geleverd_Door) ";
						$query = $query . " VALUES ('". htmlspecialchars($_POST['naam']) ."', '". $_POST['parent'] ."', '". $_SESSION['gebr_id'] ."' ,NOW(), '". $entry ."', '". htmlspecialchars($_POST['minimum']) ."', '";
						$query = $query. htmlspecialchars($_POST['maximum']) ."', '". htmlspecialchars($_POST['reserve']) ."', '". $_POST['hidden_verantwoordelijke'] ."', '".$_POST['fabricant']."', '". $_POST['leverancier'] ."')";
						if (mysql_query($query)) echo("Het nieuwe type \"". $_POST['naam'] ."\" is aan het systeem toegevoegd<br>");
						else echo("Het nieuwe type \"". $_POST['naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
						echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een type toe te voegen.</a>');
					}
					else {
	    	?>
	    	
	    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
		    	<table>
		    		<tr><td>Naam van het type:</td><td><input type="text" name="naam" value="<?php if(isset($_POST['naam'])) echo(htmlentities($_POST['naam'],ENT_QUOTES)); ?>"><?php if(isset($_POST['naam']) && $_POST['naam'] == '') echo('<b id="type_naam">* Er is geen naam ingevoerd!</b>'); ?></td></tr>
		    		<tr><td>Parent van het type:</td><td>
		    			<select name="parent" id="parent" onchange="switchDocument(<?php if(isset($_POST['hidden_verantwoordelijke'])) echo($_POST['hidden_verantwoordelijke']); else echo("-1");?>);">
		    			<?php 
		    				//Type ophalen uit gebruikersgroeprechten
		    				$query = "SELECT Comp_Type_ID FROM gebruikersgroeprechten WHERE Groep_ID = '".$_SESSION['groep_id']."'";
		    			  $resultaat = mysql_query($query);
								$data = mysql_fetch_array($resultaat);

					  		if (isset($_POST['parent'])) $Geselecteerd_Type = $_POST['parent'];
					  		else if(isset($_GET['c'])) $Geselecteerd_Type = $_GET['c'];
					  		else $Geselecteerd_Type = 'SELECTED';

								Vul_Component_Types_Select_Box($data[0], $Geselecteerd_Type, true);
		    			?></select>
		    		</td></tr>
		    		<tr><td>Structurele entry:</td><td><input name="entry" type="checkbox" value="1" <?php if(isset($_POST['entry']) && $_POST['entry'] == 1 ) echo("CHECKED"); ?>></td></tr>
		    		<tr><td>Gefabriceerd door:</td><td><select name="fabricant">
			    		<?php
			    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
		    			  $resultaat = mysql_query($query);
								
						  	while ($data = mysql_fetch_array($resultaat)) {
						  		echo('<option value="'. $data['Contact_ID'] .'"');
						  		if(isset($_POST['fabricant']) && $data['Contact_ID'] == $_POST['fabricant'])
						  			echo('SELECTED');
						  		echo('>'. $data['Contact_Naam'] .'</option>');
						  	}
				    	?>
		    			</select></td></tr>
		    		<tr><td>Geleverd door:</td><td><select name="leverancier">
			    		<?php
			    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
		    			  $resultaat = mysql_query($query);
								
						  	while ($data = mysql_fetch_array($resultaat)) {
						  		echo('<option value="'. $data['Contact_ID'] .'"');
						  		if(isset($_POST['leverancier']) && $data['Contact_ID'] == $_POST['leverancier'])
						  			echo('SELECTED');
						  		echo('>'. $data['Contact_Naam'] .'</option>');
						  	}
				    	?>
		    			</select></td></tr>
		    		<tr><td>Minimaal aan te maken aantal:</td><td><input name="minimum" type="text" value="<?php if(isset($_POST['minimum'])) echo(htmlentities($_POST['minimum'], ENT_QUOTES)); else echo('1'); ?>"><?php if (isset($_POST['minimum']) && $_POST['minimum'] < 0 ) echo('<b id="type_minimum">* Het minimum aantal ('. $_POST['minimum'] .') mag niet negatief zijn!</b>');?></td></tr>
		    		<tr><td>Maximaal aan te maken aantal:</td><td><input name="maximum" type="text" value="<?php if(isset($_POST['maximum'])) echo(htmlentities($_POST['maximum'], ENT_QUOTES)); ?>"><?php if (isset($_POST['minimum']) && isset($_POST['maximum']) && ($_POST['maximum'] < $_POST['minimum']) ) echo('<b id="type_maximum">* Het minimum aantal is hoger dan het maximum aantal!</b>'); ?> </td></tr>
		    		<tr><td>Aantal op reserve:</td><td><input name="reserve" type="text" value="<?php if(isset($_POST['reserve'])) echo(htmlentities($_POST['reserve'],ENT_QUOTES)) ?>"><?php
							if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && ($_POST['reserve'] != 0) &&
								($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
		    					echo('<b id="type_reserve">* De invoer valt buiten de min. / max. waardes!</b>');
	    				?></td></tr>
    				<?php
    					if (isset($_POST['hidden_verantwoordelijke'])) 
    						$verantwoordelijke = $_POST['hidden_verantwoordelijke'];
    					else $verantwoordelijke = -1;
    					
    					if (isset($_GET['c']))
    						$selectie = $_GET['c'];
    					else $selectie = $Geselecteerd_Type;
    				?>
    				<tr><td>Verantwoordelijke:</td><td><iframe id="frame_contact" name="frame_contact" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/type_verantwoordelijke.php?c=<?php echo($selectie . "&s=" . $verantwoordelijke);?>" width="300" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td></tr>
		    		<tr><td><input name="hidden_verantwoordelijke" id="hidden_verantwoordelijke" type="hidden" value="-1"></td><td><input name="opslaan" type="hidden" value="1"><a href="javascript:submitTypeOpslaan();">Opslaan</a></td></tr>
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
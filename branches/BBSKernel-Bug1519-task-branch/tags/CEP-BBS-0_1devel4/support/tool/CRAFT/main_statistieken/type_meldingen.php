<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=3';
	  $_SESSION['type_overzicht'] = 3;
	  
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

	    	<h2>Type meldingen</h2>
				
				<?php
					if (isset($_GET['c'])) {
						$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID = '".$_GET['c']."'";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);
						echo("U heeft \"". $data[0]  ."\" geselecteerd.");

						$query = "SELECT Count(Meld_Type_ID) FROM melding_lijst";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);

						$query = "SELECT Count(Meld_Type_ID), Count(Distinct(Comp_Lijst_ID)) FROM melding_lijst WHERE Meld_Type_ID = '".$_GET['c']."'";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						echo("<table>");
						echo("<tr><td>Aantal meldingen van dit type:</td><td>". $row[0] . "</td><td>(Totale meldingen: ". $data[0] .")</td></tr>");
						
						$query = "SELECT Count(Distinct(Comp_Type_ID)) FROM comp_lijst WHERE Comp_Lijst_ID IN (SELECT Distinct(Comp_Lijst_ID) FROM melding_lijst WHERE Meld_Type_ID = '".$_GET['c']."')";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);
						echo("<tr><td>Aantal componenten met dit type melding:</td><td>". $row[1] ."</td><td>(Verspreid over ". $data[0] ." componenttypes)</td></tr>");

						$query = "SELECT Count(Comp_lijst_ID) as aantal, Comp_Lijst_ID from melding_lijst WHERE Meld_Type_ID = '".$_GET['c']."' AND Comp_Lijst_ID > 1 GROUP BY Comp_Lijst_ID ORDER BY aantal desc";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);
						$query = "select Type_Naam from comp_type WHERE Comp_Type in (SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '".$data[1]."')";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						
						echo("<tr><td>Type component met de meeste meldingen: </td><td>" . $row[0] ."</td><td>(Aantal meldingen bij dit type: ". $data[0] .")</td></tr>");
						echo("</table>");

					}
					else { echo("Er is geen type melding geselecteerd!<br>Selecteer een type melding!");}
				?>

	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 

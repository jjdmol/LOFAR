<?php

	session_start();
	if (isset($_SESSION['laatste_inlog'])) {
		include_once("../includes/vars.php");
	
		echo("<html><head></head><body bgcolor=\"#B3CCE6\">");
	
		$query = "SELECT * FROM comp_type WHERE Reserve_Minimum !=0";
		$resultaat = mysql_query($query);
		$aantal = 0;
		echo("<table border=\"1\">");
		while ($data = mysql_fetch_array($resultaat)) {
			//het aantal componenten tellen
			$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Type_ID = '".$data['Comp_Type']."'";
			$rest = mysql_query($query);
			$row = mysql_fetch_array($rest);
			$aantal = $row[0];
			
			//het aantal componenten met status 4 tellen en dit aantal vergelijken met de drempelwaardes.
			$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Huidige_Status = 4 AND Meld_Lijst_ID in (SELECT Laatste_Melding FROM comp_lijst WHERE Comp_Type_ID = '".$data['Comp_Type']."')";
			$rest = mysql_query($query);
			$row = mysql_fetch_array($rest);
			//20 % op reserve
			
			$drempel = (round($data['Reserve_Minimum'] * 1.2));
			if ($row[0] == 0) {
				echo("<tr><td>".$data['Comp_Type']."</td><td>".substr($data['Type_Naam'], 0, 30)."</td><td><b>Geen componenten op voorraad!</b>&nbsp");
				echo("</td><td>aantal: ".$aantal."&nbsp</td><td>reserve:".$data['Reserve_Minimum']." (".$drempel.")</td><td><a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/comp_type.php?c=".$data['Comp_Type']."  \" target=\"_blank\">Info</a></td></tr>");
				$aantal++;
			}
			else if ($aantal > $data['Reserve_Minimum']) {
				echo("<tr><td>".$data['Comp_Type']."</td><td>".substr($data['Type_Naam'], 0, 30)."</td><td><b>Te weinig componenten op voorraad!</b>&nbsp");
				echo("</td><td>aantal: ".$aantal."&nbsp</td><td>reserve:".$data['Reserve_Minimum']." (".$drempel.")</td><td><a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/comp_type.php?c=".$data['Comp_Type']."  \" target=\"_blank\">Info</a></td></tr>");
				$aantal++;
			}
			else if ($aantal <= $drempel && $row[0] >= $data['Reserve_Minimum']) {
				echo("<tr><td>".$data['Comp_Type']."</td><td>".substr($data['Type_Naam'], 0, 30)."</td><td>De reserves raken op!&nbsp");
				echo("</td><td>aantal: ".$aantal."&nbsp</td><td>reserve:".$data['Reserve_Minimum']." (".$drempel.")</td><td><a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/comp_type.php?c=".$data['Comp_Type']."  \" target=\"_blank\">Info</a></td></tr>");
				$aantal++;
			}
		}
		echo("</table>");
		if($aantal == 0 )
			echo("Er zijn voldoende componenten op reserve");
	}
	echo("</body></html>");

?>
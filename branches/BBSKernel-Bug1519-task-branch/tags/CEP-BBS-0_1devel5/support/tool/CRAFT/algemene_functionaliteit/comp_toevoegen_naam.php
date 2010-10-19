<html>
	<head>
		
	</head>
	<body bgcolor="#B3CCE6">
		<?php
		
			include_once("../includes/vars.php");

			if (isset($_GET['c']) && $_GET['c'] != 0) {
				$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type = '".$_GET['c']."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
				
				$naam = $data[0];
				if(isset($_GET['n']))
					$naam = $_GET['n'];
				$inputveld = "<input id=\"sComp_Naam\" name=\"sComp_Naam\" type=\"text\" value=\"". htmlentities($naam, ENT_QUOTES);

				$query = "SELECT Count(Comp_Type_ID) FROM comp_lijst WHERE Comp_Type_ID = '".$_GET['c']."' GROUP BY Comp_Type_ID";
				$resultaat = mysql_query($query);
				if ($resultaat != null) {
					$data = mysql_fetch_array($resultaat);
					if (isset($data[0]))
						$aantal = $data[0];
					else 
						$aantal = 0;
				}	
				if (!isset($_GET['n']))
						$inputveld = $inputveld . " nr. ".($aantal + 1);
				$inputveld = $inputveld . "\">";

				echo($inputveld);
			}
		?>
	</body>
</html>
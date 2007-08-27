<?php

	session_start();
	if (isset($_GET['c']) || isset($_GET['a'])) {
?>
		<html>
			<head></head>
			<body>
				<h2>Uploaden van bestanden:</h2>

				<?php
					if (isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
		
						$tijd = time();
						$pos = strrpos($_FILES['file']['name'], "." );
						$extensie = substr($_FILES['file']['name'], $pos);

						if (move_uploaded_file($_FILES['file']['tmp_name'], "data/" . $tijd . $extensie)) {
							if (isset($_GET['a'])) {
								$_SESSION['abestand'.$_GET['a']] = ("data/" . time().$extensie);
							}
							
							if (isset($_GET['c'])) {
								$_SESSION['bestand'.$_GET['c']] = ("data/" . time().$extensie);
							}
			       	print "Het bestand is succesvol geupload!<br>";
			       	print "U kunt nu dit scherm afsluiten.";

						}
						else {
							if (isset($_GET['a'])) {
								$_SESSION['abestand'.$_GET['a']] = '-1';
							}

							if (isset($_GET['c'])) {
								$_SESSION['bestand'.$_GET['c']] = '-1';
							}
			       	print "Er is iets fout gegaan met het uploaden!<br>";
			       	print "Het bestand is niet geupload.";
						}
		       	exit;
					}
					else {
					//pad bepalen om naar toe te posten (dus zichzelf, maar dan inclusiet C OF A!!)
					$pad = "/LOFAR-craft/algemene_functionaliteit/bestand_uploaden.php"; 
					if(isset($_GET['c'])) $pad = $pad . ("?c=" . $_GET['c']);
					if(isset($_GET['a'])) $pad = $pad . ("?a=" . $_GET['a']);
				?>
				<form name="bestand" action="<?php echo($pad); ?>" method="post" enctype="multipart/form-data">
					Selecteer hieronder een bestand en <br>
					druk daarna op "Uploaden" om het bestand te uploaden.<br>
					Het geuploade bestand wordt daadwerkelijk opgeslagen,<br>
					wanneer het component of de melding opgeslagen wordt.<br><br>
					<input type="file" name="file" value="test"><br>
					<input type="hidden" name="opslaan" value="1">
					<br><a href="javascript:document.bestand.submit();">Uploaden</a>
	
				</form>
				<?php
					}
				?>

			</body>
		</html>	
<?
	}
?>
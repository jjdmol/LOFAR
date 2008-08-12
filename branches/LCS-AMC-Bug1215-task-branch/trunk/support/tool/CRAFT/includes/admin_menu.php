<?php 
	if (isset($_SESSION['admin_deel'])){
?>

<div id="hoofdbalk">
	<div id="admin_hoofdbalk">
  	<div id="spacerbalk">
    </div> 
    <div id="hoofdmenu">
			<div ID="hoofdmenuoptie6"><a class="hoofdmenuoptie" id="hoofdoptie1" href="admin.php?p=1&s=1">Comp.Type</a></div>
			<div ID="hoofdmenuoptie7"><a class="hoofdmenuoptie" id="hoofdoptie2" href="admin.php?p=2&s=1">Component</a></div>
			<div ID="hoofdmenuoptie8"><a class="hoofdmenuoptie" id="hoofdoptie3" href="admin.php?p=3&s=1">Meldingtype</a></div>
			<div ID="hoofdmenuoptie9"><a class="hoofdmenuoptie" id="hoofdoptie4" href="admin.php?p=4&s=1">Melding</a></div>
			<div ID="hoofdmenuoptie14"><a class="hoofdmenuoptie" id="hoofdoptie9" href="admin.php?p=5&s=1">Extra&nbspvelden</a></div>
			<div ID="hoofdmenuoptie10"><a class="hoofdmenuoptie" id="hoofdoptie5" href="admin.php?p=6&s=1">Gebruikersgroep</a></div>
			<div ID="hoofdmenuoptie11"><a class="hoofdmenuoptie" id="hoofdoptie6" href="admin.php?p=7&s=1">Gebruiker</a></div>
			<div ID="hoofdmenuoptie12"><a class="hoofdmenuoptie" id="hoofdoptie7" href="admin.php?p=8&s=1">Contact</a></div>
			<div ID="hoofdmenuoptie13"><a class="hoofdmenuoptie" id="hoofdoptie8" href="admin.php?p=9&s=1">Locatie</a></div>
		</div>
	</div>   
</div>
<div id="subbalk">	
	<div id="submenu">
		<ul>
	  	<li><A name="suboptie1" href="#section1"></A></li>
	    <li><A name="suboptie2" href="#section1"></A></li>
	    <li><A name="suboptie3" href="#section1"></A></li>
	    <li><A name="suboptie4" href="#section1"></A></li>
	    <li><A name="suboptie5" href="#section1"></A></li>
	    <li><A name="suboptie6" href="#section1"></A></li>
		</ul>
	</div>
	<div id="alg_opties">
		<ul>
	  	<?php if (isset($_SESSION['admin_deel']) && $_SESSION['admin_deel'] == 0) { 
	  		echo("<li><A name=\"Admin\" href=\"admin.php?p=0\">Admin-gedeelte</A></li>");
	  	 } else 
	  		echo("<li><A name=\"Admin\" href=\"main.php?p=".$_SESSION['tab']."\">Hoofdapplicatie</A></li>");
	  	?>
	  	<li><A name="LogUit" href="index.php">Uitloggen</A></li>
		</ul>
	</div>
</div>

<?php
	}
?>

<?php
	
	session_start();

?>  
  //functie welke controleert of een documentElement bestaat
  //als dit element bestaat, dan wordt de border verwijderd en de kleur gereset
  function menuOptie(item) {
    if (document.getElementById(item) != null) {
    	document.getElementById(item).style.borderStyle="none"
    	document.getElementById(item).style.backgroundColor='#0066CC'
  	}
  }
  
  //functie welke de optie waar de muis bovenzweeft highlight en er een border om heen plaatst
  function menuSelectie(item) {
		document.getElementById(item).style.borderStyle="solid"
  	document.getElementById(item).style.backgroundColor='#3399CC'
  }
    
  function stel_subbalk_samen(optie) {
    document.getElementById('suboptie1').href=("main.php?p="+optie+"&s=1");
    document.getElementById('suboptie2').href=("main.php?p="+optie+"&s=2");
    document.getElementById('suboptie3').href=("main.php?p="+optie+"&s=3");
    document.getElementById('suboptie4').href=("main.php?p="+optie+"&s=4");
    document.getElementById('suboptie5').href=("main.php?p="+optie+"&s=5");
    document.getElementById('suboptie6').href=("main.php?p="+optie+"&s=6");

  	document.getElementById('suboptie1').innerHTML="";
  	document.getElementById('suboptie2').innerHTML="";
  	document.getElementById('suboptie3').innerHTML="";
  	document.getElementById('suboptie4').innerHTML="";
  	document.getElementById('suboptie5').innerHTML="";
  	document.getElementById('suboptie6').innerHTML="";  

    if (optie == 1) {
  	}
  	else if (optie ==2) {
      <?php 
      	echo("document.getElementById('suboptie1').innerHTML=\"Comp. overzicht\";");
      	if($_SESSION['toevoegen'] == 1)  echo("document.getElementById('suboptie2').innerHTML=\"Comp. toevoegen\";");
      	if($_SESSION['bewerken'] == 1)   echo("document.getElementById('suboptie3').innerHTML=\"Comp. bewerken\";");
      	if($_SESSION['verwijderen'] ==1) echo("document.getElementById('suboptie4').innerHTML=\"Comp. verwijderen\";");
      ?>
      document.getElementById('suboptie5').innerHTML="Comp. zoeken";
  	}	
  	else if (optie ==3) {
    	<?php 
	    	echo("document.getElementById('suboptie1').innerHTML=\"Melding overzicht\";");
	    	if($_SESSION['toevoegen'] == 1)   echo("document.getElementById('suboptie2').innerHTML=\"Melding toevoegen\";");
	    	if($_SESSION['bewerken'] == 1)    echo("document.getElementById('suboptie3').innerHTML=\"Melding bewerken\";");
	    	if($_SESSION['verwijderen'] == 1) echo("document.getElementById('suboptie4').innerHTML=\"Melding verwijderen\";");
      ?>
      document.getElementById('suboptie5').innerHTML="Melding zoeken";      
   	}	
  	else if (optie ==4) {
//    	document.getElementById('suboptie1').innerHTML="Algemeen";
    	document.getElementById('suboptie1').innerHTML="Type componenten";
    	document.getElementById('suboptie2').innerHTML="Componenten";
    	document.getElementById('suboptie3').innerHTML="Type meldingen";
    	document.getElementById('suboptie4').innerHTML="Meldingen";
    	document.getElementById('suboptie5').innerHTML="Historie";
  	}	
  	else if (optie ==5) {
  	}	
  }
  
  function stel_admin_balk_samen(optie) {
    document.getElementById('suboptie1').href=("admin.php?p="+optie+"&s=1");
    document.getElementById('suboptie2').href=("admin.php?p="+optie+"&s=2");
    document.getElementById('suboptie3').href=("admin.php?p="+optie+"&s=3");
    document.getElementById('suboptie4').href=("admin.php?p="+optie+"&s=4");
    document.getElementById('suboptie5').href=("admin.php?p="+optie+"&s=5");
    document.getElementById('suboptie6').href=("admin.php?p="+optie+"&s=6");

  	document.getElementById('suboptie1').innerHTML="";
  	document.getElementById('suboptie2').innerHTML="";
  	document.getElementById('suboptie3').innerHTML="";
  	document.getElementById('suboptie4').innerHTML="";
  	document.getElementById('suboptie5').innerHTML="";
  	document.getElementById('suboptie6').innerHTML="";  

    if (optie == 1) {
    	document.getElementById('suboptie1').innerHTML="Comp. type toevoegen";
    	document.getElementById('suboptie2').innerHTML="Comp. type bewerken";
    	document.getElementById('suboptie3').innerHTML="Comp. type verwijderen";
  	}
  	else if (optie ==2) {
      document.getElementById('suboptie1').innerHTML="Comp. toevoegen";
      document.getElementById('suboptie2').innerHTML="Comp. bewerken";
      document.getElementById('suboptie3').innerHTML="Comp. verwijderen";
  	}	
  	else if (optie ==3) {
      document.getElementById('suboptie1').innerHTML="Melding type toevoegen";
      document.getElementById('suboptie2').innerHTML="Melding type bewerken";
      document.getElementById('suboptie3').innerHTML="Melding type verwijderen";
   	}
  	else if (optie ==4) {
      document.getElementById('suboptie1').innerHTML="Melding toevoegen";
      document.getElementById('suboptie2').innerHTML="Melding bewerken";
      document.getElementById('suboptie3').innerHTML="Melding verwijderen";
  	}
  	else if (optie ==5) {
      document.getElementById('suboptie1').innerHTML="Velden toevoegen";
      document.getElementById('suboptie2').innerHTML="Velden bewerken";
      document.getElementById('suboptie3').innerHTML="Velden verwijderen";
  	}
  	else if (optie ==6) {
      document.getElementById('suboptie1').innerHTML="Gebruikersgroep toevoegen";
      document.getElementById('suboptie2').innerHTML="Gebruikersgroep bewerken";
      document.getElementById('suboptie3').innerHTML="Gebruikersgroep verwijderen";
      //document.getElementById('suboptie4').innerHTML="Groepmail versturen";
  	}
  	else if (optie ==7) {
      document.getElementById('suboptie1').innerHTML="Gebruikers toevoegen";
      document.getElementById('suboptie2').innerHTML="Gebruikers bewerken";
      document.getElementById('suboptie3').innerHTML="Gebruikers verwijderen";
      //document.getElementById('suboptie4').innerHTML="Gebruikers mailen";
  	}
  	else if (optie ==8) {
      document.getElementById('suboptie1').innerHTML="Contacten toevoegen";
      document.getElementById('suboptie2').innerHTML="Contacten bewerken";
      document.getElementById('suboptie3').innerHTML="Contacten verwijderen";
  	}
  	else if (optie ==9) {
      document.getElementById('suboptie1').innerHTML="Locaties toevoegen";
      document.getElementById('suboptie2').innerHTML="Locaties bewerken";
      document.getElementById('suboptie3').innerHTML="Locaties verwijderen";
  	}
  }
  
  //functie om een kader rond de hoofdmenuopties te plaatsen en de huidige keuze te highlighten
  //ook word in deze functie de linkjes van het submenu aangepast
  function changeSubmenu(optie, actief, modus)
  {
    if (modus == 1) {
    	stel_admin_balk_samen(optie)

   		menuOptie('hoofdmenuoptie6');
   		menuOptie('hoofdmenuoptie7');
   		menuOptie('hoofdmenuoptie8'); 
   		menuOptie('hoofdmenuoptie9');
   		menuOptie('hoofdmenuoptie14');
   		menuOptie('hoofdmenuoptie10');
   		menuOptie('hoofdmenuoptie11');
   		menuOptie('hoofdmenuoptie12');
   		menuOptie('hoofdmenuoptie13');
   		
   		if (actief == 1) {
				menuSelectie('hoofdmenuoptie6');     	
	    	document.getElementById('hoofdoptie1').className = "actieveoptie";
   		}
   		else if (actief == 2) {
				menuSelectie('hoofdmenuoptie7');     	
	    	document.getElementById('hoofdoptie2').className = "actieveoptie";
   		}
   		else if (actief == 3) {
				menuSelectie('hoofdmenuoptie8');     	
	    	document.getElementById('hoofdoptie3').className = "actieveoptie";
   		}
   		else if (actief == 4) {
				menuSelectie('hoofdmenuoptie9');     	
	    	document.getElementById('hoofdoptie4').className = "actieveoptie";
   		}
   		else if (actief == 5) {
				menuSelectie('hoofdmenuoptie14');
	    	document.getElementById('hoofdoptie9').className = "actieveoptie";
   		}
   		else if (actief == 6) {
				menuSelectie('hoofdmenuoptie10');     	
	    	document.getElementById('hoofdoptie5').className = "actieveoptie";
   		}
   		else if (actief == 7) {
				menuSelectie('hoofdmenuoptie11');     	
	    	document.getElementById('hoofdoptie6').className = "actieveoptie";
   		}
   		else if (actief == 8) {
				menuSelectie('hoofdmenuoptie12');     	
	    	document.getElementById('hoofdoptie7').className = "actieveoptie";
   		}   		
   		else if (actief == 9) {
				menuSelectie('hoofdmenuoptie13');     	
	    	document.getElementById('hoofdoptie8').className = "actieveoptie";
   		}   		
    }
  	else {
  		stel_subbalk_samen(optie);
   	
   		menuOptie('hoofdmenuoptie1');
   		menuOptie('hoofdmenuoptie2');
   		menuOptie('hoofdmenuoptie3'); 
   		menuOptie('hoofdmenuoptie4');
   		menuOptie('hoofdmenuoptie5');

	    if (actief == 1) {
				menuSelectie('hoofdmenuoptie1');     	
	    	document.getElementById('hoofdoptie1').className = "actieveoptie";
	    }
	    else if (actief == 2) {
				menuSelectie('hoofdmenuoptie2');			
	    	document.getElementById('hoofdoptie2').className = 'actieveoptie';
	    }
	    else if (actief == 3) {
				menuSelectie('hoofdmenuoptie3');			
	    	document.getElementById('hoofdoptie3').className = 'actieveoptie';
	    }
	    else if (actief == 4) {
				menuSelectie('hoofdmenuoptie4');			
	    	document.getElementById('hoofdoptie4').className = 'actieveoptie';
	    }
	    else if (actief == 5) {
				menuSelectie('hoofdmenuoptie5');			
	    	document.getElementById('hoofdoptie5').className = 'actieveoptie';
	    }
	  }
  }
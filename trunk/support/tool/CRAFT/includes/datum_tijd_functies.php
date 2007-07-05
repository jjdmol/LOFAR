<?php 	
	//het controleren van de samenstelling en inhoud van een tijdveld
	function Valideer_Tijd($tijd_string) {
		//de meegegeven string opdelen in uren en minuten
		$tijd=split(":",$tijd_string);
		if ($tijd[0]!="" && $tijd[1]!="") {
			//uren controleren: de waarde moet tussen 0 (12 uur 's nachts en 23 (11 uur 's avonds) liggen
			if (!($tijd[0] >= 0 && $tijd[0] <= 23 ))
				return false;
			//minuten controleren: de waarde moet tussen 0 en 59 liggen
			if (!($tijd[1] >= 0 && $tijd[1] <= 59 ))
				return false;
		}
		//er zijn 1 of meer lege velden, dus dan de controle afbreken
		else return false;
		
		//we hebben dit punt bereikt, dus alle controles zijn succesvol doorlopen
		return true;
	}
	
	
	//het controleren van de samenstelling van een datum veld
	function Valideer_Datum($datum_string) {
		//de meegegeven string opdelen in een dag,maand en jaar
		$datum=split("-",$datum_string);
		//als er geen lege velden zijn, dan verder gaan met de controle
		if ($datum[0]!="" && $datum[1]!="" && $datum[2]!="") {
			
			//wanneer het jaarveld korter dan 4 cijfers is, dan de controle afbreken
			if (strlen($datum[2]) != 4)
				return false; 
			
			//bekijken of de samengestelde datum in de officiele ranges vallen
			if(checkdate($datum[1],$datum[0],$datum[2]) == false) 
				return false;
		} 
		//er zijn 1 of meer lege velden, dus dan de controle afbreken
		else return false;
		
		//we hebben dit punt bereikt, dus alle controles zijn succesvol doorlopen
		return true;
	}
	
?>
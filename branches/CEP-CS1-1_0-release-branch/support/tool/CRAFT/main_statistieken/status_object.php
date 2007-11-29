<?PHP
	class Status_Object
	{
		// *** VARIABELEN ***
		private $status_id;					//de ID van de status (status->Status_ID
		private $status_naam;				//de beschrijving van de status (status->Status)
		private $totaalTijd;				//de totale tijd waar deze status zich in bevonden heeft 
		private $aantal_meldingen;	//aantal malen dat deze status is opgetreden

		//waardes instellen
		function Set_ID($Is, $naam, $tijd, $aantal){
			$this->status_id = $Is;
			$this->status_naam = $naam;
			$this->totaalTijd = $tijd;
			$this->aantal_meldingen = $aantal;
		}

		//de totale tijd van de status verhogen met de meegegeven waarde
		function Add_TotaalTijd($aantal) {
	  	$this->totaalTijd = $this->totaalTijd + $aantal;
		}

		//de aantal malen dat deze melding is opgetreden verhogen met de meegegeven waarde
		function Add_Aantal($aantal) {
	  	$this->aantal_meldingen = $this->aantal_meldingen + $aantal;
		}

		//ID teruggeven
		function Get_ID() {
			return $this->status_id;
		}

		//totaal tijd teruggeven
		function Get_TotaalTijd() {
			return $this->totaalTijd;
		}

		//totaal tijd teruggeven
		function Get_Aantal() {
			return $this->aantal_meldingen;
		}
		
		//Naam teruggeven
		function Get_Naam() {
			return $this->status_naam;
		}
	}
?>
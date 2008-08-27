<?PHP
	class Type_Object
	{
		// *** VARIABELEN ***
		private $comp_id;
		private $comp_naam;
		private $Childarray = array();
	
		function Set_ID($Is, $naam){
			$this->comp_id = $Is;
			$this->comp_naam = $naam;
		}
		
		function Add($child) {
	  	$this->Childarray = $child;
		}
		
		function Get_ID() {
			return $this->comp_id;
		}

		function Get_Childarray() {
			return $this->Childarray;
		}
		
		function Get_Naam() {
			return $this->comp_naam;
		}
	}
?>
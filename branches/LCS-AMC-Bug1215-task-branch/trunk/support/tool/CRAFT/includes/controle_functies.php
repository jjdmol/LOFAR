<?php 
	function telefoon_check($telefoon) {
		$regex = '/([0]{1}[6]{1}[-\s]*([1-9]{1}[\s]*){8})|([0]{1}[1-9]{1}[0-9]{1}[0-9]{1}[-\s]*([1-9]{1}[\s]*){6})|([0]{1}[1-9]{1}[0-9]{1}[-\s]*([1-9]{1}[\s]*){7})/';
		if (preg_match($regex, $telefoon)) 
	    return true;
	 	else 
	    return false;
	}
		
	function mail_check($email) {
		$regex = '/^[\w-\.]+@([\w-]+\.)+[\w-]{2,4}$/';
		if (preg_match($regex, $email)) 
	    return true;
	 	else 
	    return false;
	}
	
	function postcode_check($postcode) {
		$regex = '/^[1-9]{1}[0-9]{3}\s?[A-z]{2}$/';
		if (preg_match($regex, $postcode)) 
	    return true;
	 	else 
	    return false;
	}
?>
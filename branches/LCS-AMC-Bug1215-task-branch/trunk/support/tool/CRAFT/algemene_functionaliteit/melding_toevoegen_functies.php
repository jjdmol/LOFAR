<?php

	session_start();

?>

function PostDocument(url) {
	var type	= document.getElementById('Type_Melding').value;
	document.getElementById('opslaan').value = 0;
	document.theForm.action = url + "&b=" + type;
	document.theForm.submit();
}

function SubmitMeldingBewerken()
{
	var aantal = document.frames['frame_extra_velden'].document.getElementById('aantal').value;

	for(i=0; i < aantal; i++) {
		var type = document.frames['frame_extra_velden'].document.getElementById('t'+i).value;
		var verplicht = document.frames['frame_extra_velden'].document.getElementById('v'+i).value;
		var id = document.frames['frame_extra_velden'].document.getElementById('i'+i).value;
		var naam = document.frames['frame_extra_velden'].document.getElementById('n'+i).value;

		if (type != 5) {
			var waarde = document.frames['frame_extra_velden'].document.getElementById(i).value;		
			document.getElementById(i).value = waarde;
		}

		document.getElementById('t' + i).value = type;
		document.getElementById('v' + i).value = verplicht;
		document.getElementById('i' + i).value = id;
		document.getElementById('n' + i).value = naam;
	}
	
	
	var aantemaken = document.frames['frame_extra_velden'].document.getElementById('aantemaken').value;

	for(i=0; i < aantemaken; i++) {
		var type = document.frames['frame_extra_velden'].document.getElementById('at'+i).value;
		var verplicht = document.frames['frame_extra_velden'].document.getElementById('av'+i).value;
		var id = document.frames['frame_extra_velden'].document.getElementById('ai'+i).value;
		var naam = document.frames['frame_extra_velden'].document.getElementById('an'+i).value;

		if (type != 5) {
			var waarde = document.frames['frame_extra_velden'].document.getElementById('a' + i).value;		
			document.getElementById('a' + i).value = waarde;
		}

		document.getElementById('at' + i).value = type;
		document.getElementById('av' + i).value = verplicht;
		document.getElementById('ai' + i).value = id;
		document.getElementById('an' + i).value = naam;
	}
	
	document.theForm.submit();
}


function SubmitMeldingToevoegen() 
{
	var aantal = document.frames['frame_extra_velden'].document.getElementById('aantal').value;
	
	for(i=0; i < aantal; i++) {
		var type = document.frames['frame_extra_velden'].document.getElementById('t'+i).value;
		var verplicht = document.frames['frame_extra_velden'].document.getElementById('v'+i).value;
		var id = document.frames['frame_extra_velden'].document.getElementById('i'+i).value;
		var naam = document.frames['frame_extra_velden'].document.getElementById('n'+i).value;

		if (type != 5) {
			var waarde = document.frames['frame_extra_velden'].document.getElementById(i).value;
			document.getElementById(i).value = waarde;
		}

		document.getElementById('t' + i).value = type;
		document.getElementById('v' + i).value = verplicht;
		document.getElementById('i' + i).value = id;
		document.getElementById('n' + i).value = naam;
	}

	var s = document.frames['frame_beschrijving'].document.getElementById('sStatus').value;
	var o = document.frames['frame_oplossing'].document.getElementById('sProb_Oplossing').value;
	var b = document.frames['frame_beschrijving'].document.getElementById('sProb_Beschrijving').value;
	
	document.getElementById('hidden_status').value = s;
	document.getElementById('hidden_oplossing').value = o;
	document.getElementById('hidden_beschrijving').value = b;
	
	document.theForm.submit();
}
<?php

	session_start();
?>

function Laad_Gegevens()
{
	var aantal = document.getElementById('aantal').value;
	for(i=0; i < aantal; i++) {
		var waarde = document.getElementById(i).value;
		var type = document.getElementById('t' + i).value;
		var verplicht = document.getElementById('v' + i).value;
		var id = document.getElementById('i' + i).value;
		var naam = document.getElementById('n' + i).value;

		document.frames['frame_extra_velden'].document.getElementById(i).value = waarde;
		document.frames['frame_extra_velden'].document.getElementById('t'+i).value = type;
		document.frames['frame_extra_velden'].document.getElementById('v'+i).value = verplicht;
		document.frames['frame_extra_velden'].document.getElementById('i'+i).value = id;
		document.frames['frame_extra_velden'].document.getElementById('n'+i).value = naam;
	}
}


function switchMelding() 
{
	var y=document.getElementById('type_melding').value;
	document.getElementById('frame_melding').src = "<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_melding.php?c=" + y;
}


function SubmitComponentBewerken() 
{
	var aantal = document.frames['frame_extra_velden'].document.getElementById('aantal').value;
	for(i=0; i < aantal; i++) {
		var type = document.frames['frame_extra_velden'].document.getElementById('t'+i).value;
		var verplicht = document.frames['frame_extra_velden'].document.getElementById('v'+i).value;
		var id = document.frames['frame_extra_velden'].document.getElementById('i'+i).value;
		var naam = document.frames['frame_extra_velden'].document.getElementById('n'+i).value;

		if(type != 5) {
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

		if(type != 5) {
			var waarde = document.frames['frame_extra_velden'].document.getElementById('a' + i).value;		
			document.getElementById('a' + i).value = waarde;
		}

		document.getElementById('at' + i).value = type;
		document.getElementById('av' + i).value = verplicht;
		document.getElementById('ai' + i).value = id;
		document.getElementById('an' + i).value = naam;
	}
	
	var aantal = document.frames['frame_melding'].document.getElementById('aantal').value;
	
	for(i=0; i < aantal; i++) {
		var type = document.frames['frame_melding'].document.getElementById('t'+i).value;
		var verplicht = document.frames['frame_melding'].document.getElementById('v'+i).value;
		var id = document.frames['frame_melding'].document.getElementById('i'+i).value;
		var naam = document.frames['frame_melding'].document.getElementById('n'+i).value;

		if(type != 5) {
			var waarde = document.frames['frame_melding'].document.getElementById(i).value;
			document.getElementById('m_' + i).value = waarde;
		}

		document.getElementById('m_t' + i).value = type;
		document.getElementById('m_v' + i).value = verplicht;
		document.getElementById('m_i' + i).value = id;
		document.getElementById('m_n' + i).value = naam;
	}


	var s = document.frames['frame_melding'].document.getElementById('sStatus').value;
	var m = document.frames['frame_melding'].document.getElementById('sMelding').value;
	var n = document.frames['frame_contact'].document.getElementById('sVerantwoordelijke').value;
	var p = document.frames['frame_parent'].document.getElementById('sComp_Parent').value;

	document.getElementById('hidden_melding').value = m;
	document.getElementById('hidden_status').value = s;
	document.getElementById('hidden_verantwoordelijke').value = n;
	document.getElementById('hidden_parent').value = p;

	document.theForm.submit();
}

function PostDocument(url) {
	var type	= document.getElementById('comp_type').value;
	document.getElementById('opslaan').value = 0;
	document.theForm.action = url + "&c=" + type;
	document.theForm.submit();
}

function PostBewerkenDocument(url) {
	document.theForm.submit();
}

function submitComponentToevoegen() 
{
	var aantal = document.frames['frame_extra_velden'].document.getElementById('aantal').value;
	
	for(i=0; i < aantal; i++) {
		var type = document.frames['frame_extra_velden'].document.getElementById('t'+i).value;
		var verplicht = document.frames['frame_extra_velden'].document.getElementById('v'+i).value;
		var id = document.frames['frame_extra_velden'].document.getElementById('i'+i).value;
		var naam = document.frames['frame_extra_velden'].document.getElementById('n'+i).value;

		if(type != 5) {
			var waarde = document.frames['frame_extra_velden'].document.getElementById(i).value;
			document.getElementById(i).value = waarde;
		}

		document.getElementById('t' + i).value = type;
		document.getElementById('v' + i).value = verplicht;
		document.getElementById('i' + i).value = id;
		document.getElementById('n' + i).value = naam;
	}


	var aantal = document.frames['frame_melding'].document.getElementById('aantal').value;
	
	for(i=0; i < aantal; i++) {
		var type = document.frames['frame_melding'].document.getElementById('t'+i).value;
		var verplicht = document.frames['frame_melding'].document.getElementById('v'+i).value;
		var id = document.frames['frame_melding'].document.getElementById('i'+i).value;
		var naam = document.frames['frame_melding'].document.getElementById('n'+i).value;

		if(type != 5) {
			var waarde = document.frames['frame_melding'].document.getElementById(i).value;
			document.getElementById('m_' + i).value = waarde;
		}

		document.getElementById('m_t' + i).value = type;
		document.getElementById('m_v' + i).value = verplicht;
		document.getElementById('m_i' + i).value = id;
		document.getElementById('m_n' + i).value = naam;
	}

	
	var n = document.frames['frame_contact'].document.getElementById('sVerantwoordelijke').value;
	var s = document.frames['frame_melding'].document.getElementById('sStatus').value;
	var m = document.frames['frame_melding'].document.getElementById('sMelding').value;
	var w = document.frames['frame_parent'].document.getElementById('sComp_Parent').value;
	var x = document.frames['frame_naam'].document.getElementById('sComp_Naam').value;
	var y = document.frames['frame_fabricant'].document.getElementById('sComp_Fabricant').value;
	var z = document.frames['frame_leverancier'].document.getElementById('sComp_Leverancier').value;
	var levertijd  = document.frames['frame_leverancier'].document.getElementById('sLevertijd').value;
	var leverdatum = document.frames['frame_leverancier'].document.getElementById('sLeverdatum').value;
	var fabricagetijd  = document.frames['frame_fabricant'].document.getElementById('sFabricagetijd').value;
	var fabricagedatum = document.frames['frame_fabricant'].document.getElementById('sFabricagedatum').value;

	<?php

		if(isset($_SESSION['admin_deel']) && 	$_SESSION['admin_deel'] > 0) {
			
			echo("if (document.frames['frame_parent'].document.getElementById('schaduw')) { \n");
			echo("var q = document.frames['frame_parent'].document.getElementById('schaduw').value;\n");
			echo("document.getElementById('hidden_schaduw').value = q; \n }");
		}

	?>
	
	document.getElementById('hidden_verantwoordelijke').value = n;
	document.getElementById('hidden_parent').value = w;
	document.getElementById('hidden_naam').value = x;
	document.getElementById('hidden_fabricant').value = y;
	document.getElementById('hidden_leverancier').value = z;
	document.getElementById('hidden_leverdatum').value = leverdatum;
	document.getElementById('hidden_levertijd').value = levertijd;
	document.getElementById('hidden_fabricagedatum').value = fabricagedatum;
	document.getElementById('hidden_fabricagetijd').value = fabricagetijd;
	document.getElementById('hidden_melding').value = m;
	document.getElementById('hidden_status').value = s;

	document.theForm.submit();
}
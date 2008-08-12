<?php
  //functies om de velden mbt het zoeken van gegevens te regelen
?>


function switchDocument(selectveld, textveld)
{
	if (document.getElementById(selectveld).value == -1 )
		document.getElementById(textveld).value = '';	
}

# NodeRed Dashboard

Het dashboard bestaat uit 3 onderdelen, van links naar rechts de gallerij, het manueel controlepaneel en het auto-route controlepaneel.

![](./assets/img/nodered%20dashboard.png)
_NodeRed Dashboard 1_

## Gallerij
De gallerij laat wanneer de pagina voor het eerst geladen wordt de eerst afbeelding uit de database zien. Met de getallen onder de afbeelding weet de gebruiker de hoeveelste afbeelding dat hij/zij bekijkt. Dit getal update ook automatisch wanneer een nieuwe foto wordt toegevoegd. Om andere afbeeldingen te kiezen kan de gebruiker de twee pijlen gebruiken. De afbeelding wordt dan telkens uit de database gehaald op basis van welke foto wordt gevraagd. Moest er een probleem zijn met de foto's in de database kan de gebruiker ze ook verwijderen met de CLEAR knop.

## Manueel controlepaneel

Het control panel heeft 7 verschillende knoppen en een slider. Met de knoppen kan je naar links, rechts, beneden en boven gaan, maar ook stoppen en een foto nemen. Dit is bedoeld om de camera manueel te kunnen bewegen. De laatste knop is calibrate, eens hierop is gedrukt zal bij de "Calibration status" de tekst "Calibrating" komen te staan. Eens de machine klaar is met kalibreren zal deze tekst veranderen naar "done". Zo weet de gebruiker dat de machine klaar is met kalibreren. Tenslotte is er ook nog een slider om de snelheid van de motoren te selecteren. Deze slider bepaald hoeveel microsteps de motor uitvoert in 1 clock cycle.

## Auto-route controlepaneel

Het auto-route gedeelte geeft de gebruiker de mogelijkheid om een foto te nemen van één plant naar keuze, of van alle planten in één keer. Vooraleer de gebruiker dit kan doen moet hij wel eerst de kalibratie uitvoeren of manueel de camera naar de hoek links onder brengen met het control panel. De foto's worden dan getoond in de gallerij.
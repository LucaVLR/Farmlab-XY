# Jelte
## StallGuard
Vorig semester begonnen we met deze functie te onderzoeken, maar hadden we niet genoeg tijd om dit af te ronden. 
Dus zal ik beginnen met deze functie onder te zoeken om het te kunnen toepassen in ons project.
De StallGuard functie is zeer simpel, het is een waarde die kan worden uitgelezen. Deze waarde is recht evenredig met hoeveel stroom de motor trekt van de bron.
De TMC heeft ook een instelbare threshold voor deze waarde. Eens de StallGuard waarde boven deze threshold komt, zal de DIAG pin hoog worden gemaakt.
Deze verandering kan gedetecteerd worden door de microcontroller en de motoren stoppen.

Een probleem dat ik ondervonden heb, is dat het register van de threshold maar 8 bits is terwijl de StallGuard waarde groter dan 8 bits is.
Omdat de motoren altijd veel stroom trekken is deze waarde soms boven de threshold ook al moeten ze niet stoppen met draaien.
Een oplossing hiervoor is gewoon de instelbare threshold niet gebruiken en zelf de threshold te maken met een simpele if-regel.

Door deze functie van de TMC hebben we geen nood meer aan de limit switches waardoor de code minder complex is en besparen we een aantal GPIO pinnen.

## Kalibratie
Met de StallGuard functie kunnen we ook een kalibratie functie maken voor het XY-systeem. Dit houdt in dat de camera naar één van de hoeken gaat zodat het op een 0 punt staat.
Dan kan later door het gebruik van coördinaten een autoroute functie ontworpen worden. De kalibratie is zeer eenvoudig, eens het start zal de camera naar onder beginnen te bewegen.
Als de camera helemaal beneden is zullen de motoren verder proberen draaien, ook al kunnen ze niet, hierdoor trekken ze meer stroom en wordt de StallGuard getriggerd.
De motoren zullen stoppen en na een seconden wachten zal de camera beginnen naar links te gaan. Als de camera dan helemaal links is zullen de motoren ook stoppen.
De camera staat dan links benenden en is klaar voor de autoroute.

Er was één enkel probleem met de kalibratie. Soms zou de StallGuard getriggerd worden bij de start van de kalibratie.
Dit kan simpel gefixt worden door de kalibratie altijd 2 te activeren. Eens ik dit toepaste, is de kalibratie nooit meer misgelopen.
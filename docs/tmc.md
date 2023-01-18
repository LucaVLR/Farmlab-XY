# TMC2209-LA

De TMC2209 is een bipolar stepper motor driver. De chip komt in een QFN28 package en heeft verschillende opertation modes en eigenschappen zoals: STEP/DIR interface, Smooth Running 256 Microstep, StealthChop, SpreadCycle, StallGuard, een UART interface en nog meer. We kozen deze chip omdat deze beschikt over handige eigenschappen die wij zochten, zoals de UART interface en StallGuard. Alle informatie over de TMC2209 is gevonden in de [datasheet](https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2209_Datasheet_V103.pdf).

![](./assets/img/blokschema%20TMC.png)

_TMC2209 block diagram_

## StallGuard

StallGuard is een zeer nuttige functie van de TMC2209 die we gebruiken om de nood aan limit switches te verwijderen. Deze methode was voor ons meer voordelig dan de limit switches, want de ESP-CAM beschikt niet over heel veel IO-pins. De reden waarom we normaal limit switches nodig hebben is om aan het systeem te laten weten wanneer de camera tegen de rand van het XY-systeem gaat botsen zodat de motoren niet verder kunnen draaien. Wat StallGuard doet is eigenlijk zeer simpel. Het gaat de stroom door de motor constant meten. Als een stappenmotor probeert te draaien maar iets houdt de schacht tegen, dan zal de stroom door de motor verhogen. Dus wanneer de camera botst tegen de rand en de motoren proberen verder te draaien zullen deze geblokkeerd worden en meer stroom trekken. Deze waarde wordt vergeleken met een instelbare threshold. Wanneer de StallGuard waarde boven de threshold komt zal de DIAG pin hoog worden. We kunnen het spanningsniveau deze pin meten via de ESP en zo weten wanneer de camera niet verder mag bewegen. Door de TMC dan kort uit en terug aan te zetten door een puls naar de ENN pin te sturen, zal de DIAG pin terug laag worden. Dit kan ook op een simpelere manier, door de DIAG pin rechtstreeks aan de ENN pin te verbinden.

Dit klinkt allemaal heel goed op papier, maar in de realiteit werkt niet altijd alles zo perfect als verwacht. Er zijn enkele nadelen met het StallGuard systeem die we ondervonden tijdens het integreren. Één van de problemen is dat de instalbare threshold een 8-bit waarde is (max 255), maar de StallGuard waarde die de chip meet is groter dan een 8-bit waarde. Meestal was de default waarde wanneer de motoren bewogen al rond de 500. Hierdoor konden we niet de DIAG pin gebruiken, maar we vonden wel een oplossing hiervoor. Door zelf in onze code de waarde in te lezen, zelf een instelbare threshole te hebben en de motoren zelf te stoppen, is dit probleem opgelost.

Een ander probleem dat we hebben vastgesteld is dat de meting van het StallGuard systeem niet altijd heel goed is. Hierdoor moeten we bij de kalibratie de code een keer herhalen om de kans op fouten zo klein mogelijk te maken.

## UART Interface

De TMC2209 beschikt ook over een UART interface om te communiceren met de ESP. Deze manier van communiceren was voor ons meer voordelig dan het STEP/DIR interface want de ESP-CAM beschikt niet over heel veel IO-pins. De manier waarop de ESP en TMC zullen communiceren heet 1-wire UART. We kunnen ook hiermee verschillende slaves aansturen met 1 master. Dit is mogelijk door de MS1 en MS2 pins, deze pins worden normaal gebruikt om de step resolution in te stellen, maar kunnen ook gebruikt worden om het slave address in te stellen.

![](./assets/img/blokschema%20TMC%20UART.png)

_UART Communicatie met meerdere slaves_

Ook hier ondervonden we de nodige problemen. Zoals eerder vermeld kan je de step resolution en slave address instellen via dezelfde pins. Wanner we de chips hun slave address gaven zouden ze ook elk een andere step resolutie hebben waardoor 1 motor sneller draaide dan de andere. Dit hebben we simpel kunnen oplossen door de registers rechtstreeks aan te spreken en de functionaliteit van de MS pin aan te passen naar alleen slave address ([datasheet p20](https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2209_Datasheet_V103.pdf#page=20)).

## Software library

Zelf elk register aansturen via UART is een zeer grote taak. Daarom hebben we gezocht naar een library die dit werk voor ons zal doen. We vonden een library genaamd TMC2209 van Peter Polidoro die alles kon doen wat we nodig hadden. Hier is de [GitHub repository](https://github.com/janelia-arduino/TMC2209). Met deze library kunnen we met simpele methodes de snelheid, resolutie, run current, stop current en nog meer instellen. Omdat het werkt met OOP konden we ook heel simpel 2 verschillende motoren aansturen met deze library.

Maar er was een klein probleem met deze library in het begin. Het probleem was dat de ESP niet zou communiceren met de TMC ook al was de setup correct. Nadat we in de code van de library keken vonden we een variable genaamd "blocking\_" deze variable zal alle communicatie blokkeren tot de ESP en TMC kunnen communiceren. Maar ook al was de setup correct, deze variable wou niet van waarde veranderen. Dus onze oplossing was om deze variable er uit te halen. Na deze kleine aanpassing werkte de communicatie perfect.

Iets anders waarvoor opgelet moet worden in deze library is dat er al code is om de Serial van de ESP klaar te zetten. Als je dan nog zelf Serial.begin() zet in de ESP code zal deze crashen en voor problemen zorgen.

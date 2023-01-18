# Kalibratiesysteem

Het kalibratiesysteem zal StallGuard gebruiken om de motor naar de hoek links onder van het XY-systeem te brengen. In onze software is het kalibratiesysteem een simpele functie met als argument de threshold voor StallGuard. Het bestaat uit 2 loops, de eerste om naar links te gaan en de volgende om naar onder te gaan. Als de StallGuard waarde van motor 1 of motor 2 over de threshold komt dan zullen ze beiden stoppen en naar de volgende richting gaan. Deze functie wordt altijd 2 keer doorlopen (met gebruik van een for loop) om het risico op fouten zo klein mogelijk te maken. Ten slotte wordt er nog een bericht gestuurd naar het dashboard om de gebruiker te laten weten dat de machine klaar is met kalibreren.

![](RackMultipart20230118-1-es64sw_html_9ec00279d43bf50a.png)

_Kalibrate functie_
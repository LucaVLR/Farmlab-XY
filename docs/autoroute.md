# 5Autoroute

Voor we aan de autoroute konden beginnen moesten we eerst een paar afmetingen nemen, berekeningen maken en regels vastlegen. Hieronder zie je een mockup van het resultaat van onze metingen en berekeningen. Elke coördinaat representeert 100ms van beweging van de camera. Dus bv. bij plant 1 is de X-coördinaat 28.4, dit betekent dat de camera 2.84 seconden lang naar rechts zal moeten gaan vanaf het startpunt. Deze coördinaten worden doorgestuurd via het NodeRed dashboard. Elke knop stuurt de respectievelijke coördinaten. De Autoroute knop zal dan een grote string van alle coördinaten doorsturen.

![](RackMultipart20230118-1-es64sw_html_ea5cf3530faeae82.png)

_Autoroute mockup_

## 5.1Single Autoroute

Single autoroute wordt gedaan door een simpele functie die als agruments de X en Y-coördinaat krijgen. Deze 2 variabelen komen van een andere functie die de string van het MQTT bericht zal parsen naar 2 floats. Voor deze functie kan beginnen moet de camera altijd gekalibreerd zijn, dus in de hoek linker onderaan staan. Hierdoor zal de camera nooit naar beneden moeten gaan, dus moet dit ook niet geprogrammeerd worden. Als X negatief is moet de camera naar links gaan en vice versa. De delay is dan gewoon de waarde van de coördinaat maal 100ms, want 1 coördinaat = 100ms.

![](RackMultipart20230118-1-es64sw_html_985eed1ccc0426a3.png)

_Autoroute code 1_
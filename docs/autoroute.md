# Autoroute

Voor we aan de autoroute konden beginnen moesten we eerst een paar afmetingen nemen, berekeningen maken en regels vastlegen. Hieronder zie je een mockup van het resultaat van onze metingen en berekeningen. Elke coördinaat representeert 100ms van beweging van de camera. Dus bv. bij plant 1 is de X-coördinaat 28.4, dit betekent dat de camera 2.84 seconden lang naar rechts zal moeten gaan vanaf het startpunt. Deze coördinaten worden doorgestuurd via het NodeRed dashboard. Elke knop stuurt de respectievelijke coördinaten. De Autoroute knop zal dan een grote string van alle coördinaten doorsturen.

![](./assets/img/autoroute%20mockup.png)
  
_Autoroute mockup_

## Single Autoroute

Single autoroute wordt gedaan door een simpele functie die als agruments de X en Y-coördinaat krijgen. Deze 2 variabelen komen van een andere functie die de string van het MQTT bericht zal parsen naar 2 floats. Voor deze functie kan beginnen moet de camera altijd gekalibreerd zijn, dus in de hoek linker onderaan staan. Hierdoor zal de camera nooit naar beneden moeten gaan, dus moet dit ook niet geprogrammeerd worden. Als X negatief is moet de camera naar links gaan en vice versa. De delay is dan gewoon de waarde van de coördinaat maal 100ms, want 1 coördinaat = 100ms.

## Full Autoroute

Met de volledige autoroute gebruiken we dezelfde functie als de single autoroute. Het enige verschil is dat we deze een paar keer loopen voor dat we terug kalibreren. Het dashboard stuurt alle coördinaten door en het systeem weet wannneer dat een enkele of volledige autoroute gevraagt wordt.

Telkens dat de camera op de positie van een plant komt, pakt ze een foto. Deze wordt dan geconverteerd en doorgestuurd terwijl dat naar de volgende plant bewogen wordt.

## Code

```cpp
void autoRoute(float x, float y) {
  if(x >= 0.0) {
    stepper_driver.moveAtVelocity(-RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(-RUN_VELOCITY);
    delay(x*100);
  }
  else {
    stepper_driver.moveAtVelocity(RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(RUN_VELOCITY);
    delay((-x)*100);
  }

  stepper_driver.moveAtVelocity(-RUN_VELOCITY);
  stepper_driver2.moveAtVelocity(RUN_VELOCITY);
  delay(y*100);
  
  stepper_driver.moveAtVelocity(0);
  stepper_driver2.moveAtVelocity(0);
  delay(1000);
  startCalibration = true;
}

```

_Autoroute code_

![](./assets/img/autoroute%20blockdiagram.png)

_Autoroute code blokdiagram_

# Kalibratiesysteem

Het kalibratiesysteem zal StallGuard gebruiken om de motor naar de hoek links onder van het XY-systeem te brengen. In onze software is het kalibratiesysteem een simpele functie met als argument de threshold voor StallGuard. Het bestaat uit 2 loops, de eerste om naar links te gaan en de volgende om naar onder te gaan. Als de StallGuard waarde van motor 1 of motor 2 over de threshold komt dan zullen ze beiden stoppen en naar de volgende richting gaan. Deze functie wordt altijd 2 keer doorlopen (met gebruik van een for loop) om het risico op fouten zo klein mogelijk te maken. Ten slotte wordt er nog een bericht gestuurd naar het dashboard om de gebruiker te laten weten dat de machine klaar is met kalibreren.

```cpp
void calibrateXY(unsigned int th) {
  for(byte i = 0; i < 2; i++) {
    stepper_driver.moveAtVelocity(RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(RUN_VELOCITY);
    delay(10);
    
    while(1) {
      if(stepper_driver.getStallGuardResult() > th || stepper_driver2.getStallGuardResult() > th) {
        stepper_driver.moveAtVelocity(0);
        stepper_driver2.moveAtVelocity(0);
        delay(250);
        break;
      }
    }
  
    stepper_driver.moveAtVelocity(RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(-RUN_VELOCITY);
    delay(10);
    
    while(1) {
      if(stepper_driver.getStallGuardResult() > th || stepper_driver2.getStallGuardResult() > th) {
        stepper_driver.moveAtVelocity(0);
        stepper_driver2.moveAtVelocity(0);
        delay(250);
        break;
      }
    }    
  }

  client.publish("/MCU/CALIBRATION", "done");
}
```

_Kalibrate functie_

## Blokdiagram
![](./assets/img/kalibratie%20blockdiagram.png)

_Kalibratie blokdiagram_

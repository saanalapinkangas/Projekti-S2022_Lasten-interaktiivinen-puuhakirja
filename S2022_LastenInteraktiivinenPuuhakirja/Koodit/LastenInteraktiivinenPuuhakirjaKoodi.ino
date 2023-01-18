#define nappienLkm 4
#define paavariLedienLkm 3

/* Pinnit Arduino Unolla / Mega:
MISO 12 / 50
SCK 13 / 52
MOSI 11 / 51
SS eli chipselect / 53
*/

#include <SD.h>               // lisää nämä kirjastot mukaan (SD, TMRpcm, SPI)
#define SD_ChipSelectPin 53   // valitse pinni johon yhdistät sd-kortin
#include <TMRpcm.h>
#include <SPI.h>

TMRpcm tmrpcm;      // määritetään äänikutsu

const int nappi1 = 24; //numerosivu: 1
const int nappi2 = 25; //numerosivu: 1
const int nappi3 = 26; //numerosivu: 1
const int nappi4 = 27; //numerosivu: 1
const int nappi5 = 28; //numerosivu: 1
const int nappi6 = 29; //numerosivu: 1
const int nappi7 = 30; //numerosivu: 1
const int nappi8 = 31; //numerosivu: 1
const int nappi9 = 32; //numerosivu: 1
const int nappi10 =33; //numerosivu: 1
const int nappi11 = 34; //värisivu: sininen
const int nappi12 = 35; //värisivu: punainen
const int nappi13 = 36; //värisivu: keltainen
const int nappi14 = 37; //värsivu: sekoitusnappi

const int led1 = 38; //numerosivu: 1, värisivu:sininen
const int led2 = 39; //numerosivu: 2, värisivu: punainen
const int led3 = 40; //numerosivu: 3, värisivu: keltainen
const int led4 = 41; //numerosivu: 4, värisivu: violetti
const int led5 = 42; //numerosivu: 5, värisivu: vihreä
const int led6 = 43; //numerosivu: 6, värisivu: oranssi
const int led7 = 44; //numerosivu: 7, värisivu: ruskea
const int led8 = 45; //numerosivu: 8
const int led9 = 46; //numerosivu: 9
const int led10 = 47; //numerosivu: 10

uint16_t edellisetNapit = 0xffff;
uint16_t vilkutuslaskuri = 0xffff;

bool nappiOnPainettu1(void);
bool nappiOnPainettu2(void);
bool nappiOnPainettu3(void);
bool nappiOnPainettu4(void);
bool nappiOnPainettu5(void);
bool nappiOnPainettu6(void);
bool nappiOnPainettu7(void);
bool nappiOnPainettu8(void);
bool nappiOnPainettu9(void);
bool nappiOnPainettu10(void);

void numerosivu(void);
void alustaTimer(void);
uint16_t nappienTila(void);
void ledienTila(uint16_t, uint16_t);
uint16_t montakoNappiaPainettu(uint16_t);
bool nappiPainettuna(uint16_t, uint16_t);
void napitLedeiksi(uint16_t, uint16_t);

void setup() {
  // put your setup code here, to run once:
  tmrpcm.speakerPin=11;
  Serial.begin(9600);

  randomSeed(analogRead(0));

  pinMode(nappi1, INPUT_PULLUP);
  pinMode(nappi2, INPUT_PULLUP);
  pinMode(nappi3, INPUT_PULLUP);
  pinMode(nappi4, INPUT_PULLUP);
  pinMode(nappi5, INPUT_PULLUP);
  pinMode(nappi6, INPUT_PULLUP);
  pinMode(nappi7, INPUT_PULLUP);
  pinMode(nappi8, INPUT_PULLUP);
  pinMode(nappi9, INPUT_PULLUP);
  pinMode(nappi10, INPUT_PULLUP);
  pinMode(nappi11, INPUT_PULLUP);
  pinMode(nappi12, INPUT_PULLUP);
  pinMode(nappi13, INPUT_PULLUP);
  pinMode(nappi14, INPUT_PULLUP);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);
  pinMode(led9, OUTPUT);
  pinMode(led10, OUTPUT);

  alustaTimer();

  digitalWrite(SD_ChipSelectPin, HIGH);
  pinMode(SD_ChipSelectPin, OUTPUT);
  if(!SD.begin(SD_ChipSelectPin))   // testataan kortin toimivuus
  {
    Serial.println("SD fail");
    return;
  }
  tmrpcm.setVolume(6);    // äänimääritys 1-6
  //tmrpcm.play("varikeltainen.wav");
  //delay(2000);
  //tmrpcm.play("keltainen.wav");
  //delay(1000);
  tmrpcm.play("numero10.wav");
  delay(1000);
  tmrpcm.play("yksiversio3.wav");
}

void pysaytaTimer()
{
  // Pysäytä timer
  TCCR1B &= ~(1 << CS10);
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS12);
  // Sammuta ledit
  ledienTila(0xffff, led1);
  vilkutuslaskuri = 0xffff;
  edellisetNapit = 0xffff;
}

ISR(TIMER1_COMPA_vect)
{
  if (vilkutuslaskuri < 11) //vilkutuksen keston säätäminen. Mitä isompi luku, sitä kauemmin vilkkuvat
  {
    // Jos alin bitti on yksi -> Ledit päälle. Sammuta ledit jos alin bitti 0
    if (vilkutuslaskuri & 1)
    {
      ledienTila(edellisetNapit, led1);
    }
    else
    {
      ledienTila(0xffff, led1);
    }
    vilkutuslaskuri++;
  }
  else
  {
    pysaytaTimer();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  
  pysaytaTimer();
  while (1)
  {
    if(vilkutuslaskuri == 0xFFFF)
    {
      numerosivu();
    }
    uint16_t testi = nappienTila();
    uint16_t nappejaPainettu = montakoNappiaPainettu(testi); 
    napitLedeiksi(testi, nappejaPainettu);
  }
}

void alustaTimer(void)
{
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TCCR1B |= (0 << CS11);
  TCCR1B |= (1 << CS12);
  OCR1A = 6000; //nappien vilkkumisnopeuden säätäminen. Mitä isompi luku, sitä hitaampaa vilkkuu.
  TIMSK1 |= (1 << OCIE1A);
  vilkutuslaskuri = 0;
  interrupts();
}

uint16_t nappienTila (void) //funktio tallentaa bittimuotoon nappien tilat (0 tai 1) muuttujaan nappienBitit
{
  uint16_t nappienBitit = 0; 
  for (int i = 0; i < nappienLkm; i++)
  {
    uint16_t napinTila = digitalRead(i+34); // napit pinneissä 24-37. +34 viittaa värisivun nappien aloituspinniin
    nappienBitit |= napinTila << i;
  }
  return nappienBitit;
}

uint16_t montakoNappiaPainettu(uint16_t nappienTilat) //funktio laskee, kuinka monta nappia on painettu samaan aikaan
{
  uint16_t nappejaPainettu = 0;
  for (int i = 0; i < nappienLkm; i++)
  {
    int bitti = (nappienTilat>>i)&0x01;
    if (bitti == 0)
    {
      nappejaPainettu++;
    }
  }
  return nappejaPainettu;
}

void ledienTila(uint16_t nappienTilat, uint16_t aloituspinni) //funktio kirjoittaa ledeille ledien tilan bittimuotoon nappien tilan perusteella. Aloituspinni on led1 pinni eli 38
{
  for(int i = 0; i< paavariLedienLkm; i++)
  {
    uint16_t bitti = (nappienTilat>>i)&0x01;
    uint16_t kaannos = ~bitti&0x01;
    digitalWrite(i+aloituspinni, kaannos);
  }
}

bool nappiPainettuna(uint16_t nappienTilat, uint16_t indeksi) //funktio tarkistaa, onko yksittäinen nappi painettuna. Indeksi kertoo, mikä nappi kyseessä 0=sininen, 1=punainen, 2=keltainen, 3=sekoitusnappi)
{
  return (~nappienTilat & (1 << indeksi)) != 0;
}

void aanet(uint16_t nappienTilat, uint16_t nappejaPainettu)
{
  if (nappiPainettuna(nappienTilat, 0))
  {
    tmrpcm.play("blue.wav");
    delay(1500);
  }
  if (nappiPainettuna(nappienTilat, 1))
  {
    tmrpcm.play("red.wav");
    delay(1500);
  }
  if (nappiPainettuna(nappienTilat, 2))
  {
    tmrpcm.play("yellow.wav");
    delay(1500);
  }
}

void napitLedeiksi(uint16_t nappienTilat, uint16_t nappejaPainettu) //funktiossa sekoitusnapin toiminto eli kun sekoitusnappia painetaan, välivärin led syttyy. Funktiossa myös sekoitusnappia edeltävä ledin vilkutus. Funktio tallentaa edellisetNapit globaaliin muuttujaan edellisten nappien tilan.
{
  if (nappiPainettuna(nappienTilat, 3)) //indeksi 3 tarkoittaa sekoitusnappia. Sekoitusnappi painettuna.
  {
    if (montakoNappiaPainettu(edellisetNapit) > 1)
    {
      uint16_t sekoitusNapit = edellisetNapit; //muuttujassa edellisten nappien eli päävärinapppien tila
      pysaytaTimer();
      if (montakoNappiaPainettu(sekoitusNapit) == 2) //jos päävärinapeista painettuna 2kpl
      {
        if(nappiPainettuna(sekoitusNapit, 0) && nappiPainettuna(sekoitusNapit, 1)) //sininen ja punainen nappi painettuna. led4 eli violetti pitäisi syttyä Kytkentä ja kirjan värien sijoitus tarkistettava, että vastaavat oikeita värejä
        {
          tmrpcm.play("violet.wav");
          digitalWrite(3+led1, 1);
        }
        else if (nappiPainettuna(sekoitusNapit, 0) && nappiPainettuna(sekoitusNapit, 2)) //sininen ja keltainen nappi painettuna. led5 eli vihreä pitäisi syttyä.
        {
          tmrpcm.play("green.wav");
          digitalWrite(4+led1, 1);
        }
        else //punainen ja keltainen nappi painettuna. led6 eli oranssi pitäisi syttyä.
        {
          tmrpcm.play("orange.wav");
          digitalWrite(5+led1, 1);
        }
      }
      else //sininen, punainen ja keltainen nappi painettuna. led7 eli ruskea pitäisi syttyä.
      {
        tmrpcm.play("brown.wav");
        digitalWrite(6+led1, 1);
      }
      //tänne äänet. Delay poistetaan ja laitetaan tilalle äänen soitto.
      delay(1500);
      for(int i = 0; i < 4; i++)
      {
        digitalWrite(3+i+led1, 0); //ledien sammuttaminen
      }
    }
  }
  else //sekoitusnappi ei painettuna
  {
    if(vilkutuslaskuri == 0xffff) //vilkutuslaskuria ei käynnistetty eli ei aloitettu vilkuttamaan
    {
      ledienTila(nappienTilat, led1);
    }
    if (nappejaPainettu == 0) // napeista päästetty irti eli nappeja painettu 0kpl
    {
      if ((montakoNappiaPainettu(edellisetNapit) > 1) && (vilkutuslaskuri == 0xffff)) // nappeja painettu enemmän kuin 0kpl ja vilkutuslaskuria ei käynnistetty
      {
        alustaTimer(); //timerin avulla aloitetaan vilkutus
      }
      else if(montakoNappiaPainettu(edellisetNapit) == 1)
      {
        aanet(edellisetNapit, 1);
        edellisetNapit = 0xffff;
      }
    }
    else if (nappejaPainettu == 1) // nappeja painettu 2kpl eli ei vielä päästetty irti. Ei siis vilkuta
    {
      if ((montakoNappiaPainettu(edellisetNapit) < 2))  
      {
        edellisetNapit = nappienTilat; // tallentaa edellisetNapit muuttujaan nappien tilat
      }
    }
    else if (nappejaPainettu == 2) // nappeja painettu 2kpl eli ei vielä päästetty irti. Ei siis vilkuta
    {
      if ((montakoNappiaPainettu(edellisetNapit) != 3))  
      {
        edellisetNapit = nappienTilat; // tallentaa edellisetNapit muuttujaan nappien tilat
      }
    }
    else if (nappejaPainettu == 3) // nappeja painettu 3kpl eli ei vielä päästetty irti. Ei siis vilkuta
    {
      edellisetNapit = nappienTilat;
    }
  }
}

bool nappiOnPainettu1(void) {
  int buttonPushed1 = digitalRead(nappi1);
  if (buttonPushed1 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu2(void) {
  int buttonPushed2 = digitalRead(nappi2);
  if (buttonPushed2 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu3(void) {
  int buttonPushed3 = digitalRead(nappi3);
  if (buttonPushed3 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu4(void) {
  int buttonPushed4 = digitalRead(nappi4);
  if (buttonPushed4 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu5(void) {
  int buttonPushed5 = digitalRead(nappi5);
  if (buttonPushed5 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu6(void) {
  int buttonPushed6 = digitalRead(nappi6);
  if (buttonPushed6 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu7(void) {
  int buttonPushed7 = digitalRead(nappi7);
  if (buttonPushed7 == 0) {return true;}
  else {return false;}
  }
  
bool nappiOnPainettu8(void) {
  int buttonPushed8 = digitalRead(nappi8);
  if (buttonPushed8 == 0) {
    return true;}
  else {
    return false;}
  }
bool nappiOnPainettu9(void) {
  int buttonPushed9 = digitalRead(nappi9);
  if (buttonPushed9 == 0) {
    return true;}
  else {
    return false;}
  }
bool nappiOnPainettu10(void) {
  int buttonPushed10 = digitalRead(nappi10);
  if (buttonPushed10 == 0) {
    return true;}
  else {
    return false;}
  }

void numerosivu(void)
{
  int arrayOfLEDS[10] = {38, 39, 40, 41, 42, 43, 44, 45, 46, 47}; // Ledien ohjauspinnit.
      
      int randomNumber1;                                       // alustetaan 5 muuttujaa arvottaville numeroille.
      int randomNumber2;
      int randomNumber3;
      int randomNumber4;
      int randomNumber5;

      int randomPosition;                                     // alustetaan muuttuja randomia taulukon sijaintia varten (alkiota?).
        
      if (nappiOnPainettu1()){
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        tmrpcm.play("01.wav");
          while(nappiOnPainettu1()){
            digitalWrite(randomNumber1, HIGH);
//delay(3000);  
            delay(5);
            }
      }
      else if (nappiOnPainettu2())
      {
      randomPosition = random(0, 9);               // Arvotaan 1. random ledpinniä vastaava sijainti taulukosta. Pinnejä (ja ledejä) on yht 10 kpl.
      randomNumber1 = arrayOfLEDS[randomPosition]; // Sijoitetaan randomNumber1:si arvottua alkiota vastaava pinninumero.
      arrayOfLEDS[randomPosition] = 99;            // Vaihdetaan alkion paikalle pinniksi kelpaamaton nro.
      randomPosition = random(0, 9);               // Arvotaan 2. random ledpinniä vastaava sijainti taulukosta.
      randomNumber2 = arrayOfLEDS[randomPosition]; // Sijoitetaan randomNumber2:si arvottua sijaintia vastaava pinninro.
        if (randomNumber2 != 99)                   // Tarkistetaan onko arvottu toinen nro aiemmin arvottu 1. nro. Toteutus näin, ettei tartte verrata kuin yhteen arvoon. 
        {
          tmrpcm.play("02.wav");
          while (nappiOnPainettu2())               // Tehdään asioita, kun nappi on painettuna.
            {
            digitalWrite(randomNumber1, HIGH);     // Sytytetään ledit.
            digitalWrite(randomNumber2, HIGH);
            delay(5);
            }
          }
        }
      else if (nappiOnPainettu3())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        randomPosition = random(0, 9);
        randomNumber2 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        if (randomNumber2 != 99){ 
          randomPosition = random(0, 9);
          randomNumber3 = arrayOfLEDS[randomPosition];
          arrayOfLEDS[randomPosition] = 99;
            if (randomNumber3 != 99) 
            {
              tmrpcm.play("03.wav");
              while (nappiOnPainettu3()) 
              {
                digitalWrite(randomNumber1, HIGH);
                digitalWrite(randomNumber2, HIGH);
                digitalWrite(randomNumber3, HIGH);
                delay(5);
              }
            }
          }
        }
        else if (nappiOnPainettu4())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        randomPosition = random(0, 9);
        randomNumber2 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        if (randomNumber2 != 99){ 
          randomPosition = random(0, 9);
          randomNumber3 = arrayOfLEDS[randomPosition];
          arrayOfLEDS[randomPosition] = 99;
            if (randomNumber3 != 99) {
              randomPosition = random(0, 9);
              randomNumber4 = arrayOfLEDS[randomPosition];
              arrayOfLEDS[randomPosition] = 99;
                if (randomNumber4 != 99) {
                  tmrpcm.play("04.wav");
                  while (nappiOnPainettu4()) 
                    {
                    digitalWrite(randomNumber1, HIGH);
                    digitalWrite(randomNumber2, HIGH);
                    digitalWrite(randomNumber3, HIGH);
                    digitalWrite(randomNumber4, HIGH);
                    delay(5);
                    }
                }
              }
            }
        }
      else if (nappiOnPainettu5())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        randomPosition = random(0, 9);
        randomNumber2 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        if (randomNumber2 != 99){ 
          randomPosition = random(0, 9);
          randomNumber3 = arrayOfLEDS[randomPosition];
          arrayOfLEDS[randomPosition] = 99;
            if (randomNumber3 != 99) {
              randomPosition = random(0, 9);
              randomNumber4 = arrayOfLEDS[randomPosition];
              arrayOfLEDS[randomPosition] = 99;
                if (randomNumber4 != 99) {
                randomPosition = random(0, 9);
                randomNumber5 = arrayOfLEDS[randomPosition];
                arrayOfLEDS[randomPosition] = 99;
                if (randomNumber5 != 99) {
                  tmrpcm.play("05.wav");
                  while (nappiOnPainettu5()) 
                  {
                    digitalWrite(randomNumber1, HIGH);
                    digitalWrite(randomNumber2, HIGH);
                    digitalWrite(randomNumber3, HIGH);
                    digitalWrite(randomNumber4, HIGH);
                    digitalWrite(randomNumber5, HIGH);
                    delay(5);
                  }
                }
                }
              }
              }        
        }
        else if (nappiOnPainettu6())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        randomPosition = random(0, 9);
        randomNumber2 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        if (randomNumber2 != 99){ 
          randomPosition = random(0, 9);
          randomNumber3 = arrayOfLEDS[randomPosition];
          arrayOfLEDS[randomPosition] = 99;
            if (randomNumber3 != 99) {
              randomPosition = random(0, 9);
              randomNumber4 = arrayOfLEDS[randomPosition];
              arrayOfLEDS[randomPosition] = 99;
                if (randomNumber4 != 99) {
                   tmrpcm.play("06.wav");
                  digitalWrite(led1, HIGH);         // Sytytetään ensin kaikki ledit.
                  digitalWrite(led2, HIGH);
                  digitalWrite(led3, HIGH);
                  digitalWrite(led4, HIGH);
                  digitalWrite(led5, HIGH);
                  digitalWrite(led6, HIGH);
                  digitalWrite(led7, HIGH);
                  digitalWrite(led8, HIGH);
                  digitalWrite(led9, HIGH);
                  digitalWrite(led10, HIGH);
                    while (nappiOnPainettu6()) 
                      { 
                      digitalWrite(randomNumber1, LOW);  // Sammutetaan 4 arvottua lediä.
                      digitalWrite(randomNumber2, LOW);
                      digitalWrite(randomNumber3, LOW);
                      digitalWrite(randomNumber4, LOW);
                      delay(5);
                      }
                    }
                  }
                }
        }
        else if (nappiOnPainettu7())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        randomPosition = random(0, 9);
        randomNumber2 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        if (randomNumber2 != 99){
          randomPosition = random(0, 9);
          randomNumber3 = arrayOfLEDS[randomPosition];
          arrayOfLEDS[randomPosition] = 99;
          if (randomNumber3 != 99) {
            tmrpcm.play("07.wav");
            digitalWrite(led1, HIGH);
            digitalWrite(led2, HIGH);
            digitalWrite(led3, HIGH);
            digitalWrite(led4, HIGH);
            digitalWrite(led5, HIGH);
            digitalWrite(led6, HIGH);
            digitalWrite(led7, HIGH);
            digitalWrite(led8, HIGH);
            digitalWrite(led9, HIGH);
            digitalWrite(led10, HIGH);
            while (nappiOnPainettu7()) 
            {           
            digitalWrite(randomNumber1, LOW);
            digitalWrite(randomNumber2, LOW);
            digitalWrite(randomNumber3, LOW);
            delay(5);
            }
          }
          }
        } 
        else if (nappiOnPainettu8())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        randomPosition = random(0, 9);
        randomNumber2 = arrayOfLEDS[randomPosition];
        arrayOfLEDS[randomPosition] = 99;
        if (randomNumber2 != 99){
          tmrpcm.play("08.wav");
          digitalWrite(led1, HIGH);
          digitalWrite(led2, HIGH);
          digitalWrite(led3, HIGH);
          digitalWrite(led4, HIGH);
          digitalWrite(led5, HIGH);
          digitalWrite(led6, HIGH);
          digitalWrite(led7, HIGH);
          digitalWrite(led8, HIGH);
          digitalWrite(led9, HIGH);
          digitalWrite(led10, HIGH); 
            while (nappiOnPainettu8()) 
            {
            digitalWrite(randomNumber1, LOW);
            digitalWrite(randomNumber2, LOW);
            delay(5);
            }
          }
        }
        
        else if (nappiOnPainettu9())
        {
        randomPosition = random(0, 9);
        randomNumber1 = arrayOfLEDS[randomPosition];
        tmrpcm.play("09.wav");
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
        digitalWrite(led3, HIGH);
        digitalWrite(led4, HIGH);
        digitalWrite(led5, HIGH);
        digitalWrite(led6, HIGH);
        digitalWrite(led7, HIGH);
        digitalWrite(led8, HIGH);
        digitalWrite(led9, HIGH);
        digitalWrite(led10, HIGH);
        while (nappiOnPainettu9()) 
        {  
          digitalWrite(randomNumber1, LOW);
          delay(5);
          }
        }
      else if (nappiOnPainettu10())
        { tmrpcm.play("10.wav");
          while (nappiOnPainettu10())
            {
            digitalWrite(led1, HIGH);    // Sytytetään kaikki 10 lediä.
            digitalWrite(led2, HIGH);
            digitalWrite(led3, HIGH);
            digitalWrite(led4, HIGH);
            digitalWrite(led5, HIGH);
            digitalWrite(led6, HIGH);
            digitalWrite(led7, HIGH);
            digitalWrite(led8, HIGH);
            digitalWrite(led9, HIGH);
            digitalWrite(led10, HIGH);
            
            delay(5);
            }
        }
        else {
          digitalWrite(led1, LOW);        // Kun mitään nappia ei ole painettu, sammutetaan kaikki ledit.
          digitalWrite(led2, LOW);
          digitalWrite(led3, LOW);
          digitalWrite(led4, LOW);
          digitalWrite(led5, LOW);
          digitalWrite(led6, LOW);
          digitalWrite(led7, LOW);
          digitalWrite(led8, LOW);
          digitalWrite(led9, LOW);
          digitalWrite(led10, LOW);
          } // 1. if-else-lause sulkeet kiinni
}


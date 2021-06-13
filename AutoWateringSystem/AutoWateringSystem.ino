#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <RTClib.h>

#pragma region V�ltoz�k �s szenzorok

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD objektum l�trehoz�sa, az LCD c�m�nek, a karaktereinek �s a sorainak megad�sa
RTC_DS3231 rtc; // L�trehozzuk az RTC objektumot
dht DHT; // DHT-11 objektum l�trehoz�sa

#define dataPin 22 // DHT11 PIN
#define echoPin 52 // HC-SR04 ECHOPIN
#define trigPin 53 //HC-SR04 TRIGPIN

const int Relay_Pin = A0; //Rel� adatpin (Analog 0)
long duration; // a hanghull�m terjed�si idej�nek a v�ltoz�ja
int distance; // v�ltoz� a t�vols�gm�r�shez (v�zszint)
int temp; // v�ltoz� a h�m�rs�kletnek
int hum; // v�ltoz� a pratartalomnak
int second; // v�ltoz� az RTC m�sodperc�nek
int hour; // v�ltoz� az RTC �r�j�nak
int minute; // v�ltoz� az RTC perc�nek
#pragma endregion

void setup()
{
	pinMode(trigPin, OUTPUT); // HC-SR04 TRIGPIN megad�sa kimenetk�nt (53-as digital pin)
	pinMode(echoPin, INPUT); // HC-SR04 ECHOPIN megad�sa bemenetk�nt (52-es digital pin)
	pinMode(Relay_Pin, OUTPUT); //Rel� PIN megad�sa kimenetk�nt (22-es digitalpin)
	Serial.begin(9600); // initialize serial
	lcd.init(); // LCD bekapcsol�sa
	lcd.backlight(); // LCD h�tt�rvil�g�t�s
	rtc.begin(); // RTC bekapcsol�sa
	delay(500); // f�l m�sodperc v�rakoz�s
}

void loop()
{
	LCDTime(); // id� megjelen�t�se az LCD-n
	TemperatureAndHumidity(); //h�m�rs�klet �s p�ratartalaom megjelen�t�se az LCD-n
	WaterLevel(); //v�zszint megjelen�t�se az LCD-n
	Pump(); //V�zpumpa

	if (Serial.available())
	{
		char input = Serial.read();
		if (input == 'u') UpdateRTC(); //RTC friss�t�se a soros monitoron kereszt�l az 'u' megnyom�sa ut�na

	}
	//teszt(); //pumpa �s t�ml� tesztel�se
}

#pragma region Met�dusok
void TemperatureAndHumidity() //H�m�rs�kelt �s p�ratartalom megjelen�t�se
{
	int readData = DHT.read11(dataPin); //A szenzor adatainak a beolvas�sa
	temp = DHT.temperature; //H�m�rs�klet �rt�k�nek a kiolvas�sa
	hum = DHT.humidity; //P�ratartalom �rt�k�nek a kiolvas�sa

	//A kiolvasott �rt�kek megjelen�t�se az LCD-n
	lcd.setCursor(4, 0); // A kurzor be�ll�t�sa
	lcd.print("Temp: ");
	lcd.print(temp); //H�m�rs�kelt kiirat�sa
	lcd.print((char)223); //A � jel megjelen�t�se
	lcd.setCursor(4, 1); // A kurzor be�ll�t�sa
	lcd.print("Hum: ");
	lcd.print(hum); // A p�ratartalom kiirat�sa 
	lcd.print(" %");

	DelayAndClear(2000);
}
void UpdateRTC() //RTC frissit�se a soros monitoron kereszt�l
{
	lcd.clear(); // LCD t�rl�se
	lcd.setCursor(0, 0); //Kurzor be�ll�t�sa
	lcd.print("Edit Mode..."); //Kiratjuk a szerkeszt� m�dot

	const char txt[6][15] =
	{
	"year [4-digit]", "month [1~12]", "day [1~31]",
	"hours [0~23]", "minutes [0~59]", "seconds [0~59]"
	}; // A d�tum �s az id� form�tum�nak a megad�sa

	String str = ""; //Egy STRING objektum l�trehoz�sa
	long newDate[6]; //T�mb az �j d�tumnak

	while (Serial.available())
	{
		Serial.read(); // Soros puffer t�rl�se
	}

	for (int i = 0; i < 6; i++) //Bek�rj�k a soros monitoron kereszt�l az �j d�tumot �s id�t
	{

		Serial.print("Enter ");
		Serial.print(txt[i]);
		Serial.print(": ");

		while (!Serial.available()) // V�runk az adatokra
		{
			;
		}

		str = Serial.readString(); // Adatbeolvas�s
		newDate[i] = str.toInt(); // A beolvasott adatokat sz�mm� konvert�ljuk �s mentj�k

		Serial.println(newDate[i]); // A beolvasott adatok megjelen�t�se
	}

	rtc.adjust(DateTime(newDate[0], newDate[1], newDate[2], newDate[3], newDate[4], newDate[5])); //Az RTC friss�t�se a megadott adatok alapj�n
	Serial.println("RTC Updated!"); // Visszajelz�s a sikeres friss�t�sr�l
}
void LCDTime() //A d�tum �s az id� kiirat�sa az LCD-re
{
	DateTime rtcTime = rtc.now(); //DateTime objektum l�trehoz�sa

	second = rtcTime.second(); //m�sodperc
	minute = rtcTime.minute(); //perc
	hour = rtcTime.hour(); // �ra
	int dayoftheweek = rtcTime.dayOfTheWeek(); //a h�t napja
	int day = rtcTime.day(); // nap
	int month = rtcTime.month(); // h�nap
	int year = rtcTime.year(); // �v

	lcd.setCursor(0, 0); //kurzor be�ll�t�sa

	// d�tum �s az id� kiirat�sa �v-h�nap-nap �s h�t napja r�vid�tve form�tumban

	lcd.print(year);
	lcd.print(".");
	if (month < 10) lcd.print("0"); // ha a sz�m kisebb, mint 10 el� �runk egy null�t (0)
	lcd.print(month);
	lcd.print(".");
	if (day < 10) lcd.print("0"); // ha a sz�m kisebb, mint 10 el� �runk egy null�t (0)
	lcd.print(day);

	lcd.setCursor(13, 0); //kurzor be�ll�t�sa

	const char dayInWords[7][4] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" }; // a h�t napjai r�vid�tve, angol form�tumban (I2C LCD nem tud �kezetet kezelni)
	lcd.print(dayInWords[dayoftheweek]); // a h�t napjai r�vid�tve kiirat�sa

	lcd.setCursor(0, 1); //kurzor be�ll�t�sa

	// az �ra-perc-m�sodperc kiirat�sa  
	if (hour < 10) lcd.print("0"); // ha a sz�m kisebb, mint 10 el� �runk egy null�t (0)
	lcd.print(hour);
	lcd.print(":");
	if (minute < 10) lcd.print("0"); // ha a sz�m kisebb, mint 10 el� �runk egy null�t (0)
	lcd.print(minute);
	lcd.print(":");
	if (second < 10) lcd.print("0"); // ha a sz�m kisebb, mint 10 el� �runk egy null�t (0)
	lcd.print(second);
	DelayAndClear(2000); //K�sleltet�s �s a kijelz� let�rl�se
}
void Pump() //A pumpa m�k�dtet�se
{
	//m�g finom�tani kell 
	if (distance < 0 || distance>2000) digitalWrite(Relay_Pin, LOW); //Figylej�k a tart�jba l�v� v�zszintet �s csak adott �rt�k k�z�tt m�k�dtetj�k a pump�t, mert k�l�nben le�g. Az �rt�k a tart�ly m�ret�t�l f�gg�en v�ltozhat, illetve valamennyit r� kell hagyni, ha esetleg a szenzor nem m�k�dik akkor null�t ad vissza
	else
	{
		// A h�m�rs�klethez viszony�tva kapcsoljuk be a pump�t
		if (10 < temp && temp <= 20)
		{
			WateringTime(13, 0, 2, 0, 60);
			WateringTime(13, 5, 7, 0, 60);
			////Locsol� program ind�t�sa az �ra, kezd� perc, a v�g perc, a kezd� m�sodperc �s a v�g m�sodperc megad�s�val
			//WateringTime(8, 0, 10, 0, 60); //d�lel�tt
			//WateringTime(18, 0, 10, 0, 60); //d�lut�n
		}

		else if (20 < temp && temp <= 30)
		{
			WateringTime(13, 0, 2, 0, 60);
			WateringTime(13, 5, 7, 0, 60);
			////Locsol� program ind�t�sa az �ra, kezd� perc, a v�g perc, a kezd� m�sodperc �s a v�g m�sodperc megad�s�val
			////d�lel�tt
			//WateringTime(8, 0, 10, 0, 60);
			//WateringTime(10, 0, 10, 0, 60);
			////d�lut�n
			//WateringTime(16, 0, 10, 0, 60);
			//WateringTime(18, 0, 10, 0, 60);
		}

		else if (30 < temp && temp <= 50)
		{
			WateringTime(13, 0, 2, 0, 60);
			WateringTime(13, 5, 7, 0, 60);
			////Locsol� program ind�t�sa az �ra, kezd� perc, a v�g perc, a kezd� m�sodperc �s a v�g m�sodperc megad�s�val
			////d�lel�tt
			//WateringTime(7, 0, 10, 0, 60);
			//WateringTime(8, 0, 10, 0, 60);
			//WateringTime(9, 0, 10, 0, 60);
			//WateringTime(10, 0, 10, 0, 60);
			////d�lut�n
			//WateringTime(17, 0, 10, 0, 60);
			//WateringTime(18, 0, 10, 0, 60);
			//WateringTime(19, 0, 10, 0, 60);
			//WateringTime(20, 0, 10, 0, 60);
		}
	}
}
void WaterLevel() //A v�zszint figyel�se
{
	digitalWrite(trigPin, LOW); // A TRIGPIN-t kikapcsoljuk
	delayMicroseconds(10); //10 microsecundum v�rakoz�s
	digitalWrite(trigPin, HIGH); // 10 microsecundumra bekapcsoljuk a TRIGPIN-t
	delayMicroseconds(10); //10 microsecundum v�rakoz�s
	digitalWrite(trigPin, LOW); // A TRIGPIN-t kikapcsoljuk

	duration = pulseIn(echoPin, HIGH); // Az ECHOPIN seg�ts�g�vel megadjuk a hanghull�m terjed�si idej�t mikroszekundumban kifejezve
	distance = duration * 0.034 / 2; // A t�vols�g kisz�m�t�sa (terjed�si sebess�g osztva kett�vel (oda �s vissza)

	// A v�zszint megjeln�t�se az LCD-n
	lcd.setCursor(2, 0); //A kurzor be�ll�t�sa
	lcd.print("Water level");
	lcd.setCursor(4, 1); //A kurzor be�ll�t�sa
	lcd.print("-");
	lcd.print(distance); //A v�zszint kiirat�sa
	lcd.print(" cm");
	DelayAndClear(2000); //K�sleltet�s �s a kijelz� let�rl�se
}
void DelayAndClear(int ms) //K�sleltet�s �s az LCD t�rl�se megadott milisecundom �rt�kkel
{
	delay(ms);
	lcd.clear();
}
void WateringTime(int hour_when_watering_start, int minute_when_watering_start, int minute_when_watering_end, int second_when_watering_start, int second_when_watering_end) //Az �nt�z�si program bekapcsol�sa �ra, kezd� �s befejez� perc �s kezd� �s befejez� m�sodperc megad�s�val
{
	if (hour == hour_when_watering_start &&
		minute >= minute_when_watering_start && minute <= minute_when_watering_end &&
		second_when_watering_start < second && second < second_when_watering_end) //Ha az �rt�kek megfelel�ek bekapcsoljuk a pump�t adott id�tartamra
	{
		digitalWrite(Relay_Pin, HIGH);
	}
	else digitalWrite(Relay_Pin, LOW); //A pumpa kikapcsol�sa
}
#pragma endregion

void teszt() // a pumpa �s a t�ml� tesztel�se, hogy m�k�dik-e
{
	digitalWrite(Relay_Pin, HIGH);
}
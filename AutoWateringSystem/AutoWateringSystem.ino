#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <RTClib.h>

#pragma region Változók és szenzorok

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD objektum létrehozása, az LCD címének, a karaktereinek és a sorainak megadása
RTC_DS3231 rtc; // Létrehozzuk az RTC objektumot
dht DHT; // DHT-11 objektum létrehozása

#define dataPin 22 // DHT11 PIN
#define echoPin 52 // HC-SR04 ECHOPIN
#define trigPin 53 //HC-SR04 TRIGPIN

const int Relay_Pin = A0; //Relé adatpin (Analog 0)
long duration; // a hanghullám terjedési idejének a változója
int distance; // változó a távolságméréshez (vízszint)
int temp; // változó a hõmérsékletnek
int hum; // változó a pratartalomnak
int second; // változó az RTC másodpercének
int hour; // változó az RTC órájának
int minute; // változó az RTC percének
#pragma endregion

void setup()
{
	pinMode(trigPin, OUTPUT); // HC-SR04 TRIGPIN megadása kimenetként (53-as digital pin)
	pinMode(echoPin, INPUT); // HC-SR04 ECHOPIN megadása bemenetként (52-es digital pin)
	pinMode(Relay_Pin, OUTPUT); //Relé PIN megadása kimenetként (22-es digitalpin)
	Serial.begin(9600); // initialize serial
	lcd.init(); // LCD bekapcsolása
	lcd.backlight(); // LCD háttérvilágítás
	rtc.begin(); // RTC bekapcsolása
	delay(500); // fél másodperc várakozás
}

void loop()
{
	LCDTime(); // idõ megjelenítése az LCD-n
	TemperatureAndHumidity(); //hõmérséklet és páratartalaom megjelenítése az LCD-n
	WaterLevel(); //vízszint megjelenítése az LCD-n
	Pump(); //Vízpumpa

	if (Serial.available())
	{
		char input = Serial.read();
		if (input == 'u') UpdateRTC(); //RTC frissétése a soros monitoron keresztül az 'u' megnyomása utána

	}
	//teszt(); //pumpa és tömlõ tesztelése
}

#pragma region Metódusok
void TemperatureAndHumidity() //Hõmérsékelt és páratartalom megjelenítése
{
	int readData = DHT.read11(dataPin); //A szenzor adatainak a beolvasása
	temp = DHT.temperature; //Hõmérséklet értékének a kiolvasása
	hum = DHT.humidity; //Páratartalom értékének a kiolvasása

	//A kiolvasott értékek megjelenítése az LCD-n
	lcd.setCursor(4, 0); // A kurzor beállítása
	lcd.print("Temp: ");
	lcd.print(temp); //Hõmérsékelt kiiratása
	lcd.print((char)223); //A ° jel megjelenítése
	lcd.setCursor(4, 1); // A kurzor beállítása
	lcd.print("Hum: ");
	lcd.print(hum); // A páratartalom kiiratása 
	lcd.print(" %");

	DelayAndClear(2000);
}
void UpdateRTC() //RTC frissitése a soros monitoron keresztül
{
	lcd.clear(); // LCD törlése
	lcd.setCursor(0, 0); //Kurzor beállítása
	lcd.print("Edit Mode..."); //Kiratjuk a szerkesztõ módot

	const char txt[6][15] =
	{
	"year [4-digit]", "month [1~12]", "day [1~31]",
	"hours [0~23]", "minutes [0~59]", "seconds [0~59]"
	}; // A dátum és az idõ formátumának a megadása

	String str = ""; //Egy STRING objektum létrehozása
	long newDate[6]; //Tömb az új dátumnak

	while (Serial.available())
	{
		Serial.read(); // Soros puffer törlése
	}

	for (int i = 0; i < 6; i++) //Bekérjük a soros monitoron keresztül az új dátumot és idõt
	{

		Serial.print("Enter ");
		Serial.print(txt[i]);
		Serial.print(": ");

		while (!Serial.available()) // Várunk az adatokra
		{
			;
		}

		str = Serial.readString(); // Adatbeolvasás
		newDate[i] = str.toInt(); // A beolvasott adatokat számmá konvertáljuk és mentjük

		Serial.println(newDate[i]); // A beolvasott adatok megjelenítése
	}

	rtc.adjust(DateTime(newDate[0], newDate[1], newDate[2], newDate[3], newDate[4], newDate[5])); //Az RTC frissítése a megadott adatok alapján
	Serial.println("RTC Updated!"); // Visszajelzés a sikeres frissítésrõl
}
void LCDTime() //A dátum és az idõ kiiratása az LCD-re
{
	DateTime rtcTime = rtc.now(); //DateTime objektum létrehozása

	second = rtcTime.second(); //másodperc
	minute = rtcTime.minute(); //perc
	hour = rtcTime.hour(); // óra
	int dayoftheweek = rtcTime.dayOfTheWeek(); //a hét napja
	int day = rtcTime.day(); // nap
	int month = rtcTime.month(); // hónap
	int year = rtcTime.year(); // év

	lcd.setCursor(0, 0); //kurzor beállítása

	// dátum és az idõ kiiratása év-hónap-nap és hét napja rövidítve formátumban

	lcd.print(year);
	lcd.print(".");
	if (month < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
	lcd.print(month);
	lcd.print(".");
	if (day < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
	lcd.print(day);

	lcd.setCursor(13, 0); //kurzor beállítása

	const char dayInWords[7][4] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" }; // a hét napjai rövidítve, angol formátumban (I2C LCD nem tud ékezetet kezelni)
	lcd.print(dayInWords[dayoftheweek]); // a hét napjai rövidítve kiiratása

	lcd.setCursor(0, 1); //kurzor beállítása

	// az óra-perc-másodperc kiiratása  
	if (hour < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
	lcd.print(hour);
	lcd.print(":");
	if (minute < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
	lcd.print(minute);
	lcd.print(":");
	if (second < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
	lcd.print(second);
	DelayAndClear(2000); //Késleltetés és a kijelzõ letörlése
}
void Pump() //A pumpa mûködtetése
{
	//még finomítani kell 
	if (distance < 0 || distance>2000) digitalWrite(Relay_Pin, LOW); //Figylejük a tartájba lévõ vízszintet és csak adott érték között mûködtetjük a pumpát, mert különben leég. Az érték a tartály méretétõl függõen változhat, illetve valamennyit rá kell hagyni, ha esetleg a szenzor nem mûködik akkor nullát ad vissza
	else
	{
		// A hõmérséklethez viszonyítva kapcsoljuk be a pumpát
		if (10 < temp && temp <= 20)
		{
			WateringTime(13, 0, 2, 0, 60);
			WateringTime(13, 5, 7, 0, 60);
			////Locsoló program indítása az óra, kezdõ perc, a vég perc, a kezdõ másodperc és a vég másodperc megadásával
			//WateringTime(8, 0, 10, 0, 60); //délelõtt
			//WateringTime(18, 0, 10, 0, 60); //délután
		}

		else if (20 < temp && temp <= 30)
		{
			WateringTime(13, 0, 2, 0, 60);
			WateringTime(13, 5, 7, 0, 60);
			////Locsoló program indítása az óra, kezdõ perc, a vég perc, a kezdõ másodperc és a vég másodperc megadásával
			////délelõtt
			//WateringTime(8, 0, 10, 0, 60);
			//WateringTime(10, 0, 10, 0, 60);
			////délután
			//WateringTime(16, 0, 10, 0, 60);
			//WateringTime(18, 0, 10, 0, 60);
		}

		else if (30 < temp && temp <= 50)
		{
			WateringTime(13, 0, 2, 0, 60);
			WateringTime(13, 5, 7, 0, 60);
			////Locsoló program indítása az óra, kezdõ perc, a vég perc, a kezdõ másodperc és a vég másodperc megadásával
			////délelõtt
			//WateringTime(7, 0, 10, 0, 60);
			//WateringTime(8, 0, 10, 0, 60);
			//WateringTime(9, 0, 10, 0, 60);
			//WateringTime(10, 0, 10, 0, 60);
			////délután
			//WateringTime(17, 0, 10, 0, 60);
			//WateringTime(18, 0, 10, 0, 60);
			//WateringTime(19, 0, 10, 0, 60);
			//WateringTime(20, 0, 10, 0, 60);
		}
	}
}
void WaterLevel() //A vízszint figyelése
{
	digitalWrite(trigPin, LOW); // A TRIGPIN-t kikapcsoljuk
	delayMicroseconds(10); //10 microsecundum várakozás
	digitalWrite(trigPin, HIGH); // 10 microsecundumra bekapcsoljuk a TRIGPIN-t
	delayMicroseconds(10); //10 microsecundum várakozás
	digitalWrite(trigPin, LOW); // A TRIGPIN-t kikapcsoljuk

	duration = pulseIn(echoPin, HIGH); // Az ECHOPIN segítségével megadjuk a hanghullám terjedési idejét mikroszekundumban kifejezve
	distance = duration * 0.034 / 2; // A távolság kiszámítása (terjedési sebesség osztva kettõvel (oda és vissza)

	// A vízszint megjelnítése az LCD-n
	lcd.setCursor(2, 0); //A kurzor beállítása
	lcd.print("Water level");
	lcd.setCursor(4, 1); //A kurzor beállítása
	lcd.print("-");
	lcd.print(distance); //A vízszint kiiratása
	lcd.print(" cm");
	DelayAndClear(2000); //Késleltetés és a kijelzõ letörlése
}
void DelayAndClear(int ms) //Késleltetés és az LCD törlése megadott milisecundom értékkel
{
	delay(ms);
	lcd.clear();
}
void WateringTime(int hour_when_watering_start, int minute_when_watering_start, int minute_when_watering_end, int second_when_watering_start, int second_when_watering_end) //Az öntözési program bekapcsolása óra, kezdõ és befejezõ perc és kezdõ és befejezõ másodperc megadásával
{
	if (hour == hour_when_watering_start &&
		minute >= minute_when_watering_start && minute <= minute_when_watering_end &&
		second_when_watering_start < second && second < second_when_watering_end) //Ha az értékek megfelelõek bekapcsoljuk a pumpát adott idõtartamra
	{
		digitalWrite(Relay_Pin, HIGH);
	}
	else digitalWrite(Relay_Pin, LOW); //A pumpa kikapcsolása
}
#pragma endregion

void teszt() // a pumpa és a tömlõ tesztelése, hogy mûködik-e
{
	digitalWrite(Relay_Pin, HIGH);
}
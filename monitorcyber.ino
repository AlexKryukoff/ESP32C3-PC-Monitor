//версия ядра 2.0.4
#include <OneWire.h>
#include <DallasTemperature.h>
const int TEMP_PIN = 4;     // Датчик DS18B20 на GPIO4
const int FAN_PIN = 5;      // Вентилятор на GPIO5
const int PWM_FREQ = 25000;     // Частота 25 кГц (тихий режим)
const int PWM_CHANNEL = 0;      // Канал ШИМ
const int PWM_RESOLUTION = 8;   // Разрядность 8 бит (0-255)
const float TEMP_MIN = 35.0;
const float TEMP_MAX = 55.0;
const int DUTY_MIN = 180;        // Минимальный рабочий цикл (чтобы закрутился)
const int DUTY_MAX = 255;
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
float currentTemp = 0.0; // Темп переменная
int dutyCycle = 0;       // Обопроты переменная
#define TEXT			TFT_DARKGREY
#define BACK		TFT_WHITE
#define RING_RADIUS 46
#define lastActiveDelay 5000
bool PC_Connected = false; // Флаг: true - комп на связи, false - связи нет
unsigned long circleTimer = 0; // Таймер для удержания кружочка на экране
long lastActiveConn = 0;	// Время принятия пакета
float value[12];
float wp;
float up;
boolean iv = true;
const String autoRequest = "MADS";// Запрос на подключение от сервера.
const String autoResponse = "Hello MADS\n";// Ответ на попытку подключения.
const int RequestInterval = 500;// Интервал в миллисекундах между запросами данных.
String serialBuffer;      // Текущий текст, полученный через последовательный порт.
unsigned long lastRequestTime = 0;// Время последнего запроса данных.
unsigned long currentTime = 0;// Текущий буфер времени. Переполняется через 49 дней.
#include <TFT_eSPI.h> 
#include <Fonts/Custom/orbitron_bold16pt7b.h>
#include <Fonts/Custom/orbitron_bold12pt7b.h>
#include <Fonts/Custom/orbitron_bold10pt7b.h>
#include <Fonts/Custom/orbitron_bold8pt7b.h>
#include <Fonts/Custom/orbitron_bold6pt7b.h>
#include <Fonts/Custom/orbitron_bold4pt7b.h>
#include <Fonts/Custom/orbitron_bold3pt7b.h>
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite img = TFT_eSprite(&tft);   // Спрайт для колец
TFT_eSprite list = TFT_eSprite(&tft);  // Спрайт для полосок внизу
TFT_eSprite txtBuf = TFT_eSprite(&tft); // Спрайт для текста FPS и PING
TFT_eSprite graphBuf = TFT_eSprite(&tft); // Спрайт для графиков
// --- ТВОЯ ПАЛИТРА (можно менять под себя) ---
#define COLOR_MAIN_BG  tft.color565(30, 30, 33)
#define COLOR_RING_BG  tft.color565(45, 45, 48)   
#define COLOR_TEXT     TFT_WHITE
#define COLOR_FRAME    tft.color565(60, 60, 65)

// --- БОЛЬШИЕ КОЛЬЦА (С зазором 1 пиксель) ---
#define R_OUT_START 53
#define R_OUT_END   46  // Уменьшили толщину внешнего кольца до 7 пикселей

#define R_IN_START  44  // Внутреннее начинается на 44 (радиус 45 пустой — зазор ровно 1 пиксель)
#define R_IN_END    37  // Уменьшили толщину внутреннего кольца до 7 пикселей
// --- МИКРО-КОЛЬЦА (С зазором 1 пиксель) ---
#define R_MINI_OUT_START 22  
#define R_MINI_OUT_END   19  // Толщина внешнего кольца стала 4 пикселя (22, 21, 20, 19)

#define R_MINI_IN_START  17  // Внутреннее начинается на 17 (радиус 18 пустой — зазор ровно 1 пиксель)
#define R_MINI_IN_END    14  // Толщина внутреннего кольца тоже 4 пикселя (17, 16, 15, 14)
//анимация 

// Целевые значения от ПК
float ssdTemp = 30.0;
float ramUsed = 4.0;   // Сколько ГБ занято (от 0 до 16)
float psuTemp = 35.0;
float ssdTemp2 = 30.0;       // Целевое значение от ПК для второго SSD
float smoothSsdTemp2 = 30.0; // Для плавности LERP

// Переменные для плавности LERP
float smoothSsdTemp = 30.0;
float smoothRamUsed = 4.0;
float smoothPsuTemp = 35.0;

float smoothCpuTemp = 30.0;
float smoothCpuLoad = 0.0;
float smoothGpuTemp = 30.0;
float smoothGpuLoad = 0.0;
// Коэффициент плавности: от 0.01 (очень медленно) до 1.0 (мгновенно)
float k = 0.1;

//графики
#define GRAPH_WIDTH 142
#define GRAPH_HEIGHT 114

int fpsHistory[GRAPH_WIDTH] = {0};
int pingHistory[GRAPH_WIDTH] = {0};

unsigned long drawTime = 0;
#define NSTARS 1024
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};
uint8_t za, zb, zc, zx;
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za^zc^zx);
  zb = (zb+za);
  zc = ((zc+(zb>>1))^za);
  return zc;
}
void setup() {
	delay(2000);
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
  // Настройка ШИМ для ESP32
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(FAN_PIN, PWM_CHANNEL);
  sensors.begin();
  sensors.setWaitForConversion(false);
	Serial.begin(9600);		// Скорость последовательной передачи данных
	tft.setTextWrap(false); // Не переносить текст на следующую строку
	tft.init();
  tft.fillScreen(COLOR_MAIN_BG);	// Очистить экран
    // Инициализация памяти под спрайты
  img.createSprite(110, 110); 
  list.createSprite(240, 80); 
  txtBuf.createSprite(85, 114); // Создаем окошко шириной 85 и высотой 114
  graphBuf.createSprite(GRAPH_WIDTH, GRAPH_HEIGHT);

	delay(200); 
}
//---------- Главный цикл -----------------------
//---------- Главный цикл -----------------------
void loop() {
  tft.setRotation(2);

  // Флаг, который покажет, что данные от ПК пришли ХОТЯ БЫ ОДИН РАЗ с момента включения
  static bool firstDataReceived = false;

  // 1. Чтение данных из последовательного порта
  while (Serial.available() > 0) {
    serialBuffer = Serial.readStringUntil('\n');
    if (serialBuffer.equals(autoRequest)) {
      Serial.print(autoResponse);
      serialBuffer = "";  
    }
    else {
      lastActiveConn = millis(); // Время принятия пакета
      DecodeStr();
      
      // ХАК: Как только сюда попал первый рабочий пакет — даем добро на запуск колец
      firstDataReceived = true; 
      
      circleTimer = millis() + 100; 
      serialBuffer = "";  // Очистка буфера
    }
  }

  // 2. Проверяем связь (функция сама переключит флаг PC_Connected)
  activityChecker(); 

  // =========================================================================
  // 3. ИСПРАВЛЕННАЯ ОТРИСОВКА ПО ФЛАГАМ
  // =========================================================================
  // Кольца включатся ТОЛЬКО если есть связь И данные пришли хотя бы один раз
  if (PC_Connected == true && firstDataReceived == true) {
    
    drawAnimation(false); // Стираем заставку фоном (вызовется один раз при переключении)

    // --- Твои старые расчеты плавности для CPU и GPU ---
    smoothCpuTemp += (value[0] - smoothCpuTemp) * k; 
    smoothGpuTemp += (value[1] - smoothGpuTemp) * k; 
    smoothCpuLoad += (value[2] - smoothCpuLoad) * k; 
    smoothGpuLoad += (value[3] - smoothGpuLoad) * k;

    // --- НОВЫЕ расчеты плавности для мини-колец ---
    smoothSsdTemp += (value[11] - smoothSsdTemp) * k; 
    smoothSsdTemp2 += (value[9] - smoothSsdTemp2) * k;
    smoothRamUsed += (value[7] - smoothRamUsed) * k; 
    smoothPsuTemp += (currentTemp - smoothPsuTemp) * k; 
    
    // --- Отрисовка больших колец ---
    DrawDisplay(); 

    // --- Отрисовка новых мини-колец ---
    drawMiniSSD((int)(smoothSsdTemp + 0.5), (int)(smoothSsdTemp2 + 0.5));
    drawMiniRAM(smoothRamUsed); 
    drawMiniPSU((int)(smoothPsuTemp + 0.5), dutyCycle);

  } else {
    // === РЕЖИМ ЗАСТАВКИ ===
    // Сюда плата зайдет СРАЗУ при старте (так как firstDataReceived еще false),
    // а также если в процессе работы пропадет связь (PC_Connected станет false).
    drawAnimation(true); 
  }

  // 4. Твои остальные фоновые запросы
  SendDataRequest();      
  sensorsrequestTemperatures();
}
//---------- Узнаём текущее время ---------------
void SendDataRequest() {
	currentTime = millis();
	if (currentTime < lastRequestTime) lastRequestTime = 0;
	if (Serial.available() == 0 && currentTime - lastRequestTime > RequestInterval)
		lastRequestTime = currentTime;
}
//---------- Парсим принятые данные -------------
void DecodeStr() {
	String sbuf;  int ls = 0;
	for (int i = 0; i < 12; i++) {
		ls = serialBuffer.indexOf(";");
		sbuf = serialBuffer.substring(0, ls);
		serialBuffer.remove(0, ls + 1);
		if (i == 9) wp = sbuf.toFloat();
		if (i == 8) up = sbuf.toFloat();
		value[i] = floor(sbuf.toFloat());
		if (i == 10) {
			value[i] = floor(sbuf.toFloat());
			if (value[i] > 1000) value[10] = 75;
		}
		if (i < 4)  if (value[i] > 99) value[i] = 99;
	}
}
//------------ Выводим на дисплей ---------------
void DrawDisplay() {
	tft.setTextColor(TEXT, COLOR_MAIN_BG);
	//tft.drawCircle(224, 175, 6, TFT_WHITE);//для красной точки 
	//if (millis() < circleTimer) {
    //tft.fillCircle(224, 175, 5, TFT_RED);       // Горит красный, пока идет таймер
  //} else {
   // tft.fillCircle(224, 175, 5, TEXT); // Гаснет (заливается цветом фона)
  //}
  

uint16_t currentTopFrameColor = COLOR_RING_BG;
if (value[0] >= 90 || value[1] >= 85 || value[9] >= 70 || value[11] >= 80 || currentTemp >= 60) {
  currentTopFrameColor = TFT_RED;
}

// Рисуем верхнюю рамку твоей функцией, но с динамическим цветом
Round2Rect(0, 240, 0, 192, currentTopFrameColor);

uint16_t currentBottomFrameColor = COLOR_RING_BG; // Стандартный цвет по умолчанию
// =========================================================================
// ДИНАМИЧЕСКИЙ ВЫБОР ЦВЕТА ДЛЯ НИЖНЕЙ РАМКИ (FPS И ПИНГ)
// =========================================================================


int currentFps = fpsHistory[GRAPH_WIDTH - 1];
int currentPing = pingHistory[GRAPH_WIDTH - 1];

// Если FPS просел ниже 60 ИЛИ пинг улетел выше 150 — красим низ в красный
if (currentFps < 60 || currentPing > 150) {
  currentBottomFrameColor = TFT_RED;
}

// Рисуем нижнюю рамку твоей функцией
Round2Rect(0, 240, 195, 125, currentBottomFrameColor);


int finalCpuTemp = constrain((int)(smoothCpuTemp + 0.5), 0, 100);
  int finalGpuTemp = constrain((int)(smoothGpuTemp + 0.5), 0, 100);
  int finalCpuLoad = constrain((int)(smoothCpuLoad + 0.5), 0, 100);
  int finalGpuLoad = constrain((int)(smoothGpuLoad + 0.5), 0, 100);

  // ОТРИСОВКА CPU (левый экран)
  img.fillSprite(COLOR_MAIN_BG); 
  drawTempLayer(finalCpuTemp, "CPU"); 
  drawLoadLayer(finalCpuLoad);           
  img.pushSprite(9, 9);                       

  // ОТРИСОВКА GPU (правый экран)
  img.fillSprite(COLOR_MAIN_BG); 
  drawTempLayer(finalGpuTemp, "GPU"); 
  drawLoadLayer(finalGpuLoad);           
  img.pushSprite(121, 9);
  
  txtBuf.pushSprite(150, 201);

  // 1. Сначала обновляем математику графиков (передаем FPS и Пинг)
updateGraphData((int)value[10], (int)value[5]);

// ПЕРЕДАЕМ ДИНАМИЧЕСКИЙ ЦВЕТ В ФУНКЦИИ (вместо пустых скобок)
  drawBottomGraphs(currentBottomFrameColor);
  drawBottomNumbers(currentBottomFrameColor);

// 4. Выплескиваем оба готовых спрайта на физический экран!
// График встает слева (от X=5 до X=147)
graphBuf.pushSprite(5, 201);

// Цифры встают справа (от X=150 до X=235)
txtBuf.pushSprite(150, 201);
// СОЕДИНИТЕЛЬНЫЙ МОСТИК: рисуем линию в промежутке между спрайтами (от 147 до 150 по X)
tft.drawFastHLine(147, 258, 3, currentBottomFrameColor);

}

void updateGraphData(int nextFps, int nextPing) {
  // Сдвигаем все значения в массивах на 1 шаг влево
  for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
    fpsHistory[i] = fpsHistory[i + 1];
    pingHistory[i] = pingHistory[i + 1];
  }
  
  // Ограничиваем входящие значения, чтобы не было диких вылетов за рамки графиков
  if (nextFps > 1000) nextFps = 1000; // Максимум для графика FPS (например, 240 кадров)
  if (nextPing > 3000) nextPing = 3000; // Максимум для графика Пинга (например, 150 ms)

  // Записываем новые значения в самый конец массива
  fpsHistory[GRAPH_WIDTH - 1] = nextFps;
  pingHistory[GRAPH_WIDTH - 1] = nextPing;
}

// Вспомогательная функция для плавного смешивания двух цветов (градиент)
uint16_t getGradientColor(uint16_t color1, uint16_t color2, float factor) {
  if (factor <= 0.0) return color1;
  if (factor >= 1.0) return color2;
  
  // Раскладываем первый цвет на R, G, B
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;
  
  // Раскладываем второй цвет на R, G, B
  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;
  
  // Смешиваем компоненты на основе коэффициента factor (от 0.0 до 1.0)
  uint8_t rOut = r1 + (r2 - r1) * factor;
  uint8_t gOut = g1 + (g2 - g1) * factor;
  uint8_t bOut = b1 + (b2 - b1) * factor;
  
  return (rOut << 11) | (gOut << 5) | bOut;
}

void drawBottomGraphs(uint16_t lineColor) {
  // 1. Очищаем графический спрайт фоном
  graphBuf.fillSprite(COLOR_MAIN_BG);
  
  // Рисуем сквозную тонкую линию твоим цветом
  graphBuf.drawFastHLine(0, 57, GRAPH_WIDTH, lineColor);

  // Настройки для мелкого шрифта шкал
  graphBuf.setTextFont(1);
  graphBuf.setTextSize(1);
  graphBuf.setTextColor(TFT_DARKGREY);
  graphBuf.setTextDatum(TL_DATUM);

  int startX = 22; // Отступ под цифры шкал слева

  // =========================================================================
  // РАСЧЕТ МАСШТАБА И ГРАДИЕНТА ДЛЯ FPS
  // =========================================================================
  int maxFps = 0;
  int minFps = 999;
  
  for (int i = startX; i < GRAPH_WIDTH; i++) {
    if (fpsHistory[i] > maxFps) maxFps = fpsHistory[i];
    if (fpsHistory[i] < minFps) minFps = fpsHistory[i];
  }
  
  if ((maxFps - minFps) < 10) {
    maxFps += 5;
    minFps = (minFps > 5) ? minFps - 5 : 0;
  }

  // Выводим границы FPS
  graphBuf.drawString(String(maxFps), 2, 5);   
  graphBuf.drawString(String(minFps), 2, 45);  

  // ВЫЧИСЛЕНИЕ ПЛАВНОГО ЦВЕТА FPS (Зелёный вместо Белого)
  int latestFps = fpsHistory[GRAPH_WIDTH - 1];
  uint16_t currentFpsColor;

  if (latestFps < 60) {
    // От 0 до 60 кадров: плавный переход от КРАСНОГО к ЖЁЛТОМУ
    float factor = (float)latestFps / 60.0;
    currentFpsColor = getGradientColor(TFT_RED, TFT_YELLOW, factor);
  } else if (latestFps < 140) {
    // От 60 до 140 кадров: плавный переход от ЖЁЛТОГО к ЗЕЛЁНОМУ
    float factor = (float)(latestFps - 60) / (140.0 - 60.0);
    currentFpsColor = getGradientColor(TFT_YELLOW, TFT_GREEN, factor);
  } else {
    currentFpsColor = TFT_GREEN; // Всё, что выше 140 FPS — сочный зелёный
  }


  // =========================================================================
  // РАСЧЕТ МАСШТАБА И ГРАДИЕНТА ДЛЯ PING
  // =========================================================================
  int maxPing = 0;
  int minPing = 999;
  
  for (int i = startX; i < GRAPH_WIDTH; i++) {
    if (pingHistory[i] > maxPing) maxPing = pingHistory[i];
    if (pingHistory[i] < minPing) minPing = pingHistory[i];
  }
  
  if ((maxPing - minPing) < 10) {
    maxPing += 5;
    minPing = (minPing > 5) ? minPing - 5 : 0;
  }

  // Выводим границы Пинга
  graphBuf.drawString(String(maxPing), 2, 65);  
  graphBuf.drawString(String(minPing), 2, 104); 

  // ВЫЧИСЛЕНИЕ ПЛАВНОГО ЦВЕТА ПИНГА (Зелёный вместо Белого)
  int latestPing = pingHistory[GRAPH_WIDTH - 1];
  uint16_t currentPingColor;

  if (latestPing < 40) {
    currentPingColor = TFT_GREEN; // Идеальный пинг (до 40 мс) — чистый зелёный
  } else if (latestPing < 100) {
    // От 40 до 100 мс: плавный переход от ЗЕЛЁНОГО к ЖЁЛТОМУ
    float factor = (float)(latestPing - 40) / (100.0 - 40.0);
    currentPingColor = getGradientColor(TFT_GREEN, TFT_YELLOW, factor);
  } else if (latestPing < 180) {
    // От 100 до 180 мс: плавный переход от ЖЁЛТОГО к КРАСНОМУ
    float factor = (float)(latestPing - 100) / (180.0 - 100.0);
    currentPingColor = getGradientColor(TFT_YELLOW, TFT_RED, factor);
  } else {
    currentPingColor = TFT_RED; // Пинг выше 180 мс — критический красный
  }


  // =========================================================================
  // ОТРИСОВКА СУПЕР-СГЛАЖЕННЫХ ГРАФИКОВ С ОБНОВЛЕННЫМ ГРАДИЕНТОМ
  // =========================================================================
  for (int i = startX + 2; i < GRAPH_WIDTH - 2; i++) {
    
    // --- ГЛУБОКОЕ СГЛАЖИВАНИЕ FPS ---
    int smoothFpsPrev = (fpsHistory[i-3] + fpsHistory[i-2] + fpsHistory[i-1] + fpsHistory[i] + fpsHistory[i+1]) / 5;
    int smoothFpsCurr = (fpsHistory[i-2] + fpsHistory[i-1] + fpsHistory[i] + fpsHistory[i+1] + fpsHistory[i+2]) / 5;

    int yFpsPrev = map(smoothFpsPrev, minFps, maxFps, 52, 5);
    int yFpsCurr = map(smoothFpsCurr, minFps, maxFps, 52, 5);
    
    graphBuf.drawLine(i - 1, yFpsPrev,     i, yFpsCurr,     currentFpsColor); 
    graphBuf.drawLine(i - 1, yFpsPrev - 1, i, yFpsCurr - 1, currentFpsColor); 
    graphBuf.drawLine(i - 1, yFpsPrev + 1, i, yFpsCurr + 1, currentFpsColor); 
    graphBuf.drawLine(i - 2, yFpsPrev,     i - 1, yFpsCurr, currentFpsColor); 


    // --- ГЛУБОКОЕ СГЛАЖИВАНИЕ PING ---
    int smoothPingPrev = (pingHistory[i-3] + pingHistory[i-2] + pingHistory[i-1] + pingHistory[i] + pingHistory[i+1]) / 5;
    int smoothPingCurr = (pingHistory[i-2] + pingHistory[i-1] + pingHistory[i] + pingHistory[i+1] + pingHistory[i+2]) / 5;

    int yPingPrev = map(smoothPingPrev, minPing, maxPing, 110, 65);
    int yPingCurr = map(smoothPingCurr, minPing, maxPing, 110, 65);
    
    graphBuf.drawLine(i - 1, yPingPrev,     i, yPingCurr,     currentPingColor); 
    graphBuf.drawLine(i - 1, yPingPrev - 1, i, yPingCurr - 1, currentPingColor); 
    graphBuf.drawLine(i - 1, yPingPrev + 1, i, yPingCurr + 1, currentPingColor); 
    graphBuf.drawLine(i - 2, yPingPrev,     i - 1, yPingCurr, currentPingColor); 
  }
}
void drawBottomNumbers(uint16_t lineColor) {
  // 1. Очищаем весь спрайт цветом фона
  txtBuf.fillSprite(COLOR_MAIN_BG); 
  
  // Рисуем сквозную тонкую линию в правом спрайте
  txtBuf.drawFastHLine(0, 57, 85, lineColor);
  // Центр нашего текстового окошка по горизонтали
  int centerX = 42; 

  // --- ВЕРХНИЙ ЭТАЖ: FPS ---
  txtBuf.setFreeFont(&orbitron_bold16pt7b); // Цифры оставляем КРУПНЫМИ, как в кольцах
  txtBuf.setTextColor(TFT_WHITE);
  txtBuf.setTextDatum(BC_DATUM);            // Выравнивание по центру-низу
  txtBuf.drawString(String((int)value[10]), centerX, 54); // Крупные цифры FPS

  txtBuf.setFreeFont(&orbitron_bold6pt7b);  // Твой шрифт для надписей (вместо 3pt)
  txtBuf.setTextColor(TFT_DARKGREY);
  txtBuf.setTextDatum(TC_DATUM);            // Выравнивание по центру-топу
  txtBuf.drawString("FPS", centerX, 2);    // Надпись ровно по центру над цифрами FPS

  // --- НИЖНИЙ ЭТАЖ: PING ---
  txtBuf.setFreeFont(&orbitron_bold16pt7b); // Цифры оставляем КРУПНЫМИ
  txtBuf.setTextColor(TFT_WHITE);
  txtBuf.setTextDatum(BC_DATUM);

  // =========================================================================
  // ПРОВЕРКА НА БЕСКОНЕЧНОСТЬ
  // =========================================================================
  String pingText;
  // Если пинг улетает в космос или уходит в минус (признак ошибки связи на ПК)
  if (value[5] > 4000.0 || value[5] < 0.0) {
    pingText = "!"; // Выводим две маленькие "о", на шрифте Orbitron это выглядит как "∞"
  } else {
    pingText = String((int)value[5]); // Иначе выводим реальный пинг как обычно
  }

  // Рисуем полученный текст (цифры или бесконечность)
  txtBuf.drawString(pingText, centerX, 112); 
  // =========================================================================

  txtBuf.setFreeFont(&orbitron_bold6pt7b); // Твой шрифт для надписей
  txtBuf.setTextColor(TFT_DARKGREY);
  txtBuf.setTextDatum(TC_DATUM);
  txtBuf.drawString("ms", centerX, 60);     // Надпись ровно по центру над цифрами Пинга
}

void drawSplitArc(int r_out, int r_in, int limitAngle, uint16_t color, uint16_t bg_color) {
  limitAngle = constrain(limitAngle, 0, 360);

  // 1. ОЧИСТКА ФОНА (Две идеальные половинки по 180 градусов)
  // Правая сторона: старт на 12 часах (180°) и финиш на 6 часах (360°)
  img.drawArc(55, 55, r_out, r_in, 180, 360, bg_color, COLOR_MAIN_BG, true); 
  // Левая сторона: старт на 6 часах (0°) и финиш на 12 часах (180°)
  img.drawArc(55, 55, r_out, r_in, 0, 180, bg_color, COLOR_MAIN_BG, true);   

  // 2. ОТРИСОВКА АКТИВНОГО ЦВЕТА
  if (limitAngle <= 0) return; // Если значение 0, цветной сектор даже не пытаемся рисовать

  if (limitAngle <= 180) {
    // Если угол меньше половины круга, заполняем только правую сторону
    img.drawArc(55, 55, r_out, r_in, 180, 180 + limitAngle, color, COLOR_MAIN_BG, true);
  } else {
    // Если больше половины: правую сторону заливаем целиком (от 180 до 360)
    img.drawArc(55, 55, r_out, r_in, 180, 360, color, COLOR_MAIN_BG, true);
    // А на левой стороне дорисовываем только хвостик (limitAngle минус уже нарисованные 180)
    img.drawArc(55, 55, r_out, r_in, 0, limitAngle - 180, color, COLOR_MAIN_BG, true);
  }
}
void drawMiniSplitArc(TFT_eSprite &sprite, int limitAngle, uint16_t color, uint16_t bg_color, int r_start, int r_end) {
  limitAngle = constrain(limitAngle, 0, 360);

  // Очистка фона по указанным радиусам (центр 23, 23)
  sprite.drawArc(23, 23, r_start, r_end, 180, 360, bg_color, COLOR_MAIN_BG, true); 
  sprite.drawArc(23, 23, r_start, r_end, 0, 180, bg_color, COLOR_MAIN_BG, true);   

  if (limitAngle <= 0) return; 

  if (limitAngle <= 180) {
    sprite.drawArc(23, 23, r_start, r_end, 180, 180 + limitAngle, color, COLOR_MAIN_BG, true);
  } else {
    sprite.drawArc(23, 23, r_start, r_end, 180, 360, color, COLOR_MAIN_BG, true);
    sprite.drawArc(23, 23, r_start, r_end, 0, limitAngle - 180, color, COLOR_MAIN_BG, true);
  }
}
// Новый четырехцветный градиент: Синий -> Зеленый -> Желтый -> Красный
uint16_t getTemperatureGradient(float percentage) {
  percentage = constrain(percentage, 0.0, 100.0);
  uint8_t r = 0, g = 0, b = 0;

  if (percentage < 33.33) {
    // Этап 1: От Синего (0, 0, 255) к Зеленому (0, 255, 0)
    float ratio = percentage / 33.33;
    r = 0;
    g = (uint8_t)(ratio * 255.0);
    b = (uint8_t)((1.0 - ratio) * 255.0);
  } 
  else if (percentage < 66.66) {
    // Этап 2: От Зеленого (0, 255, 0) к Желтому (255, 255, 0)
    float ratio = (percentage - 33.33) / 33.33;
    r = (uint8_t)(ratio * 255.0);
    g = 255;
    b = 0;
  } 
  else {
    // Этап 3: От Желтого (255, 255, 0) к Красному (255, 0, 0)
    float ratio = (percentage - 66.66) / 33.34;
    r = 255;
    g = (uint8_t)((1.0 - ratio) * 255.0);
    b = 0;
  }

  // Конвертируем стандартный RGB в формат RGB565 (16-бит) для дисплея
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void drawTempLayer(int tempValue, String label) {
  // 1. Рассчитываем ЧЕСТНЫЙ ПРОЦЕНТ заполнения шкалы (от 0.0 до 100.0)
  // Для диапазона 25-95 градусов
  float percentage = (float)(constrain(tempValue, 25, 95) - 25) / (95 - 25) * 100.0;

  // 2. Переводим этот процент в угол от 0 до 360 градусов
  int limitAngle = map((int)percentage, 0, 100, 0, 360);
  
  // 3. Передаем в градиент именно ПРОЦЕНТ, а не сырую температуру!
  uint16_t arcColor = getTemperatureGradient(percentage);

  // 4. Отрисовка кольца
  drawSplitArc(R_OUT_START, R_OUT_END, limitAngle, arcColor, COLOR_RING_BG);

  // --- Дальше идет твой стандартный вывод текста (не меняется) ---
  img.setTextDatum(MC_DATUM);
  img.setTextColor(TFT_WHITE);
  img.setFreeFont(&orbitron_bold16pt7b);
  img.drawString(String(tempValue), 53, 50); 

  int textWidth = img.textWidth(String(tempValue)); 
  int xPos = 53 + (textWidth / 2) + 2;        
  int yPos = 50 - 13;                         
  img.drawCircle(xPos, yPos, 2, TFT_LIGHTGREY);    
  
  img.setFreeFont(&orbitron_bold4pt7b);
  img.setTextColor(TFT_DARKGREY);
  img.drawString(label, 55, 30);
}
// --- МИКРО-СЛOЙ SSD (Левый) ---
void drawMiniSSD(int tempValue1, int tempValue2) {
  TFT_eSprite mini = TFT_eSprite(&tft);
  mini.createSprite(46, 46); 
  mini.fillSprite(COLOR_MAIN_BG);

  // --- СЛОЙ 1: Внутреннее кольцо (SSD 1) ---
  // Калибруем диапазон: теперь от 25 до 75 градусов
  float percent1 = (float)(constrain(tempValue1, 35, 85) - 35) / (85 - 35) * 100.0;
  int angle1 = map((int)percent1, 0, 100, 0, 360);
  uint16_t color1 = getTemperatureGradient(percent1); // Градиент тоже станет точнее
  drawMiniSplitArc(mini, angle1, color1, COLOR_RING_BG, R_MINI_IN_START, R_MINI_IN_END);

  // --- СЛОЙ 2: Внешнее кольцо (SSD 2) ---
  // Точно так же калибруем диапазон для второго диска
  float percent2 = (float)(constrain(tempValue2, 25, 75) - 25) / (75 - 25) * 100.0;
  int angle2 = map((int)percent2, 0, 100, 0, 360);
  uint16_t color2 = getTemperatureGradient(percent2);
  drawMiniSplitArc(mini, angle2, color2, COLOR_RING_BG, R_MINI_OUT_START, R_MINI_OUT_END);

  // --- ТЕКСТ (Центровка и вывод температуры первого SSD) ---
  mini.setTextDatum(MC_DATUM);
  mini.setTextColor(TFT_WHITE);
  mini.setFreeFont(&orbitron_bold6pt7b);
  
  int textX = 21;
  int textY = 22;
  mini.drawString(String(tempValue1), textX, textY); 

  int textWidth = mini.textWidth(String(tempValue1));
  int xPos = textX + (textWidth / 2) + 1;
  int yPos = textY - 4; 
  mini.drawCircle(xPos, yPos, 1, TFT_LIGHTGREY);

  mini.pushSprite(41, 125); // Твоя зафиксированная координата по Y
  mini.deleteSprite();

  // Подпись под кольцом
  tft.setFreeFont(&orbitron_bold3pt7b);
  tft.setTextColor(TFT_DARKGREY);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("SSD", 64, 183); // Твоя зафиксированная координата по Y
}

// --- МИКРО-СЛОЙ RAM (Средний) ---
void drawMiniRAM(float ramValue) {
  TFT_eSprite mini = TFT_eSprite(&tft);
  mini.createSprite(46, 46); // Новый размер
  mini.fillSprite(COLOR_MAIN_BG);

  float percentage = (ramValue / 16.0) * 100.0;
  int limitAngle = map((int)percentage, 0, 100, 0, 360);

  // ДОБАВИЛИ РАДИУСЫ В КОНЕЦ: R_MINI_OUT_START, R_MINI_OUT_END
  drawMiniSplitArc(mini, limitAngle, TFT_DARKGREY, COLOR_RING_BG, R_MINI_OUT_START, R_MINI_OUT_END);

  mini.setTextDatum(MC_DATUM);
  mini.setTextColor(TFT_WHITE);
  mini.setFreeFont(&orbitron_bold6pt7b);
  mini.drawString(String(ramValue, 1), 23, 22); // Координаты под центр 23, Y=22
  mini.setFreeFont(&orbitron_bold3pt7b);
  mini.setTextColor(TFT_LIGHTGREY); // или TFT_WHITE, если нужно поярче
  mini.setTextDatum(BC_DATUM);
  mini.drawString("GB", 22, 39); // Встанет аккуратно в самый низ внутренней области
  mini.pushSprite(97, 125); // Новая координата на экране
  mini.deleteSprite();

  tft.setFreeFont(&orbitron_bold3pt7b);
  tft.setTextColor(TFT_DARKGREY);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("RAM", 120, 183);
  
}

// --- СЛОЙ БЛОКА ПИТАНИЯ (Обновленный) ---
void drawMiniPSU(int tempValue, int dutyValue) {
  TFT_eSprite mini = TFT_eSprite(&tft);
  mini.createSprite(46, 46); 
  mini.fillSprite(COLOR_MAIN_BG);

  // --- СЛОЙ 1: Внешнее кольцо (Температура БП: от 30 до 80 градусов) ---
  float percentage = (float)(constrain(tempValue, 30, 80) - 30) / (80 - 30) * 100.0;
  int limitAngle = map((int)percentage, 0, 100, 0, 360);
  uint16_t arcColor = getTemperatureGradient(percentage); 
  
  // Рисуем внешнюю дугу температуры
  drawMiniSplitArc(mini, limitAngle, arcColor, COLOR_RING_BG, R_MINI_OUT_START, R_MINI_OUT_END);

  // --- СЛОЙ 2: Внутреннее кольцо (Индикатор вентилятора) ---
  uint16_t fanColor;
  if (dutyValue >= 255) {
    fanColor = TFT_RED;        // Вентилятор включился — внутреннее кольцо горит красным
  } else {
    fanColor = COLOR_RING_BG;  // Вентилятор молчит — внутреннее кольцо серое (сливается с фоном)
  }
  
  // Рисуем полный круг (360 градусов) для вентилятора
  drawMiniSplitArc(mini, 360, fanColor, COLOR_RING_BG, R_MINI_IN_START, R_MINI_IN_END);

  // --- ТЕКСТ (Центровка и вывод температуры) ---
  mini.setTextDatum(MC_DATUM);
  mini.setTextColor(TFT_WHITE);
  mini.setFreeFont(&orbitron_bold6pt7b);
  
  int textX = 21;
  int textY = 22;
  mini.drawString(String(tempValue), textX, textY); 

  int textWidth = mini.textWidth(String(tempValue));
  int xPos = textX + (textWidth / 2) + 1;
  int yPos = textY - 4;
  mini.drawCircle(xPos, yPos, 1, TFT_LIGHTGREY);

  mini.pushSprite(153, 125); // Зафиксировано
  mini.deleteSprite();

  // Подпись под кольцом
  tft.setFreeFont(&orbitron_bold3pt7b);
  tft.setTextColor(TFT_DARKGREY);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("PSU", 176, 183); // Зафиксировано
}
void drawLoadLayer(int loadValue) {
  // Переводим входящие 0-100 напрямую в угол 0-360 градусов
  int limitAngle = map(loadValue, 0, 100, 0, 360);
  
  // Рисуем внутреннее кольцо нагрузки (активная часть — темно-серая)
  drawSplitArc(R_IN_START, R_IN_END, limitAngle, TFT_DARKGREY, COLOR_RING_BG);

  // --- Вывод процентов нагрузки внизу ---
  img.setFreeFont(&orbitron_bold6pt7b);
  img.setTextColor(TFT_LIGHTGREY);
  img.drawString(String(loadValue) + "%", 55, 75);
}
// Функция создания градиента (синий -> зеленый -> красный)
uint16_t getGradient(float percent) {
  uint8_t r, g, b;
  if (percent < 50) {
    r = 0; g = map(percent, 0, 50, 0, 255); b = map(percent, 0, 50, 255, 0);
  } else {
    r = map(percent, 50, 100, 0, 255); g = map(percent, 50, 100, 255, 0); b = 0;
  }
  return tft.color565(r, g, b);
}
//-------------- Двойная рамка ------------------
void Round2Rect(int x0, int x1, int y0, int y1, int col) {
	tft.drawRoundRect(x0, y0, x1, y1, 8, col);
	tft.drawRoundRect(x0 + 1, y0 + 1, x1 - 2, y1 - 2, 7, col);
	tft.drawRoundRect(x0 + 2, y0 + 2, x1 - 4, y1 - 4, 6, col);
	tft.drawRoundRect(x0 + 3, y0 + 3, x1 - 6, y1 - 6, 5, col);
	tft.drawRoundRect(x0 + 4, y0 + 4, x1 - 8, y1 - 8, 4, col);
}

//---------- Проверка связи с ПК ---------------------------------
void activityChecker() {
  if (millis() - lastActiveConn > lastActiveDelay) {
    // ---- ПК ОТКЛЮЧЕН ----
    if (PC_Connected == true) { 
      // Этот блок выполнится ОДИН РАЗ в момент отключения, чтобы очистить экран
      tft.setRotation(2);
      tft.fillScreen(COLOR_MAIN_BG);  // Твоя очистка экрана
      // Если тут у тебя вызывается функция заставки (например, drawSaver();), поставь её сюда
    }
    PC_Connected = false; // Опускаем флаг, связь пропала
  } 
  else {
    // ---- ПК ПОДКЛЮЧЕН ----
    PC_Connected = true;  // Поднимаем флаг, связь есть!
  }
}

// Добавляем параметр active: true — крутим анимацию, false — выключаем и стираем её
void drawAnimation(bool active) {
  const int SPR_W = 216; 
  const int SPR_H = 216;   
  const int CENTER_X = 108; 
  const int CENTER_Y = 108; 
  const int BALL_R = 101;   
  const int TRAIL_LENGTH = 7; 

  const int POS_X = 12; // Координаты отрисовки на физическом экране
  const int POS_Y = 52;

  static unsigned long lastBallTime = 0;
  static float angle = 0;
  
  static int trailX[TRAIL_LENGTH];
  static int trailY[TRAIL_LENGTH];
  static bool trailInitialized = false;

  static TFT_eSprite ringBuf = TFT_eSprite(&tft);
  static bool spriteCreated = false;
  
  // Флаг, который помнит, рисовали ли мы анимацию в прошлом кадре
  static bool wasActive = false; 

  // =========================================================================
  // РЕЖИМ ОЧИСТКИ: Если анимацию выключили, но в прошлом кадре она еще была
  // =========================================================================
  if (!active) {
    if (wasActive) {
      // Заливаем цветом основного фона ровно ту зону, где жил наш спрайт
      tft.fillRect(POS_X, POS_Y, SPR_W, SPR_H, COLOR_MAIN_BG);
      wasActive = false; // Сбросили флаг, чтобы не тереть экран вхолостую
    }
    return; // Выходим, ничего больше не рисуем
  }

  // Если зашли сюда — анимация активна
  wasActive = true;

  if (!spriteCreated) {
    if (!ringBuf.createSprite(SPR_W, SPR_H)) return; 
    spriteCreated = true;
  }

  if (!trailInitialized) {
    int startX = CENTER_X + BALL_R * cos(angle);
    int startY = CENTER_Y + BALL_R * sin(angle);
    for (int i = 0; i < TRAIL_LENGTH; i++) {
      trailX[i] = startX;
      trailY[i] = startY;
    }
    trailInitialized = true;
  }

  if (millis() - lastBallTime > 16) {
    lastBallTime = millis();

    for (int i = 0; i < TRAIL_LENGTH - 1; i++) {
      trailX[i] = trailX[i + 1];
      trailY[i] = trailY[i + 1];
    }
    
    int bx = CENTER_X + BALL_R * cos(angle);
    int by = CENTER_Y + BALL_R * sin(angle);
    trailX[TRAIL_LENGTH - 1] = bx;
    trailY[TRAIL_LENGTH - 1] = by;

    ringBuf.fillSprite(COLOR_MAIN_BG);

    ringBuf.fillCircle(CENTER_X, CENTER_Y, BALL_R + 5, COLOR_RING_BG); 
    ringBuf.fillCircle(CENTER_X, CENTER_Y, BALL_R - 5, COLOR_MAIN_BG); 

    for (int i = 0; i < TRAIL_LENGTH; i++) {
      int brightness = 45 + (i * (70 / TRAIL_LENGTH));
      uint16_t segmentColor = tft.color565(brightness, brightness, brightness);
      int dynamicRadius = 1 + (i * 6 / TRAIL_LENGTH);
      ringBuf.fillCircle(trailX[i], trailY[i], dynamicRadius, segmentColor);
    }

    ringBuf.fillCircle(bx, by, 7, tft.color565(135, 135, 140));

    ringBuf.pushSprite(POS_X, POS_Y); 

    angle += 0.026f; 
    if (angle >= 2 * PI) angle = 0;
  }
}
void sensorsrequestTemperatures() {
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);
  if (currentTemp == DEVICE_DISCONNECTED_C || currentTemp < -50) {
    dutyCycle = 255; 
    ledcWrite(PWM_CHANNEL, dutyCycle);
    return;
  }
  if (currentTemp >= 55.0) {
    dutyCycle = 255; // ПОЛНОСТЬЮ ОТКРЫТ (холодный режим транзистора)
  } 
  else if (currentTemp <= 43.0) {
    dutyCycle = 0;   // ПОЛНОСТЬЮ ЗАКРЫТ
  }
  ledcWrite(PWM_CHANNEL, dutyCycle);
}
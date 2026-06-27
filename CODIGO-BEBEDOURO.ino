#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ============================================================
// CONFIGURAÇÕES DE REDE
// ============================================================
// Substitua pelo Wi-Fi do seu laboratório ou roteador do celular
const char* SSID     = "CIMATEC-VISITANTE"; 
const char* PASSWORD = "";

// ============================================================
// CREDENCIAIS SUPABASE 
// ============================================================
const char* SUPABASE_URL = "https://abcixwqyasjaycnhughw.supabase.co/rest/v1/leituras_curral";
const char* SUPABASE_KEY = "sb_publishable_i50SjuvVqKZVg1kbUQeLOQ_Y_hNoHzs";

// ============================================================
// DEFINIÇÕES DE HARDWARE
// ============================================================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define TRIG_PIN 5
#define ECHO_PIN 18

// ============================================================
// CONTROLE DE TEMPO E SIMULAÇÃO
// ============================================================
const unsigned long INTERVALO_MS = 5000; // Lê a cada 5 segundos
unsigned long ultimaLeitura = 0;
int contadorLeituras = 0;

// ============================================================
// ESTRUTURA E BUFFER DO SUPABASE
// ============================================================
struct LeituraAgua {
  float nivel_agua;
  float temperatura;
  float ph;
  float turbidez;
  float condutividade;
};

#define BUFFER_SIZE 20
LeituraAgua bufferPendente[BUFFER_SIZE];
int bufferCount = 0;

// ─────────────────────────────────────────────────────────
//  Envia UMA leitura ao Supabase via HTTP POST
// ─────────────────────────────────────────────────────────
bool enviarSupabase(LeituraAgua& l) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.begin(SUPABASE_URL);
  http.addHeader("Content-Type",  "application/json");
  http.addHeader("apikey",        SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + String(SUPABASE_KEY));
  http.addHeader("Prefer",        "return=minimal");

  StaticJsonDocument<256> doc;
  doc["nivel_agua"]    = l.nivel_agua;
  doc["temperatura"]   = l.temperatura;
  doc["ph"]            = l.ph;
  doc["turbidez"]      = l.turbidez;
  doc["condutividade"] = l.condutividade;

  String payload;
  serializeJson(doc, payload);

  int httpCode = http.POST(payload);
  http.end();

  return (httpCode == 201); // 201 Created indica sucesso no Supabase
}

// ─────────────────────────────────────────────────────────
//  Reenvio das leituras pendentes no buffer
// ─────────────────────────────────────────────────────────
void reenviarBuffer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[BUFFER] Sem WiFi, reenvio adiado.");
    return;
  }

  int reenviados = 0;
  int i = 0;

  while (i < bufferCount) {
    if (enviarSupabase(bufferPendente[i])) {
      memmove(bufferPendente + i,
              bufferPendente + i + 1,
              (bufferCount - i - 1) * sizeof(LeituraAgua));
      bufferCount--;
      reenviados++;
    } else {
      i++;
    }
  }
  if (reenviados > 0) {
    Serial.printf("[BUFFER] Reenviados: %d | Pendentes: %d\n", reenviados, bufferCount);
  }
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("\n============================================");
  Serial.println("   SISTEMA DE QUALIDADE DA ÁGUA - INICIANDO  ");
  Serial.println("============================================");

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando WiFi");
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500); 
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[AVISO] Sem WiFi - modo buffer ativado.");
  }
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO_MS) {
    ultimaLeitura = agora;

    // 1. Tenta reenviar dados retidos sem internet
    if (bufferCount > 0) {
      reenviarBuffer();
    }

    // 2. Leitura de Sensores Físicos
    float temperatura_real = dht.readTemperature();
    if (isnan(temperatura_real)) temperatura_real = 0.0;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duracao = pulseIn(ECHO_PIN, HIGH);
    float distancia_cm = duracao * 0.034 / 2;
    // Supondo que a vasilha tem 20cm de altura total.
    float nivel_agua = 20.0 - distancia_cm; 

    // 3. Mock: Geração de Dados Sintéticos e Anomalias
    float ph_simulado, turbidez_simulada, condutividade_simulada;
    contadorLeituras++;

    if (contadorLeituras >= 10) {
      Serial.println("\n>>> ALERTA: SIMULANDO CONTAMINAÇÃO DA ÁGUA <<<");
      ph_simulado = random(40, 50) / 10.0;         // pH Ácido
      turbidez_simulada = random(60, 100);         // Água Turva
      condutividade_simulada = random(1200, 1500); // Excesso de minerais
      contadorLeituras = 0;
    } else {
      ph_simulado = random(65, 80) / 10.0;         // Ideal
      turbidez_simulada = random(0, 15);           // Ideal
      condutividade_simulada = random(500, 800);   // Ideal
    }

    // 4. Prepara o pacote (Struct)
    LeituraAgua leituraAtual;
    leituraAtual.nivel_agua = nivel_agua;
    leituraAtual.temperatura = temperatura_real;
    leituraAtual.ph = ph_simulado;
    leituraAtual.turbidez = turbidez_simulada;
    leituraAtual.condutividade = condutividade_simulada;

    // 5. Exibição
    Serial.println("--------------------------------------------");
    Serial.print("Nível: "); Serial.print(nivel_agua); Serial.println(" cm");
    Serial.print("Temperatura: "); Serial.print(temperatura_real); Serial.println(" C");
    Serial.print("pH: "); Serial.println(ph_simulado);
    Serial.print("Turbidez: "); Serial.print(turbidez_simulada); Serial.println(" NTU");
    Serial.print("Condutividade: "); Serial.print(condutividade_simulada); Serial.println(" uS/cm");

    // 6. Envio via Supabase com Buffer de Proteção
    if (enviarSupabase(leituraAtual)) {
      Serial.println("SUPABASE: Dados gravados com sucesso!");
    } else {
      Serial.println("SUPABASE: Falha no envio. Guardando no buffer...");
      if (bufferCount < BUFFER_SIZE) {
        bufferPendente[bufferCount++] = leituraAtual;
        Serial.printf("BUFFER: %d/%d leituras pendentes.\n", bufferCount, BUFFER_SIZE);
      } else {
        Serial.println("BUFFER: Cheio! Descartando leitura mais antiga.");
        memmove(bufferPendente, bufferPendente + 1, (BUFFER_SIZE - 1) * sizeof(LeituraAgua));
        bufferPendente[BUFFER_SIZE - 1] = leituraAtual;
      }
    }
    Serial.println("============================================\n");
  }
}
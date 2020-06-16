# LPWA

LPWAのテスト用プログラム置き場

---

## LoRa

LoRaのためのテストプログラム
温度と光センサーの値をLoRa通信する。
SenseWay Mission Connectのサービスを利用したもの。

### LoRa構成

``` 構成.
Sensor => Arduino(LoRaWAN Shield for Arduino付) =(LoRaWAN)> SenseWay Mission Connect
```

### 参考

1. 使用したサービス - [サービス概要 ｜ センスウェイ株式会社](https://www.senseway.net/service/network-service/)
2. 使用したデバイス - [LoRaWAN Shield for Arduino ｜ SenseWay Mission Connect LoRaWAN 対応製品ガイド](https://www.senseway.net/lorawan-service/item/device/lorawan-shield-for-arduino/)

---

## mqtt_subscribe

SenseWay Mission Connectからデータを受け取るためのプログラム
AWSやAzureなどでデータを吸出ししたくない時に運用することができる。

### mqtt_subscribe構成

``` 構成.
Arduino =(LoRaWAN)> SenseWay Mission Connect =(MQTT)> mqttsubscribe
```

詳細はファイル内のREADMEを参照

---

## Sigfox

Sigfoxをテストで送信してみるもの

### Sigfox構成

``` 構成.
マイコン =(UART)> Sigfoxbreakoutboard =(Sigfox)> sigfoxサービス
```

### 参考

1. 使用したデバイス - [Sigfox Breakout board BRKWS01 RC3 - スイッチサイエンス](https://www.switch-science.com/catalog/5373/)
2. プログラム参考 - [Sigfox Breakout Board (BRKWS01 RC3)の利用方法 - Qiita](https://qiita.com/ghibi/items/37154bad78d9bb57e335)
3. 参考 - [Sigfox Devkit（開発キット）を1年間の無償回線で利用する - Qiita](https://qiita.com/ghibi/items/5893923b44f1757294e9)
4. BackendSigfox(利用したサービス) - [BackendSigfox](https://backend.sigfox.com/auth/login)
5. 参考 - [Sigfox IoT Agency Platformで簡単ダッシュボード - すべてのモノが「つながる」新たな未来へ。IoTネットワーク「Sigfox」](https://www.kccs-iot.jp/20190121-technical/)
# Gait_Sensor

歩容センサーの研究用リポジトリ
アーカイブ目的で運用している。

---

## 概要

健康促進などのための歩容を計測するデバイスを作成している。
その研究データを貯蓄するリポジトリ。

---

## ファイル構成

詳細は各フォルダー内に記載する。

- Gaitsensor --- 歩容センサーの計測を行うプログラム。BLE通信で左右の足のデータを計測し、Bluetooth classicで表示する。LPWA通信する機能などを追加予定。
- GaitSensor_Blynkdemo --- 歩容センサーのデモプログラム。スマートフォンアプリの[Blynk](https://blynk.io/)を用いて歩容の情報を表示するプログラムになっている。バグあり。
- GaitSensor_pintest --- 歩容センサーの動作確認をすることができるプログラム。
- GaitSensor_resource --- 歩容センサーを構築するための3Dプリンターのモデルをためておく場所
- LAWA --- LPWA通信(SensewayのLoRaWAN, Sigfox)をすることができるプログラム。

---

## 使用しているハードウェア

なるべく持続的に購入することができるリンクを添付しておく。

### ```マイコン部分```

- [ATOM Lite](https://www.switch-science.com/catalog/6262/)

今後はnRF52などのマイコンでの動作を行う予定。

### ```電池系```

- [リチウムイオンポリマー電池(3.7V、300mAh) DTP502035(PHR)](https://www.marutsu.co.jp/pc/i/1559500/)
- [LiPo Charger Plus(LiPo充電器) PRT-15217](https://www.marutsu.co.jp/pc/i/1558373/)

今後はボタン電池稼働を目指している。

### ```センサー```

センサーはetextileセンサーを用いている。


## 参加した学会

おそらく閲覧するにはいろいろ手続きが必要。

1. [CiNii 論文 -  インソール型歩容センサーのための歩容解析システムの構築](https://ci.nii.ac.jp/naid/170000182936/)
2. [研究会 開催プログラム - 2020-05-HIP-HCS-HI-SIGCE](https://www.ieice.org/ken/program/index.php?tgs_regid=d7f360cdb003f3c0347899b62250078bd4f27df02b0f48658e3a7b9b5705ebab)

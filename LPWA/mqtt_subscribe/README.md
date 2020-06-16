# mqtt subscriber for sensewaymisson connect

## 概要

LoRaのデータを管理するsenseway misson connectからのデータ受信をmqttプロトコルを用いて行う。
受信したデータは任意のデータのみ取り出し、csvにデータを保存する。

## システム構成

Arduino LoRaシールド =(LoRa)=> senseway mission connect =(mqtt)=> PC　=> csvfile

## ファイル構成

* mqtt_subscribe.py     ... mqttのサブスクライブが行える
* read_passward.py      ... sensewaymissonconnectのユーザー情報が書かれているテキストファイルからユーザー情報を読み取る
* pass_example.txt      ... pass.txtの書き方が書いてある
* Pipfile               ... pipのバージョン管理用
* data/data.csv         ... mqttsubscribeの出力先
* data/data_example.csv ... csvの出力例

## 実行環境

### python3.x系

各バージョン毎で動作確認等は行っていないが3.7.3で構築を行っているのでその近辺のバージョンは
確実に動く

### pip

#### * paho-mqtt

MQTTをpythonで動かすのに使用するライブラリ

#### ``` $pip install paho-mqtt ```

でインストールできる。

#### * pandas

データ解析を行うライブラリ
csv出力に使用する

#### ``` $pip install pandas ```

でインストールできる。


### memo

pipのインストールはproxy環境下では動作しません。

``` $pip install paho-mqtt --proxy=url:port```

で動作が行えます。


## 実行方法

pass_example.txtの中のように
* userid
* passward
* deveui
を記入したpass.txtを作成して

pythonファイルがあるディレクトリで

``` $python  mqtt_subscribe.py```

で実行することができる。


### 参考URL
1. [pythonでMQTT送受信 - Qiita](https://qiita.com/hsgucci/items/6461d8555ea1245ef6c2)
2. [pandas.DataFrameの構造とその作成方法 | note.nkmk.me](https://note.nkmk.me/python-pandas-dataframe-values-columns-index/)
import paho.mqtt.client as mqtt
import json
import datetime
import pandas as pd

from read_password import read_pass

password_file = "pass.txt"

USER_ID, USER_PASS, DEVEUI = read_pass(password_file)
print("USER_ID : " + USER_ID)
print("USER_PASS : " + USER_PASS)
print("DEVEUI : " + DEVEUI)

"""MQTT receive data!!"""
receive_data = None
receive_time = None
receive_diveui = None
receive_count = None

def csv_data_write(datalist):
  df = pd.DataFrame({"EUI"  : datalist[0],
                     "data" : datalist[1],
                     "time" : datalist[2],},
                     index=[datalist[3],])
  #print(df)
  df.to_csv("data/data.csv" , encoding="utf-8",  mode='a', header=False)

def header_csv():
  df = pd.DataFrame({"EUI"  : "EUI",
                     "data" : "data",
                     "time" : "time",},
                     index=[" ",])
  df.to_csv("data/data.csv" , encoding="utf-8",  mode='w', header=False)

# receive date time is UTC... need UTC to JST time. this function UTC string to JST string changing!
def utc_to_jst(timestamp_utc):
    datetime_utc = datetime.datetime.strptime(timestamp_utc, "%Y-%m-%dT%H:%M:%S.%f%z")
    datetime_jst = datetime_utc.astimezone(datetime.timezone(datetime.timedelta(hours =+ 9)))
    timestamp_jst = datetime.datetime.strftime(datetime_jst, '%Y-%m-%d %H:%M:%S')
    return timestamp_jst

# connect MQTT broker callback
def on_connect(client, userdata, flag, rc):
  print("Connected with result code : " + str(rc))
  #print("user ID : " + USER_ID + ", dev ID :" + DEVEUI)
  client.subscribe("lora/" + USER_ID + "/" + DEVEUI + "/rx")

# disconnect MQTT broker callback
def on_disconnect(client, userdata, flag, rc):
  if  rc != 0:
    print("Unexpected disconnection.\n")

# message receive callback
def on_message(client, userdata, msg):
  # msg.topic -> topic ，msg.payload -> main message
  #print("Received message '" + str(msg.payload))
  json_data = json.loads(msg.payload)

  # token
  print("===================receive data token===================")
  # temp data hex to Dec, / 100
  receive_data = float(int(json_data["mod"]["data"], 16) / 100)
  print("receive data : " + str(receive_data))
  # gateway receive time
  receive_time = utc_to_jst((str(json_data["gw"][0]["date"])))
  print("receive time : " + receive_time)
  # dev id
  receive_diveui = str(json_data["mod"]["devEUI"])
  print("receive EUI  : " + receive_diveui)
  # dev send count
  receive_count = json_data["mod"]["cnt"]
  print("send count   : " + str(receive_count))
  print("========================================================")
  write_data = [receive_diveui, receive_data, receive_time, receive_count]
  csv_data_write(write_data)


header_csv()

# MQTT connect opition
client = mqtt.Client()
#connect callback setting
client.on_connect = on_connect
#disconnect callback setting
client.on_disconnect = on_disconnect
#message callback setting
client.on_message = on_message
#client userid pass set
client.username_pw_set(USER_ID, USER_PASS)
#client connect execute! connect to sensewaymission connect
client.connect("mqtt.senseway.net", 1883, 60)

client.loop_forever()
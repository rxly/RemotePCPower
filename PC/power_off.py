import paho.mqtt.client as mqtt
import time,json
import hmac
import os

device_name = "PC"
device_secret = "ZH0UeXXXXXXXXXXXXXXX8MisgFxD4Elz"
product_key = "a1gXXXXXqNV"


region = 'cn-shanghai'
topic = '/%s/%s/update'%(product_key,device_name)
id = '12345'
url = "%s.iot-as-mqtt.%s.aliyuncs.com"%(product_key,region )
port = 1883
clientid = id+"|securemode=3,signmethod=hmacsha1|"
user_name = "%s&%s" %(device_name,product_key)
password = 'clientId%sdeviceName%sproductKey%s'%(id,device_name,product_key)
h = hmac.new(device_secret.encode('utf-8'),password.encode('utf-8'),digestmod='sha1')
print("URL:%s"%url)
print("client_id:%s"%clientid)
print("user_name:%s"%user_name)
print("password:%s"%h.hexdigest())
is_connect = False
def on_connect(client, userdata, flags, rc):
    global is_connect
    print("Connection returned " + str(rc))
    if rc == 0:
        is_connect = True

def on_msg_recv(client, userdata, message):
    payload = str(message.payload,encoding = "utf-8")
    data = json.loads(payload)
    if data.get("power_off") is not None and data['power_off'] == 1:
        print("pc will power off")
        os.system("shutdown -s -t 0")
        return
    if data.get('state') is not None and "?".__eq__(data['state']):
        print("app get state ")
        client.publish(topic,payload='{"pc_state":%d}'%(1),qos=1)



client = mqtt.Client(client_id=clientid)
client.username_pw_set(user_name,h.hexdigest())
client.on_message = on_msg_recv
client.on_connect = on_connect
client.connect(url,port,60)
client.loop_start()



while True:
    time.sleep(3)
    print("subscribe %s"%topic)
    client.subscribe(topic,1)
    # print("subscribe %s" % topic_state)
    # client.subscribe(topic_state, 1)

    while True:
        time.sleep(99999)
    # client.publish(topic,payload='{"power_on":%d}'%(1),qos=0)
    # val += 10
    # time.sleep(1)
    # print('publish done')
    # break
package com.xuan.wakeremotepc;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.json.JSONException;
import org.json.JSONObject;

public class MainActivity extends AppCompatActivity implements View.OnClickListener{


    String product_key = "a1gaXXXXXNV";
    String device_name = "mobilXXXX";
    String device_secret = "uW7LEZXXXXXXXXXXTrgCx7eC0Q8ioK3O";

    String TAG = "WOL";

    String region = "cn-shanghai";
    String topic = "/"+product_key+"/"+device_name+"/update";
    String id = "54321";

    String content = "Message from MqttPublishSample";
    int qos = 2;
    String broker = "tcp://"+product_key+".iot-as-mqtt."+region+".aliyuncs.com";
    String clientId = id+"|securemode=3,signmethod=hmacsha1|";
    String user_name = device_name + "&" + product_key;
    //    String password = "clientId"+id+"deviceName"+device_name+"productKey%s"+product_key;
    String password = "dea338c9dd1023ad4015465a3268ade6accaf110";

    MemoryPersistence persistence = new MemoryPersistence();

    final int MQTT_MSG_CONNECTED = 0X0000;
    final int MQTT_MSG_DISCONNECT = 0X0001;
    final int PC_STATE_ON    = 0X0002;
    final int PC_STATE_OFF    = 0X0003;

    private ImageButton conn_btn;
    private TextView info;
    private myHandler handler;
    private TextView state;
    MqttClient sampleClient;
    private checkPCStateThread check_state;

    private boolean is_pc_on = false;

    @Override
    public void onClick(View view) {

    }

    private class myHandler extends Handler{
        @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            switch (msg.what){
                case MQTT_MSG_CONNECTED:
                    conn_btn.setBackgroundResource(R.mipmap.power_enable);
                    conn_btn.setEnabled(true);
                    try {
                        sampleClient.subscribe(topic,0);
                        check_state.startCheck(2000);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                    break;
                case MQTT_MSG_DISCONNECT:
                    conn_btn.setBackgroundResource(R.mipmap.power_disable);
                    conn_btn.setEnabled(false);
//                    Toast.makeText(getApplicationContext(),"连接服务器出错  退出！",Toast.LENGTH_LONG).show();
                    break;
                case PC_STATE_ON:
                    is_pc_on = true;
                    state.setVisibility(View.VISIBLE);
                    state.setText("关机");
                    info.setText("远程电脑处于开机，按电源按键关闭计算机");
                    conn_btn.setBackgroundResource(R.mipmap.power_disable);
                    break;
                case PC_STATE_OFF:
                    is_pc_on = false;
                    conn_btn.setBackgroundResource(R.mipmap.power_enable);
                    state.setVisibility(View.VISIBLE);
                    info.setText("远程电脑处于关机，按电源按键启动计算机");
                    state.setText("开机");
                    break;
            }
        }
    }

    class check_connect_state implements Runnable{
        @Override
        public void run() {
            while (true){
                if(sampleClient.isConnected()){
                    handler.sendEmptyMessage(MQTT_MSG_CONNECTED);
                    break;
                }
            }
        }
    }

    boolean mqtt_connect(){
        if(sampleClient != null && sampleClient.isConnected()){
            return true;
        }
        try {
            sampleClient = new MqttClient(broker, clientId,persistence);
            sampleClient.setCallback(new MqttClientCB());
            MqttConnectOptions connOpts = new MqttConnectOptions();
            connOpts.setCleanSession(true);
            connOpts.setUserName(user_name);
            Log.i(TAG,"Password:"+password);
            Log.i(TAG,"Connecting to broker: " + broker);
            connOpts.setPassword(password.toCharArray());
            connOpts.setKeepAliveInterval(60);
            sampleClient.connect(connOpts);
            handler.sendEmptyMessage(MQTT_MSG_CONNECTED);
            return true;
        } catch (MqttException me) {
            Log.i(TAG,"reason " + me.getReasonCode());
            Log.i(TAG,"msg " + me.getMessage());
            Log.i(TAG,"loc " + me.getLocalizedMessage());
            Log.i(TAG,"cause " + me.getCause());
            Log.i(TAG,"excep " + me);
            handler.sendEmptyMessage(MQTT_MSG_DISCONNECT);
            me.printStackTrace();
            return false;
        }
    }

//    void pc_state_flush(){
//
//        handler.postDelayed(new Runnable() {
//            @Override
//            public void run() {
//                if(is_pc_on == false){
//                    handler.sendEmptyMessage(PC_STATE_OFF);
//                }else {
//                    handler.sendEmptyMessage(PC_STATE_ON);
//                }
//                handler.removeCallbacks(this);
//            }
//        },2000);
//    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        handler = new myHandler();

        conn_btn = findViewById(R.id.connect_btn);
        conn_btn.setEnabled(false);
        info = findViewById(R.id.pc_state);
        state = findViewById(R.id.power_info);
        conn_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    Log.i(TAG,"Publish topic "+topic);
                    String payload = new String();
                    if(is_pc_on)
                        payload = "{\"power_off\":1}";
                    else
                        payload = "{\"power_on\":1}";
                    MqttMessage msg = new MqttMessage(payload.getBytes());
                    sampleClient.publish(topic, msg);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
            }
        });
        check_state = new checkPCStateThread();
        mqtt_connect();

    }

    private class MqttClientCB implements MqttCallback{

        @Override
        public void connectionLost(Throwable cause) {
            Log.i(TAG,"connectionLost ---");
            if(handler != null){
                handler.sendEmptyMessage(MQTT_MSG_DISCONNECT);
            }
        }

        @Override
        public void messageArrived(String topic, MqttMessage message) {
            String data = new String(message.getPayload());
            Log.i(TAG,"message:"+data);
            JSONObject payload = null;
            try {
                payload = new JSONObject(data);
                if(payload.getInt("pc_state") == 1 && handler != null){
                    is_pc_on = true;
                    handler.sendEmptyMessage(PC_STATE_ON);
                }

            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void deliveryComplete(IMqttDeliveryToken token) {

        }
    }

    private class checkPCStateThread implements Runnable{
        MqttMessage msg = null;
        public checkPCStateThread(){
            msg = new MqttMessage("{\"state\":\"?\"}".getBytes());
        }

        public void startCheck(int time_delay){
            try {
                is_pc_on = false;
                sampleClient.publish(topic,msg);
            } catch (MqttException e) {
                e.printStackTrace();
            }
            handler.postDelayed(this,time_delay);
        }

        public void stopCheck(){
            handler.removeCallbacks(this);
        }

        @Override
        public void run() {
            if(is_pc_on == false){
                handler.sendEmptyMessage(PC_STATE_OFF);
            }else {
                handler.sendEmptyMessage(PC_STATE_ON);
            }
            startCheck(2000);
//            handler.postDelayed(this,2000);
        }
    }

}


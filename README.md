# RemotePCPower
手机app远程控制家里的电脑开关机
# 使用方法
1.修改android_app里面的MainActivity.java定义的三元组后编译出apk安装在手机上
    String product_key = "a1gaXXXXXNV";
    String device_name = "mobilXXXX";
    String device_secret = "uW7LEZXXXXXXXXXXTrgCx7eC0Q8ioK3O";
2.将NodeMCU里面的mqtt_example.c替换到https://github.com/espressif/esp-aliyun里面，修改三元组后编译出固件烧录到NodeMCU里面
    #define PRODUCT_KEY             "a1gXXXXXqNV"
    #define PRODUCT_SECRET          "a4dtXXXXXXXXdlwP"
    #define DEVICE_NAME             "NodeMCU"
    #define DEVICE_SECRET           "9L4CSU2XXXXXXXXXXnMeYo7bZWIApqu0"
3.将PC文件加下的两个文件放到 D:/StartUp/里面，设置此电脑 ->任务计划程序 选择创建开机自动运行任务

未完待续....

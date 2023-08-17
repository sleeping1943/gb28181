# gb28181-server说明

---

## 平台说明
    此项目基于ubuntu20.04开发测试,暂不支持windows系统

## 依赖库和其他服务

* 此项目sip协议使用osip2-5.1.2和exosip2-5.1.2,版本请一定对应,以防未知错误;
* osip2-5.1.2百度云链接：https://pan.baidu.com/s/1P25xUSri3otwjfaPo5g9Zg 提取码：1234
* exosip2-5.1.2百度云链接：https://pan.baidu.com/s/1ClkPjDzc5VjSCthNAL0nLQ 提取码：1234

官方下载地址:
```
  osip:   http://ftp.twaren.net/Unix/NonGNU/osip/
  exosip: http://download.savannah.gnu.org/releases/exosip/
  c-ares: https://c-ares.haxx.se/
``````
其中c-ares为windows开发时需要用到的依赖库,windows暂不支持,顾不用下载;

### 流媒体服务器
* 使用开源的ZLMediakit作为流媒体服务器,下载地址:
```
   https://github.com/ZLMediaKit/ZLMediaKit
```
## 编译运行
* 编译安装osip,默认安装目录/usr/local/include和/usr/local/lib
```
cd osip2-5.1.2  
 ./configure
 make
 sudo make install
```

* 编译安装exosip2-5.1.2,默认安装目录/usr/local/include和/usr/local/lib
```
 cd exosip2-5.1.2
 ./configure
 make
 sudo make install
```

* 编译gb28181-server
```
1、执行对应版本的sh文件即可，如debug版本则执行debug.sh
./debug.sh
2、运行
./gb28181-server
3、退出
ctrl+c,程序接收信号退出执行而不是直接暴力退出
```

* 项目使用xmake作为构建工具,使用debug.sh/release.sh也会自动使用xmake生成CMakeLists.txt和makefile


## 特别感谢
    此项目参考https://gitee.com/Vanishi/BXC_SipServer

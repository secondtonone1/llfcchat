---
title: webrtc的coturn服务搭建
date: 2026-02-11 08:51:30
tags: [C++聊天项目] 
categories: [C++聊天项目]
---

## 前情回顾

前面我们已经将聊天的基本功能完成了，接下来来安装coturn服务，生成webrtc视频通信

coturn服务集成了stun和turn服务器

------

**三个常用的概念**

> **STUN = 告诉你自己是谁**
>  **TURN = 帮你转发数据**
>  **coturn = STUN/TURN 的服务器实现软件**

------

![image-20260212125500580](https://cdn.llfc.club/image-20260212125500580.png)



## 为什么需要 STUN / TURN？

因为现在大部分设备都在 **NAT（内网）后面**：

```
手机/电脑 → 路由器(NAT) → 公网
```

内网 IP（192.168.x.x）在公网是看不到的。

WebRTC / 视频通话 / P2P 通信 就必须解决：

👉 “我怎么让对方知道我的公网地址？”

------

## STUN 是什么？

### 全称：

Session Traversal Utilities for NAT

### 作用：

👉 **告诉客户端：你的公网 IP 和端口是多少**

工作流程：

```
客户端 → STUN服务器
STUN服务器 → 告诉你：你在公网看到的地址是 1.2.3.4:5678
```

然后客户端把这个地址发给对方。

------

### ✅ 优点

- 轻量
- 免费
- 不转发数据

### ❌ 缺点

- 只能在 NAT 允许打洞的情况下成功
- 对称 NAT 基本失败

------

## TURN 是什么？

### 全称：

Traversal Using Relays around NAT

### 作用：

👉 **当打洞失败时，帮你转发数据**

工作模式：

```
A → TURN服务器 → B
```

TURN 服务器相当于一个“中转站”。

------

### ✅ 优点

- 几乎 100% 成功
- 解决所有 NAT 问题

### ❌ 缺点

- 服务器要承担带宽
- 成本高

------

## coturn 是什么？

👉 coturn 是一个 **开源的 STUN + TURN 服务器软件**

官网：
 https://github.com/coturn/coturn

你部署 coturn 之后：

- 可以当 STUN 服务器
- 也可以当 TURN 服务器

------

### 三者关系图

```
            WebRTC 通信
                 │
     ┌───────────┴───────────┐
     │                       │
  先尝试 STUN           如果失败 → TURN
     │                       │
  直接P2P通信         通过服务器转发
                 │
             coturn
     （实现STUN和TURN的程序）
```



## 安装coturn

因为docker被限制，自己尝试安装了docker拉取coturn服务还是超时，所以果断选择本机安装。

本机安装有两种方式，一种源码方式，一种集成服务安装。

官方推荐集成服务安装，源码安装是为了制作自己的webrtc服务，修改源码才用到，这里我们不用。

``` bash
# 更新软件包
sudo apt update

# 安装 Coturn
sudo apt install coturn -y

# 查看版本
turnserver --help | head -1

```

## 修改配置文件

先备份原来的配置文件

``` bash
sudo cp /etc/turnserver.conf /etc/turnserver.conf.bak
```

再修改配置文件

``` bash
sudo vim /etc/turnserver.conf
```

改为如下内容

``` bash
# 监听端口
listening-ip = 0.0.0.0
listening-port=3478
tls-listening-port=5349

# 外部 IP
external-ip=81.68.86.146
#relay-ip=81.68.86.146

# 启用指纹
fingerprint

# 使用长期凭证机制
lt-cred-mech

# 用户认证
user=webrtc:Kx9mP2qL8rY5tZ

# Realm
realm=turn.example.com

# 日志
log-file=/var/log/turnserver/turnserver.log
verbose

# 端口范围
min-port=49152
max-port=65535

# 拒绝私有 IP
no-multicast-peers
denied-peer-ip=0.0.0.0-0.255.255.255
denied-peer-ip=10.0.0.0-10.255.255.255
denied-peer-ip=172.16.0.0-172.31.255.255
denied-peer-ip=192.168.0.0-192.168.255.255

```

## 创建日志目录

```bash
# 创建日志目录
sudo mkdir -p /var/log/turnserver

# 设置权限
sudo chown turnserver:turnserver /var/log/turnserver
```

### 验证服务

```bash
# 查看端口监听
sudo netstat -tuln | grep 3478

# 查看日志
sudo tail -f /var/log/turnserver/turnserver.log

# 或者
sudo journalctl -u coturn -f
```



## 启用并启动服务

```
# 启用 Coturn 服务
sudo sed -i 's/#TURNSERVER_ENABLED=1/TURNSERVER_ENABLED=1/' /etc/default/coturn

# 启动服务
sudo systemctl enable coturn
sudo systemctl start coturn

# 查看状态
sudo systemctl status coturn
```

如果出错，可以手动编辑设置启动 Coturn 服务

```
# 编辑配置文件
sudo vim /etc/default/coturn
```

找到这一行：

```
#TURNSERVER_ENABLED=1
```

**去掉注释符号 `#`**，改为：

```
TURNSERVER_ENABLED=1
```

保存并且退出

## 常用管理命令

```bash
# 启动服务
sudo systemctl start coturn

# 停止服务
sudo systemctl stop coturn

# 重启服务
sudo systemctl restart coturn

# 查看状态
sudo systemctl status coturn

# 查看日志
sudo journalctl -u coturn -n 100 -f

# 查看配置文件
sudo cat /etc/turnserver.conf
```

------

## 云服务器开启端口

### 必须开放的端口：

- **3478** (UDP + TCP) - STUN/TURN 主端口
- **49152-65535** (UDP) - TURN 数据中继端口范围

### 可选端口（如果使用 TLS）：

- **5349** (UDP + TCP) - TURN over TLS

------

## 腾讯云安全组配置步骤

### 1. 登录腾讯云控制台

访问：https://console.cloud.tencent.com/cvm/instance

### 2. 配置安全组

```
控制台 → 云服务器 → 实例列表 → 找到你的服务器 → 安全组 → 修改规则
```

### 3. 添加入站规则

| 类型   | 端口范围    | 协议 | 来源      | 策略 |
| ------ | ----------- | ---- | --------- | ---- |
| 自定义 | 3478        | UDP  | 0.0.0.0/0 | 允许 |
| 自定义 | 3478        | TCP  | 0.0.0.0/0 | 允许 |
| 自定义 | 49152-65535 | UDP  | 0.0.0.0/0 | 允许 |
| 自定义 | 5349        | UDP  | 0.0.0.0/0 | 允许 |
| 自定义 | 5349        | TCP  | 0.0.0.0/0 | 允许 |

------

![image-20260211135234856](https://cdn.llfc.club/image-20260211135234856.png)

## 快速验证端口是否开放

在你的服务器上执行：

```
# 查看 Coturn 是否监听端口
sudo netstat -tuln | grep -E '3478|5349'

# 应该看到类似输出：
# udp        0      0 0.0.0.0:3478            0.0.0.0:*
# tcp        0      0 0.0.0.0:3478            0.0.0.0:*
```

从外部测试（在你的本地电脑上）：

```
# 测试 UDP 端口（需要安装 nc）
nc -u -v 81.68.86.146 3478

# 测试 TCP 端口
nc -v 81.68.86.146 3478
```

## 测试 TURN 服务

访问测试页面：https://webrtc.github.io/samples/src/content/peerconnection/trickle-ice/

填入：

- **TURN URI**: `turn:81.68.86.146:3478`
- **Username**: `webrtc`
- **Password**: `Kx9mP2qL8rY5tZ`

点击 **Gather candidates**，如果看到 `relay` 类型的候选者，说明成功！

先添加，然后点击add server

![image-20260211132631986](https://cdn.llfc.club/image-20260211132631986.png)

然后点击下方

![image-20260211133328948](https://cdn.llfc.club/image-20260211133328948.png)

------

## 如果需要修改配置

```
# 编辑配置文件
sudo vim /etc/turnserver.conf

# 保存后重启服务
sudo systemctl restart coturn

# 查看日志确认
sudo journalctl -u coturn -f
```

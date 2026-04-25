---
title: webrtc信令服务器实现
date: 2026-02-11 19:23:38
tags: [C++聊天项目]
categories: [C++聊天项目]
---

# WebRTC 1v1 视频通话技术方案（Node.js + WebSocket）

------

# 一、整体架构设计

## 1.1 系统架构图

```
浏览器A  ─────┐
               │        (SDP / ICE 交换)
浏览器B  ─────┼──── WebSocket ─── 信令服务器 (Node.js)
               │
               │
        媒体流 (P2P / TURN 中继)
```

## 1.2 组件职责

| 组件        | 职责                               |
| ----------- | ---------------------------------- |
| 前端浏览器  | 建立 RTCPeerConnection，采集音视频 |
| 信令服务器  | 交换 offer / answer / ice          |
| TURN 服务器 | NAT 穿透失败时中继媒体流           |
| STUN        | 获取公网候选地址                   |

------

# 二、信令服务器技术选型

| 技术     | 原因                  |
| -------- | --------------------- |
| Express  | 快速搭建 HTTP 服务    |
| ws       | 轻量级 WebSocket 实现 |
| 原生 Map | 管理房间              |

信令服务器不参与媒体传输，只负责：

- 房间管理
- 消息转发



# 三、核心代码结构解析

##  3.0通信流程

![image-20260213085510995](https://cdn.llfc.club/image-20260213085510995.png)



------

## 3.1 静态资源加载

```js
app.use(express.static(path.join(__dirname, "..", "web")));
```

### 作用

- 将 web 目录暴露为 HTTP 静态站点
- 访问 [http://localhost:3000](http://localhost:3000/) 即加载 index.html

------

## 3.2 WebSocket 服务器创建

```js
const wss = new WebSocketServer({ server });
```

说明：

- 复用 HTTP 服务器
- WebSocket 与 HTTP 共享 3000 端口

------

# 四、房间模型设计

```js
const rooms = new Map();
```

结构：

```
Map<roomId, Set<ws>>
```

示例：

```
rooms = {
   "1001" => Set(wsA, wsB)
}
```

设计思想：

- 每个房间最多 2 人
- 每个 ws 保存自己所属 roomId

------

# 五、信令流程设计

## 5.1 加入房间流程

客户端发送：

```json
{
  "type": "join",
  "roomId": "1001"
}
```

服务器逻辑：

1. 检查 roomId
2. 从旧房间移除
3. 如果人数 >= 2 返回 full
4. 加入房间
5. 返回 joined
6. 如果人数 == 2，发送 ready

------

## 5.2 ready 阶段

当房间人数 = 2 时：

```js
send(peer, { type: "ready", isInitiator });
```

设计：

- 后加入者作为 initiator
- initiator 负责创建 offer

------

## 5.3 WebRTC 协商流程

### 时序图

```
Initiator                Receiver
   |                        |
   | ---- offer ----------> |
   |                        |
   | <--- answer -----------|
   |                        |
   | <--- ICE --------------|
   | ---- ICE ------------> |
```

------

## 5.4 信令转发逻辑

```js
if (type === "offer" || type === "answer" || type === "ice") {
  relayToOthers(roomId, ws, msg);
}
```

服务器不解析 SDP，不参与媒体。

它只是“转发器”。

------

# 六、核心函数解析

------

## 6.1 send()

```js
function send(ws, data) {
  if (ws.readyState === ws.OPEN)
    ws.send(JSON.stringify(data));
}
```

说明：

- 统一发送 JSON
- 防止向已关闭连接发送数据

------

## 6.2 relayToOthers()

```js
function relayToOthers(roomId, sender, data)
```

功能：

- 向房间内除自己外的人广播
- 实现一对一信令交换

------

## 6.3 removeFromRoom()

负责：

- 用户离开
- 通知房间剩余成员
- 清理空房间

防止内存泄漏。

------

# 七、连接生命周期管理

```js
ws.on("close", () => removeFromRoom(ws));
ws.on("error", () => removeFromRoom(ws));
```

说明：

- 防止异常断开导致房间脏数据
- 保证 rooms 数据一致性

------

## 完整源码

``` js
import express from "express";
import http from "http";
import { WebSocketServer } from "ws";

import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
// 把 web 目录设置成：server 的上一级 + /web
app.use(express.static(path.join(__dirname, "..", "web")));

const server = http.createServer(app);
const wss = new WebSocketServer({ server });

/**
 * rooms: Map<roomId, Set<ws>>
 */
const rooms = new Map();

/** 给某个 ws 发送 JSON */
function send(ws, data) {
  if (ws.readyState === ws.OPEN) ws.send(JSON.stringify(data));
}

/** 房间内给除了自己之外的其他人转发 */
function relayToOthers(roomId, sender, data) {
  const set = rooms.get(roomId);
  if (!set) return;
  for (const peer of set) {
    if (peer !== sender) send(peer, data);
  }
}

function removeFromRoom(ws) {
  const roomId = ws.roomId;
  if (!roomId) return;

  const set = rooms.get(roomId);
  if (!set) return;

  set.delete(ws);
  // 通知剩余的人：对方离开
  for (const peer of set) {
    send(peer, { type: "peer-left" });
  }

  if (set.size === 0) rooms.delete(roomId);
  ws.roomId = null;
}

wss.on("connection", (ws) => {
  ws.roomId = null;

  ws.on("message", (buf) => {
    let msg;
    try {
      msg = JSON.parse(buf.toString());
    } catch {
      return;
    }

    const { type } = msg;

    if (type === "join") {
      const roomId = String(msg.roomId || "").trim();
      if (!roomId) return send(ws, { type: "error", message: "roomId is required" });

      // 如果 ws 已在别的房间，先移除
      removeFromRoom(ws);

      const set = rooms.get(roomId) || new Set();
      if (set.size >= 2) {
        return send(ws, { type: "full", roomId });
      }

      set.add(ws);
      rooms.set(roomId, set);
      ws.roomId = roomId;

      send(ws, { type: "joined", roomId, peers: set.size - 1 });

      // 房间凑齐 2 人，通知双方开始协商
      if (set.size === 2) {
        // 约定：后加入的人做 initiator（也可以反过来）
        for (const peer of set) {
          const isInitiator = peer === ws;
          send(peer, { type: "ready", isInitiator });
        }
      }
      return;
    }

    // 后面的消息都必须在房间里
    const roomId = ws.roomId;
    if (!roomId) return send(ws, { type: "error", message: "join a room first" });

    // 透传 WebRTC 协商消息
    if (type === "offer" || type === "answer" || type === "ice") {
      relayToOthers(roomId, ws, msg);
      return;
    }

    if (type === "leave") {
      removeFromRoom(ws);
      send(ws, { type: "left" });
      return;
    }
  });

  ws.on("close", () => removeFromRoom(ws));
  ws.on("error", () => removeFromRoom(ws));
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, "0.0.0.0", () => {
  console.log(`Signaling+Web server running: http://localhost:${PORT}`);
});

```



# 八、为什么信令服务器不处理媒体？

因为：

WebRTC 是 P2P 协议。

媒体路径：

```
浏览器A  ←→  浏览器B
```

不是：

```
浏览器A  → 服务器 → 浏览器B
```

除非使用 SFU。

------

# 九、当前版本限制

| 项目     | 当前实现  |
| -------- | --------- |
| 房间人数 | 最多 2 人 |
| 认证     | 无        |
| 房间权限 | 无        |
| 重连机制 | 无        |
| 多人视频 | 不支持    |

------

# 十、如何扩展为多人房间（技术方向）

当前结构：

```
Map<roomId, Set<ws>>
```

升级方案：

1. 为每个 ws 分配唯一 peerId
2. 信令改为定向发送
3. 前端维护：

```
Map<peerId, RTCPeerConnection>
```

每加入一个人：

- 为其创建一个新的 PeerConnection
- 动态创建 video 元素

这叫：

> Mesh 架构

------

# 十一、生产环境建议

## 1️⃣ 使用 HTTPS + WSS

WebRTC 在公网通常必须 HTTPS。

## 2️⃣ TURN 使用动态签名

不要写死：

```
user=webrtc:password
```

应改为：

```
use-auth-secret
static-auth-secret=xxx
```

防止带宽被盗用。

## 3️⃣ 加入房间认证

目前任何人知道房间号即可进入。

应加入：

- token
- 用户身份

------


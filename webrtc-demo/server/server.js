import express from "express";
import http from "http";
import { WebSocketServer } from "ws";

import path from "path";
import { fileURLToPath } from "url";

//获取当前文件的路径，server.js文件路径
const __filename = fileURLToPath(import.meta.url);
//去掉文件名，获取目录的路径
const __dirname = path.dirname(__filename);

const app = express();
// 把 web 目录设置成：server 的上一级 + /web
app.use(express.static(path.join(__dirname, "..", "web")));
// 创建http服务
const server = http.createServer(app);
//创建websocket
const wss = new WebSocketServer({ server });

/**
 * rooms: Map<roomId, Set<ws>>
 */
const rooms = new Map();

/** 给某个 ws 发送 JSON */
function send(ws, data) {
  //判断ws是否处于可用状态，将data序列化为字符串发送给ws的对端(客户端)
  if (ws.readyState === ws.OPEN) ws.send(JSON.stringify(data));
}

/** 房间内给除了自己之外的其他人转发 */
function relayToOthers(roomId, sender, data) {
  //ws集合
  const set = rooms.get(roomId);
  if (!set) return;
  for (const peer of set) {
    //如果peer不是发送方，那么就是接收方的ws
    if (peer !== sender) send(peer, data);
  }
}

function removeFromRoom(ws) {
  //ws 中包含roomId
  const roomId = ws.roomId;
  if (!roomId) return;
  //根据roomId找到集合
  const set = rooms.get(roomId);
  if (!set) return;
  //将ws从集合中删除
  set.delete(ws);
  // 通知剩余的人：对方离开
  for (const peer of set) {
    send(peer, { type: "peer-left" });
  }
  //如果双方都离开，则删除房间
  if (set.size === 0) rooms.delete(roomId);
  //将roomId设置为null
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
    //将msg中的key为type的value取出来到type存储
    /**
     *  {
     *     'type' : 'join',
     *     'roomId', '1001'  
     *  }
     */
    const { type } = msg;

    if (type === "join") {
      const roomId = String(msg.roomId || "").trim();
      if (!roomId) return send(ws, { type: "error", message: "roomId is required" });

      // 如果 ws 已在别的房间，先移除
      removeFromRoom(ws);
      //利用或的短路效应，可以先根据roomId获取集合，如果没有则调用new Set()，如果有则使用roomId对应的集合
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
          //判断找到peer和ws相等，也就是后加入的一端，如果不是后加入的isInitiator为false
          //如果是isInitiator则为true
          const isInitiator = peer === ws;
          //广播，后加入的isInitiator为true，先加入的为false
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

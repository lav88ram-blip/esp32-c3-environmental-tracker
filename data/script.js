// Airscope: direct WebSocket client. The ESP32 serves this page and its WebSocket on port 81.
const $ = (id) => document.getElementById(id);
const history = [];
let socket;
let reconnectTimer;
let received = 0;

function setStatus(live, message) {
  $("connectionStatus").textContent = message;
  $("statusDot").classList.toggle("live", live);
  $("deviceState").textContent = live ? "Online" : "Offline";
  $("connectionIndicator").classList.toggle("live", live);
}
function connectSocket() {
  if (socket && (socket.readyState === WebSocket.OPEN || socket.readyState === WebSocket.CONNECTING)) return;
  const host = window.location.hostname || "192.168.4.1";
  const url = "ws://" + host + ":81/";
  $("socketAddress").textContent = url;
  $("connectionDetail").textContent = "Connecting directly to " + url;
  try { socket = new WebSocket(url); } catch (e) { setStatus(false, "WebSocket unavailable"); scheduleReconnect(); return; }
  socket.onopen = () => { setStatus(true, "Connected · WebSocket"); $("connectionDetail").textContent = "Connected directly to the ESP32. Readings arrive as soon as the sensor is sampled."; };
  socket.onmessage = (event) => {
    try {
      const d = JSON.parse(event.data);
      if (typeof d.temperature !== "number" || typeof d.humidity !== "number") return;
      updateReading(d.temperature, d.humidity, Date.now());
    } catch (_) { /* Ignore malformed messages. */ }
  };
  socket.onerror = () => setStatus(false, "WebSocket connection error");
  socket.onclose = () => { setStatus(false, "Disconnected · reconnecting"); scheduleReconnect(); };
}
function scheduleReconnect() {
  clearTimeout(reconnectTimer);
  reconnectTimer = setTimeout(connectSocket, 2500);
}
function updateReading(temp, humidity, timestamp) {
  $("temperature").textContent = temp.toFixed(1);
  $("fahrenheit").textContent = (temp * 9 / 5 + 32).toFixed(1) + " °F";
  $("humidity").textContent = Math.round(humidity);
  $("humidityBar").style.width = Math.max(0, Math.min(100, humidity)) + "%";
  $("humidityLabel").textContent = humidity < 30 ? "Dry air" : humidity > 60 ? "Humid air" : "Moderate";
  $("lastUpdated").textContent = "Last sample: " + new Date(timestamp).toLocaleTimeString([], {hour:"2-digit",minute:"2-digit",second:"2-digit"});
  history.push({temp, hum:humidity});
  if (history.length > 60) history.shift();
  received++;
  $("readingCount").textContent = received + (received === 1 ? " reading received" : " readings received");
  drawChart();
}
function drawChart() {
  const canvas = $("historyChart"), ctx = canvas.getContext("2d"), rect = canvas.getBoundingClientRect();
  if (!rect.width || !rect.height) return;
  const dpr = window.devicePixelRatio || 1;
  canvas.width = Math.round(rect.width * dpr); canvas.height = Math.round(rect.height * dpr); ctx.scale(dpr,dpr);
  const w=rect.width,h=rect.height,p={l:32,r:12,t:14,b:25},points=history.slice(-24);
  ctx.clearRect(0,0,w,h); $("chartEmpty").style.display=points.length?"none":"grid";
  if(!points.length)return;
  const temps=points.map(x=>x.temp),min=Math.floor(Math.min(...temps)-2),max=Math.ceil(Math.max(...temps)+2);
  ctx.font="10px DM Mono, monospace";ctx.textAlign="right";
  for(let i=0;i<4;i++){const y=p.t+(h-p.t-p.b)*i/3;ctx.strokeStyle="#26364a";ctx.beginPath();ctx.moveTo(p.l,y);ctx.lineTo(w-p.r,y);ctx.stroke();ctx.fillStyle="#71839b";ctx.fillText(String(Math.round(max-(max-min)*i/3)),p.l-8,y+3);}
  const x=i=>p.l+(w-p.l-p.r)*(points.length===1?.5:i/(points.length-1)), y=v=>p.t+(h-p.t-p.b)*(1-(v-min)/(max-min||1));
  function line(vals,color){ctx.beginPath();vals.forEach((v,i)=>i?ctx.lineTo(x(i),y(v)):ctx.moveTo(x(i),y(v)));ctx.strokeStyle=color;ctx.lineWidth=2;ctx.lineJoin="round";ctx.stroke();}
  line(temps,"#70f0c0");
  // Normalize humidity onto the temperature plot range so both trends remain visible.
  line(points.map(a=>min+(a.hum/100)*(max-min)),"#7aa9ff");
  ctx.textAlign="left";ctx.fillStyle="#71839b";ctx.fillText("OLDEST",p.l,h-5);ctx.textAlign="right";ctx.fillText("LATEST",w-p.r,h-5);
}
function tickClock(){ $("clock").textContent=new Date().toLocaleTimeString([], {hour:"2-digit",minute:"2-digit"}); }
tickClock();setInterval(tickClock,15000);connectSocket();window.addEventListener("resize",drawChart);
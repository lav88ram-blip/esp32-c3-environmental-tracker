// Airscope dashboard. Publish the ThingSpeak channel, then enter its numeric ID below.
const DEFAULT_CHANNEL_ID = ""; // Example: "1234567"
const REFRESH_MS = 30000;
const $ = (id) => document.getElementById(id);
let channelId = DEFAULT_CHANNEL_ID;
let history = [];
let isLive = false;

function setStatus(live, message) {
  isLive = live;
  $("connectionStatus").textContent = message;
  $("statusDot").classList.toggle("live", live);
  $("deviceState").textContent = live ? "Online" : "Demo";
}
function showReading(temp, humidity, timestamp) {
  $("temperature").textContent = Number(temp).toFixed(1);
  $("fahrenheit").textContent = (Number(temp) * 9 / 5 + 32).toFixed(1) + " °F";
  $("humidity").textContent = Math.round(Number(humidity));
  $("humidityBar").style.width = Math.max(0, Math.min(100, Number(humidity))) + "%";
  const h = Number(humidity);
  $("humidityLabel").textContent = h < 30 ? "Dry air" : h > 60 ? "Humid air" : "Moderate";
  $("lastUpdated").textContent = timestamp ? "Last sample: " + new Date(timestamp).toLocaleTimeString([], {hour:"2-digit",minute:"2-digit",second:"2-digit"}) : "Demo sample";
}
function drawChart() {
  const canvas = $("historyChart");
  const ctx = canvas.getContext("2d");
  const rect = canvas.getBoundingClientRect();
  if (!rect.width || !rect.height) return;
  const dpr = window.devicePixelRatio || 1;
  canvas.width = Math.round(rect.width * dpr);
  canvas.height = Math.round(rect.height * dpr);
  ctx.scale(dpr, dpr);
  const w = rect.width, h = rect.height, pad = {l:32,r:12,t:14,b:25};
  ctx.clearRect(0,0,w,h);
  const points = history.slice(-24);
  $("chartEmpty").style.display = points.length ? "none" : "grid";
  if (!points.length) return;
  const valuesT = points.map(p => p.temp), valuesH = points.map(p => p.hum);
  const minT = Math.floor(Math.min(...valuesT)-2), maxT = Math.ceil(Math.max(...valuesT)+2);
  const minH = 0, maxH = 100;
  ctx.font = "10px DM Mono, monospace";
  ctx.textAlign = "right";
  for (let i=0;i<4;i++) {
    const y=pad.t+(h-pad.t-pad.b)*i/3;
    ctx.strokeStyle="#26364a";ctx.lineWidth=1;ctx.beginPath();ctx.moveTo(pad.l,y);ctx.lineTo(w-pad.r,y);ctx.stroke();
    ctx.fillStyle="#71839b";ctx.fillText(String(Math.round(maxT-(maxT-minT)*i/3)),pad.l-8,y+3);
  }
  const xAt=i=>pad.l+(w-pad.l-pad.r)*(points.length===1?.5:i/(points.length-1));
  const yAt=(v,min,max)=>pad.t+(h-pad.t-pad.b)*(1-(v-min)/(max-min||1));
  function line(vals,min,max,color) {
    if (vals.length===1) {ctx.fillStyle=color;ctx.beginPath();ctx.arc(xAt(0),yAt(vals[0],min,max),3,0,Math.PI*2);ctx.fill();return;}
    ctx.beginPath();vals.forEach((v,i)=>{const x=xAt(i),y=yAt(v,min,max);i?ctx.lineTo(x,y):ctx.moveTo(x,y)});ctx.strokeStyle=color;ctx.lineWidth=2;ctx.lineJoin="round";ctx.stroke();
  }
  line(valuesT,minT,maxT,"#70f0c0");
  // Humidity is normalized to the chart's vertical range to show its trend alongside temperature.
  line(valuesH.map(v=>minT+(v/100)*(maxT-minT)),minT,maxT,"#7aa9ff");
  ctx.textAlign="left";ctx.fillStyle="#71839b";ctx.fillText("OLDEST",pad.l,h-5);
  ctx.textAlign="right";ctx.fillText("LATEST",w-pad.r,h-5);
}
async function loadChannel() {
  if (!channelId || !/^\d+$/.test(channelId)) {
    setStatus(false, "Demo mode · add channel ID");
    history = [];
    showReading(23.4, 46, Date.now());
    $("readingCount").textContent = "Waiting for your ThingSpeak channel";
    drawChart();
    return;
  }
  try {
    const url = "https://api.thingspeak.com/channels/" + encodeURIComponent(channelId) + "/feeds.json?results=24";
    const response = await fetch(url, {cache:"no-store"});
    if (!response.ok) throw new Error("ThingSpeak returned " + response.status);
    const data = await response.json();
    const feeds = (data.feeds || []).filter(f => f.field1 !== null && f.field1 !== "" && f.field2 !== null && f.field2 !== "");
    if (!feeds.length) throw new Error("No readings yet. Check that fields 1 and 2 are enabled.");
    history = feeds.map(f => ({temp:Number(f.field1),hum:Number(f.field2),time:f.created_at})).filter(p=>Number.isFinite(p.temp)&&Number.isFinite(p.hum));
    const latest = history[history.length-1];
    showReading(latest.temp,latest.hum,latest.time);
    setStatus(true,"Live · ThingSpeak");
    $("readingCount").textContent = history.length + " readings loaded";
    drawChart();
  } catch (err) {
    setStatus(false,"Waiting for data");
    $("readingCount").textContent = err.message;
    $("chartEmpty").textContent = "Could not load readings. Confirm the channel is public and Fields 1 & 2 contain data.";
    $("chartEmpty").style.display = "grid";
  }
}
$("saveChannel").addEventListener("click",()=>{
  const value=$("channelInput").value.trim();
  if (!/^\d+$/.test(value)) { $("channelInput").focus(); $("channelInput").setCustomValidity("Enter the numeric ThingSpeak channel ID."); $("channelInput").reportValidity(); return; }
  $("channelInput").setCustomValidity("");
  channelId=value;
  try { localStorage.setItem("airscopeChannelId",channelId); } catch (_) {}
  loadChannel();
});
$("channelInput").addEventListener("input",()=> $("channelInput").setCustomValidity(""));
try { channelId=localStorage.getItem("airscopeChannelId") || DEFAULT_CHANNEL_ID; } catch (_) {}
if (channelId) $("channelInput").value=channelId;
function tickClock() { $("clock").textContent=new Date().toLocaleTimeString([], {hour:"2-digit",minute:"2-digit"}); }
tickClock();setInterval(tickClock,15000);
loadChannel();setInterval(loadChannel,REFRESH_MS);
window.addEventListener("resize",drawChart);
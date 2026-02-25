let map, marker, currentEspIp = null, pollingInterval = null;
const POLL_RATE_MS = 2000; 
let isFetching = false;

function initMap() {
    map = L.map('map').setView([32.0853, 34.7818], 13);
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '© OpenStreetMap'
    }).addTo(map);
    marker = L.marker([0, 0], { opacity: 0 }).addTo(map);
}

function connectToESP32() {
    const ipInput = document.getElementById('esp-ip');
    let ipValue = ipInput ? ipInput.value.trim() : '';

    if (!ipValue) {
        alert("Please enter IP");
        return;
    }

    let cleanIP = ipValue.replace(/^https?:\/\//, "").replace(/\/+$/, "");
    currentEspIp = "http://" + cleanIP;

    setStatusMessage('Connecting...');
    
    if (pollingInterval) clearInterval(pollingInterval);
    fetchStatus();
    pollingInterval = setInterval(() => { if (!isFetching) fetchStatus(); }, POLL_RATE_MS);
}

async function fetchStatus() {
    if (!currentEspIp || isFetching) return;
    isFetching = true;

    try {
        const res = await fetch(`${currentEspIp}/status`, { 
            mode: 'cors', // וידוא מצב CORS
            cache: 'no-store' 
        });

        if (!res.ok) throw new Error(`HTTP Error: ${res.status}`);

        const data = await res.json();
        
        setStatusMessage(data.status || 'Online');
        if (data.temp) document.getElementById("temp-val").textContent = data.temp + "°C";
        if (data.hum) document.getElementById("hum-val").textContent = data.hum + "%";
        
        if (data.lat && data.lng) {
            const pos = [data.lat, data.lng];
            marker.setLatLng(pos).setOpacity(1);
            map.setView(pos, 15);
        }
    } catch (err) {
        console.error("Fetch error:", err);
        setStatusMessage('Offline / Error', true);
    } finally {
        isFetching = false;
    }
}

async function sendDrive(cmd) {
    if (!currentEspIp) return;
    try {
        await fetch(`${currentEspIp}/drive?command=${cmd}`, { mode: 'cors' });
    } catch (err) {
        console.warn('Drive failed:', err);
    }
}

function setStatusMessage(msg, isError = false) {
    const el = document.getElementById("status-val");
    if (el) {
        el.textContent = msg;
        el.style.color = isError ? "#ff4444" : "#00ff00";
    }
}

document.addEventListener('DOMContentLoaded', () => {
    if (typeof L !== 'undefined') initMap();
});
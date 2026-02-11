// Global variables
let map;
let marker;
let ws;
let reconnectInterval = 3000;

// Initialize Leaflet Map
function initMap() {
    // Default position (0,0) with zoom level 2
    map = L.map('map').setView([0, 0], 2);

    // Add OpenStreetMap tiles
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '© OpenStreetMap contributors'
    }).addTo(map);

    // Initialize marker
    marker = L.marker([0, 0], {
        title: "Road-E location",
        opacity: 0 // Hide initially
    }).addTo(map);

    // Connect to WebSocket after map is ready
    connectToGPSWebSocket();
}

// UI Update Functions
function updateUIOnline() {
    const status = document.getElementById("status-val");
    if (status) {
        status.textContent = "Online";
        status.classList.remove("no-data");
    }
}

function updateUIOffline() {
    const status = document.getElementById("status-val");
    if (status) {
        status.textContent = "Offline";
        status.classList.add("no-data");
    }
}

function setStatusMessage(msg, isError = false) {
    const status = document.getElementById("status-val");
    if (status) {
        status.textContent = msg;
        if (isError) status.classList.add("no-data");
        else status.classList.remove("no-data");
    }
}

function showMapMessage(text) {
    const msg = document.getElementById("map-message");
    if (msg) {
        msg.style.display = "block";
        msg.textContent = text;
    }
}

function hideMapMessage() {
    const msg = document.getElementById("map-message");
    if (msg) msg.style.display = "none";
}

function updateGPS(lat, lng) {
    if (!map || !marker) return;
    const latNum = Number(lat);
    const lngNum = Number(lng);
    
    if (Number.isNaN(latNum) || Number.isNaN(lngNum)) return;
    
    const pos = [latNum, lngNum];
    
    marker.setLatLng(pos);
    marker.setOpacity(1); // Show marker
    map.setView(pos, 17); // Zoom in

    hideMapMessage();
    updateUIOnline();
}

function updateSensors(temp, hum) {
    if (temp !== undefined) {
        const el = document.getElementById("temp-val");
        if (el) el.textContent = temp;
    }
    if (hum !== undefined) {
        const el = document.getElementById("hum-val");
        if (el) el.textContent = hum;
    }
}

function updateStream(url) {
    const img = document.getElementById("stream");
    const ph = document.getElementById("stream-placeholder");
    
    if (url) {
        if (img) {
            img.src = url;
            img.style.display = "block";
        }
        if (ph) ph.style.display = "none";
    } else {
        if (img) img.style.display = "none";
        if (ph) ph.style.display = "block";
    }
}

// WebSocket Logic
function connectToGPSWebSocket(wsUrl) {
    const computedUrl = (() => {
        if (wsUrl) return wsUrl;
        if (location.protocol === 'file:') return null;
        const scheme = location.protocol === 'https:' ? 'wss:' : 'ws:';
        const host = location.host || location.hostname;
        return `${scheme}//${host}/ws`;
    })();

    let triedLocalhostFallback = false;
    let attemptUrl = wsUrl || computedUrl || 'ws://localhost:8765';

    if (location.protocol === 'file:' && !wsUrl) {
        console.warn('Serving page from file:// — WebSocket same-origin detection is not possible. Trying ws://localhost:8765');
        setStatusMessage('Connecting (file:// -> localhost)...', true);
    } else {
        setStatusMessage('Connecting...');
    }
    showMapMessage('Connecting to Road-E...');

    function tryConnect(url) {
        try {
            ws = new WebSocket(url);
        } catch (err) {
            console.error('WebSocket construction failed for', url, err);
            handleConnectFailure(url, err);
            return;
        }

        ws.onopen = () => {
            console.log('WebSocket connected to', url);
            setStatusMessage('Connected');
            hideMapMessage();
        };

        ws.onmessage = (evt) => {
            try {
                const data = JSON.parse(evt.data);
                if (data.lat !== undefined && data.lng !== undefined) updateGPS(data.lat, data.lng);
                if (data.temp !== undefined || data.hum !== undefined) updateSensors(data.temp, data.hum);
                if (data.streamUrl !== undefined) updateStream(data.streamUrl);
            } catch (err) {
                console.warn('Invalid WS message, not JSON:', err, evt.data);
            }
        };

        ws.onerror = (e) => {
            console.error('WebSocket error on', url, e);
            setStatusMessage('WebSocket error', true);
            showMapMessage('WebSocket error — check server (see console).');
        };

        ws.onclose = (e) => {
            console.log('WebSocket closed for', url, 'code:', e.code, 'reason:', e.reason);
            setStatusMessage('Disconnected', true);
            showMapMessage('Disconnected from Road-E — reconnecting...');
            
            if (!triedLocalhostFallback && url !== 'ws://localhost:8765') {
                triedLocalhostFallback = true;
                console.log('Trying localhost fallback ws://localhost:8765');
                setTimeout(() => tryConnect('ws://localhost:8765'), 800);
                return;
            }
            setTimeout(() => tryConnect(url), reconnectInterval);
        };
    }

    function handleConnectFailure(url, err) {
        console.warn('Connect failed for', url, err);
        if (!triedLocalhostFallback && url !== 'ws://localhost:8765') {
            triedLocalhostFallback = true;
            console.log('Falling back to ws://localhost:8765');
            tryConnect('ws://localhost:8765');
            return;
        }
        setTimeout(() => {
            tryConnect(url);
        }, reconnectInterval);
    }

    tryConnect(attemptUrl);
}

// Control Logic
async function sendDrive(cmd) {
    console.log("Sending command: " + cmd);
    // If we have a WebSocket connection, we could send it here
    if (ws && ws.readyState === WebSocket.OPEN) {
        try {
            ws.send(JSON.stringify({ type: 'drive', command: cmd }));
        } catch (err) {
            console.error("Error sending drive command", err);
        }
    } else {
        // Fallback
    }
}

// Start everything when the page loads
document.addEventListener('DOMContentLoaded', () => {
    // Check if Leaflet is loaded
    if (typeof L !== 'undefined') {
        initMap();
    } else {
        console.error('Leaflet library not loaded');
        showMapMessage('Error: Leaflet library not loaded.');
    }
});

// Expose globals for debugging
window.updateGPS = updateGPS;
window.updateSensors = updateSensors;
window.updateStream = updateStream;
window.connectToGPSWebSocket = connectToGPSWebSocket;
window.sendDrive = sendDrive;
window.initMap = initMap;
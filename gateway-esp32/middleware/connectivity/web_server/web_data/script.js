// --- Navigation Logic ---
const navDash = document.getElementById('nav-dash');
const navConfig = document.getElementById('nav-config');
const dashView = document.getElementById('dashboard-view');
const configView = document.getElementById('config-view');

function showView(view) {
    dashView.style.display = (view === 'dash') ? 'block' : 'none';
    configView.style.display = (view === 'config') ? 'block' : 'none';
    navDash.classList.toggle('active', view === 'dash');
    navConfig.classList.toggle('active', view === 'config');
}

navDash.onclick = () => showView('dash');
navConfig.onclick = () => showView('config');

// --- Data Polling Logic ---
const stateNames = ["IDLE", "INITIALIZING", "CONFIGURING", "MONITORING", "TESTING", "ERROR"];

async function updateStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();

        // Banner
        document.getElementById('system-state').innerText = stateNames[data.state] || "UNKNOWN";
        document.getElementById('system-subtext').innerText = data.sub_status;
        const banner = document.getElementById('system-banner');
        banner.classList.toggle('fault', data.fault_active);

        // Motion & Health
        document.getElementById('motion-state').innerText = data.motion_state;
        document.getElementById('vibration-val').innerText = data.vibration.toFixed(2);
        const healthEl = document.getElementById('ele-health');
        healthEl.innerText = data.ele_health;
        healthEl.className = 'value ' + (data.ele_health === 'GOOD' ? 'status-good' : 'status-bad');

        // Connectivity
        document.getElementById('wifi-status').innerText = data.wifi_connected ? `Connected (${data.wifi_rssi}dBm)` : "Disconnected";
        document.getElementById('cell-status').innerText = data.cell_connected ? `${data.cell_operator} (${data.cell_level}/4)` : "Searching...";
        document.getElementById('mqtt-status').innerText = data.mqtt_connected ? "Online" : "Offline";

        // Edge Node
        document.getElementById('edge-link').innerText = data.edge_connected ? "Connected" : "Disconnected";
        document.getElementById('edge-armed').innerText = data.edge_armed ? "Yes" : "No";
        document.getElementById('system-uptime').innerText = data.uptime;

    } catch (error) {
        console.error("Failed to fetch status:", error);
    }
}

// Initial update and periodic polling
updateStatus();
setInterval(updateStatus, 1000);

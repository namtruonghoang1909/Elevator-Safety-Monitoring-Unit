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
// Initial update and periodic polling
updateStatus();
setInterval(updateStatus, 1000);

// --- OTA Logic ---
const btnOta = document.getElementById('btn-ota');
const otaUrl = document.getElementById('ota-url');
const otaProgressContainer = document.getElementById('ota-progress-container');
const otaBar = document.getElementById('ota-bar');
const otaStatus = document.getElementById('ota-status');

btnOta.onclick = async () => {
    const url = otaUrl.value.trim();
    if (!url || !url.startsWith('http')) {
        alert("Please enter a valid HTTP/HTTPS URL");
        return;
    }

    if (!confirm("Are you sure you want to start the firmware update? The system will reboot upon completion.")) {
        return;
    }

    try {
        btnOta.disabled = true;
        btnOta.innerText = "Processing...";

        const response = await fetch('/api/ota', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ url: url })
        });

        if (response.ok) {
            otaProgressContainer.style.display = 'block';
            otaStatus.innerText = "OTA update request accepted. Starting download...";
        } else {
            const err = await response.text();
            alert("Failed to start OTA: " + err);
            btnOta.disabled = false;
            btnOta.innerText = "Start Update";
        }
    } catch (error) {
        alert("Error: " + error);
        btnOta.disabled = false;
        btnOta.innerText = "Start Update";
    }
};

function handleOtaProgress(progress) {
    if (progress >= 0) {
        otaProgressContainer.style.display = 'block';
        btnOta.disabled = true;
        btnOta.innerText = "Updating...";
        otaBar.style.width = progress + '%';

        if (progress < 100) {
            otaStatus.innerText = `Downloading and flashing: ${progress}%`;
        } else {
            otaStatus.innerText = "Update successful! New firmware will load on next restart.";
            btnOta.innerText = "Pending Restart";
            btnOta.classList.add('btn-success');
        }
    } else {
        // Only hide if we weren't just updating (e.g., initial load)
        if (btnOta.innerText === "Updating...") {
            otaStatus.innerText = "OTA update failed or aborted.";
            btnOta.disabled = false;
            btnOta.innerText = "Start Update";
        }
    }
}

// Modify updateStatus to call handleOtaProgress
async function updateStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();

        // ... existing banner and card updates ...
        document.getElementById('system-state').innerText = stateNames[data.state] || "UNKNOWN";
        document.getElementById('system-subtext').innerText = data.sub_status;
        const banner = document.getElementById('system-banner');
        banner.classList.toggle('fault', data.fault_active);

        document.getElementById('motion-state').innerText = data.motion_state;
        document.getElementById('vibration-val').innerText = data.vibration.toFixed(2);
        const healthEl = document.getElementById('ele-health');
        healthEl.innerText = data.ele_health;
        healthEl.className = 'value ' + (data.ele_health === 'GOOD' ? 'status-good' : 'status-bad');

        document.getElementById('wifi-status').innerText = data.wifi_connected ? `Connected (${data.wifi_rssi}dBm)` : "Disconnected";
        document.getElementById('cell-status').innerText = data.cell_connected ? `${data.cell_operator} (${data.cell_level}/4)` : "Searching...";
        document.getElementById('mqtt-status').innerText = data.mqtt_connected ? "Online" : "Offline";

        document.getElementById('edge-link').innerText = data.edge_connected ? "Connected" : "Disconnected";
        document.getElementById('edge-armed').innerText = data.edge_armed ? "Yes" : "No";
        document.getElementById('system-uptime').innerText = data.uptime;

        // OTA Progress
        handleOtaProgress(data.ota_progress);

    } catch (error) {
        console.error("Failed to fetch status:", error);
    }
}


// Initial update and periodic polling
updateStatus();
setInterval(updateStatus, 1000);

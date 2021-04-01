
var isWorking = false;

setInterval(getSensorInfo, 1000);

const humidity_elem = document.getElementById("humidity-placeholder");
const temperature_elem = document.getElementById("temperature-placeholder");
const validation_message_elem = document.getElementById("validation-message");
const success_message_elem = document.getElementById("success-message");
const status_elem = document.getElementById("status-placeholder");
const toggle_start_button_elem = document.getElementById("toggle-start-button");
const autostart_enable_elem = document.getElementById("autostart-enable");
const autostart_value_elem = document.getElementById("autostart-value");

async function getSensorInfo() {
    const sensor_info_raw = await fetch("/api/info");
    const sensor_info_json = await sensor_info_raw.json();

    humidity_elem.innerText = sensor_info_json.humidity;
    temperature_elem.innerText = sensor_info_json.temperature;

    isWorking = sensor_info_json.status;
    updateUiStatus();
}

async function save() {
    const autostart_enable = autostart_enable_elem.checked;
    const humidity_value = parseInt(autostart_value_elem.value);

    if (!Number.isInteger(humidity_value) || humidity_value < 1 || humidity_value > 100) {
        validation_message_elem.innerText = "Incorrect format. Range: 1 - 100%";
        success_message_elem.innerText = "";
    }
    else {
        const body = { autostart_enable: autostart_enable, autostart_value: humidity_value };

        await fetch("api/autostart", {
            method: 'POST',
            body: JSON.stringify(body)
        })

        validation_message_elem.innerText = "";
        success_message_elem.innerText = "Saved!";
    }
}

async function toggleStatus() {
    isWorking = !isWorking;

    if (isWorking)
        turnOnRequest();
    else
        turnOffRequest();

    updateUiStatus();
}

async function turnOnRequest() {
    await fetch("/api/turn_on", { method: "POST" });
}

async function turnOffRequest() {
    await fetch("/api/turn_off", { method: "POST" });
}

function updateUiStatus() {
    if (isWorking) {
        showWorkingStatus();
        showTurnOffBtn();
    }
    else {
        showStoppedStatus();
        showTurnOnBtn();
    }
}

function showWorkingStatus() {
    status_elem.innerText = "Working"
    status_elem.classList.remove("status-placeholder-stopped");
    status_elem.classList.add("status-placeholder-working");
}

function showStoppedStatus() {
    status_elem.innerText = "Stopped"
    status_elem.classList.remove("status-placeholder-working");
    status_elem.classList.add("status-placeholder-stopped");
}

function showTurnOnBtn() {
    toggle_start_button_elem.innerText = "Manual Start"
    toggle_start_button_elem.classList.remove("btn-turn-off");
    toggle_start_button_elem.classList.add("btn-turn-on");
}

function showTurnOffBtn() {
    toggle_start_button_elem.innerText = "Stop"
    toggle_start_button_elem.classList.remove("btn-turn-on");
    toggle_start_button_elem.classList.add("btn-turn-off");
}
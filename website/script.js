async function sendDrive(cmd) {
    console.log("Sending command: " + cmd);
    try {
    } catch (err) {
        console.error("Connection lost");
    }
}

async function refreshData() {
    try {
    } catch (err) {
        console.log("Waiting for server...");
    }
}

setInterval(refreshData, 3000);
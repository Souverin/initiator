#ifndef WEB_INDEX_H
#define WEB_INDEX_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ініціатор Підриву — Симулятор</title>
    <style>
        :root {
            --bg-color: #121212;
            --card-bg: #1e1e1e;
            --text-color: #e0e0e0;
            --safe-color: #2e7d32;
            --arming-color: #f57c00;
            --armed-color: #d32f2f;
            --fail-color: #c62828;
            --btn-disabled: #424242;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
        }
        .container {
            width: 100%;
            max-width: 480px;
            background: var(--card-bg);
            padding: 25px;
            border-radius: 12px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.5);
        }
        h2 { text-align: center; margin-top: 0; color: #fff; border-bottom: 1px solid #333; padding-bottom: 10px; }
        .status-box {
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            font-size: 1.2em;
            font-weight: bold;
            margin-bottom: 20px;
            background: #2a2a2a;
            border: 2px solid #444;
            transition: all 0.3s ease;
        }
        .timer-display {
            font-size: 2.5em;
            text-align: center;
            font-family: monospace;
            margin: 15px 0;
            color: #ffb74d;
        }
        .info-panel {
            font-size: 0.9em;
            margin-bottom: 20px;
            background: #262626;
            padding: 10px 15px;
            border-radius: 6px;
        }
        .info-row { display: flex; justify-content: space-between; margin: 5px 0; }
        .btn-group { display: flex; flex-direction: column; gap: 12px; }
        button {
            padding: 14px;
            font-size: 1.1em;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            color: white;
            transition: opacity 0.2s, transform 0.1s;
        }
        button:active { transform: scale(0.98); }
        button:disabled { background-color: var(--btn-disabled) !important; cursor: not-allowed; opacity: 0.5; }
        .btn-start { background-color: #1976d2; }
        .btn-stop { background-color: #757575; }
        .btn-fire { background-color: var(--armed-color); font-size: 1.3em; padding: 18px; }
        .btn-fault { background-color: #6a1b9a; margin-top: 15px; font-size: 0.85em; opacity: 0.8; }
        
        .state-SAFE { border-color: var(--safe-color); color: #81c784; }
        .state-ARMING { border-color: var(--arming-color); color: #ffb74d; }
        .state-ARMED { border-color: var(--armed-color); color: #ef5350; animation: pulse 1s infinite; }
        .state-FIRING { background: var(--armed-color); color: white; }
        .state-FAIL_SAFE { border-color: var(--fail-color); background: #3e1010; color: #ff8a80; }

        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.6; }
            100% { opacity: 1; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Ініціатор Підриву (Симулятор)</h2>

        <div id="statusBox" class="status-box state-SAFE">
            СТАН: <span id="stateText">INIT</span>
        </div>

        <div id="timerDisplay" class="timer-display">00:00</div>

        <div class="info-panel">
            <div class="info-row"><span>Напруга живлення:</span><strong id="voltageVal">0.00 V</strong></div>
            <div class="info-row"><span>Причина Fail-Safe:</span><strong id="failReasonVal">NONE</strong></div>
        </div>

        <div class="btn-group">
            <button id="btnStart" class="btn-start" onclick="sendCommand('start')">START (Зведення)</button>
            <button id="btnStop" class="btn-stop" onclick="sendCommand('stop')">STOP (Запобіжник)</button>
            <button id="btnFire" class="btn-fire" onclick="sendCommand('fire')" disabled>ПІДРИВ / FIRE</button>
            <button class="btn-fault" onclick="injectFault()">Тест: Симуляція Збою (Fail-Safe)</button>
        </div>
    </div>

    <script>
        async function updateStatus() {
            try {
                const response = await fetch('/api/status');
                if (!response.ok) throw new Error('HTTP status ' + response.status);
                
                const data = await response.json();

                const state = data.state;
                const stateText = document.getElementById('stateText');
                const statusBox = document.getElementById('statusBox');
                const timerDisplay = document.getElementById('timerDisplay');

                stateText.innerText = state;
                statusBox.className = 'status-box state-' + state;

                document.getElementById('voltageVal').innerText = Number(data.voltage).toFixed(2) + ' V';
                document.getElementById('failReasonVal').innerText = data.fail_reason;

                if (state === 'ARMING') {
                    const totalSec = Math.ceil(data.arming_time_ms / 1000);
                    const min = Math.floor(totalSec / 60);
                    const sec = totalSec % 60;
                    timerDisplay.innerText = `${String(min).padStart(2, '0')}:${String(sec).padStart(2, '0')}`;
                } else if (state === 'ARMED') {
                    timerDisplay.innerText = "ГОТОВИЙ";
                } else if (state === 'FIRING') {
                    timerDisplay.innerText = "СПРАЦЮВАННЯ!";
                } else {
                    timerDisplay.innerText = "00:00";
                }

                document.getElementById('btnStart').disabled = (state !== 'SAFE');
                document.getElementById('btnStop').disabled = (state !== 'ARMING' && state !== 'ARMED');
                document.getElementById('btnFire').disabled = (state !== 'ARMED');

            } catch (err) {
                console.error("[JS LOG] Помилка отримання статусу:", err);
            }
        }

        async function sendCommand(cmd) {
            console.log("[JS LOG] Sending command:", cmd);
            try {
                const res = await fetch('/api/cmd', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: 'cmd=' + encodeURIComponent(cmd)
                });
                
                const resText = await res.text();
                console.log(`[JS LOG] Command response [${res.status}]:`, resText);

                if (!res.ok) {
                    alert("Команду відхилено сервером: " + resText);
                }
                updateStatus();
            } catch (err) {
                console.error("[JS LOG] Помилка відправки команди:", err);
                alert("Помилка зв'язку з сервером: " + err);
            }
        }

        async function injectFault() {
            if (confirm("Згенерувати тестову помилку напруги/датчика?")) {
                console.log("[JS LOG] Injecting test fault...");
                try {
                    const res = await fetch('/api/test/fault', { method: 'POST' });
                    const resText = await res.text();
                    console.log(`[JS LOG] Fault inject response [${res.status}]:`, resText);
                    updateStatus();
                } catch (err) {
                    console.error("[JS LOG] Помилка інжекції збою:", err);
                }
            }
        }

        setInterval(updateStatus, 500);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INDEX_H
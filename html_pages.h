#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char* CONNECT_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>M5WeatherStation — WiFi Setup</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .card {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 32px;
            padding: 40px 32px;
            max-width: 450px;
            width: 100%;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
            animation: fadeInUp 0.6s ease-out;
        }

        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .header {
            text-align: center;
            margin-bottom: 32px;
        }

        .weather-icon {
            font-size: 64px;
            margin-bottom: 12px;
            display: inline-block;
            animation: float 3s ease-in-out infinite;
        }

        @keyframes float {
            0%, 100% { transform: translateY(0px); }
            50% { transform: translateY(-10px); }
        }

        h1 {
            font-size: 28px;
            font-weight: 700;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            background-clip: text;
            color: transparent;
            margin-bottom: 8px;
        }

        .subtitle {
            color: #666;
            font-size: 14px;
        }

        .input-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: #333;
            font-size: 14px;
        }

        input {
            width: 100%;
            padding: 14px 16px;
            border: 2px solid #e2e8f0;
            border-radius: 16px;
            font-size: 16px;
            transition: all 0.3s;
            background: white;
        }

        input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }

        button {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 16px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            margin-top: 12px;
        }

        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px -5px rgba(102, 126, 234, 0.4);
        }

        button:active {
            transform: translateY(0);
        }

        .status {
            margin-top: 20px;
            padding: 12px;
            border-radius: 12px;
            text-align: center;
            font-size: 13px;
            display: none;
        }

        .status.loading {
            display: block;
            background: #e0f2fe;
            color: #0369a1;
        }

        .status.success {
            display: block;
            background: #dcfce7;
            color: #166534;
        }

        .status.error {
            display: block;
            background: #fee2e2;
            color: #991b1b;
        }

        .spinner {
            display: inline-block;
            width: 16px;
            height: 16px;
            border: 2px solid #e2e8f0;
            border-top: 2px solid #667eea;
            border-radius: 50%;
            animation: spin 0.8s linear infinite;
            margin-right: 8px;
            vertical-align: middle;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .footer {
            text-align: center;
            margin-top: 24px;
            font-size: 12px;
            color: #94a3b8;
        }
    </style>
</head>
<body>
    <div class="card">
        <div class="header">
            <div class="weather-icon">🌤️</div>
            <h1>M5WeatherStation</h1>
            <div class="subtitle">Настройка WiFi подключения</div>
        </div>

        <form id="wifiForm">
            <div class="input-group">
                <label>📡 Имя сети (SSID)</label>
                <input type="text" id="ssid" placeholder="Например: MyHomeWiFi" required autocomplete="off">
            </div>

            <div class="input-group">
                <label>🔒 Пароль</label>
                <input type="password" id="password" placeholder="••••••••" required>
            </div>

            <button type="submit">Подключиться →</button>
        </form>

        <div id="status" class="status"></div>
        <div class="footer">После подключения вы сможете выбрать город</div>
    </div>

    <script>
        const form = document.getElementById('wifiForm');
        const statusDiv = document.getElementById('status');

        form.addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            
            statusDiv.className = 'status loading';
            statusDiv.innerHTML = '<div class="spinner"></div> Подключение к WiFi...';
            
            try {
                const response = await fetch('/connect', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(password)}`
                });
                
                const text = await response.text();
                
                if (text.includes('✅')) {
                    statusDiv.className = 'status success';
                    statusDiv.innerHTML = '✅ Подключено! Устройство перезагружается...';
                    setTimeout(() => {
                        window.location.href = 'http://192.168.4.1/setup';
                    }, 2000);
                } else {
                    statusDiv.className = 'status error';
                    statusDiv.innerHTML = '❌ Не удалось подключиться. Проверьте пароль.';
                }
            } catch (error) {
                statusDiv.className = 'status error';
                statusDiv.innerHTML = '❌ Ошибка соединения. Попробуйте снова.';
            }
        });
    </script>
</body>
</html>
)rawliteral";

const char* SETUP_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>M5WeatherStation — Weather Settings</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            background: linear-gradient(135deg, #0f172a 0%, #1e293b 50%, #0f172a 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 32px;
            padding: 40px 32px;
            max-width: 480px;
            width: 100%;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
            animation: fadeInUp 0.6s ease-out;
        }

        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .header {
            text-align: center;
            margin-bottom: 32px;
        }

        .weather-icon {
            font-size: 56px;
            margin-bottom: 12px;
            display: inline-block;
        }

        h1 {
            font-size: 26px;
            font-weight: 700;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            background-clip: text;
            color: transparent;
            margin-bottom: 6px;
        }

        .subtitle {
            color: #666;
            font-size: 13px;
        }

        .preview-card {
            background: linear-gradient(135deg, #667eea15 0%, #764ba215 100%);
            border-radius: 20px;
            padding: 24px;
            text-align: center;
            margin-bottom: 28px;
            border: 1px solid rgba(102, 126, 234, 0.2);
        }

        .preview-temp {
            font-size: 48px;
            font-weight: 700;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            background-clip: text;
            color: transparent;
        }

        .preview-desc {
            color: #555;
            margin-top: 8px;
            font-size: 14px;
        }

        .input-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: #333;
            font-size: 14px;
        }

        input, select {
            width: 100%;
            padding: 14px 16px;
            border: 2px solid #e2e8f0;
            border-radius: 16px;
            font-size: 16px;
            transition: all 0.3s;
            background: white;
            font-family: inherit;
        }

        input:focus, select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }

        button {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 16px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            margin-top: 8px;
        }

        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px -5px rgba(102, 126, 234, 0.4);
        }

        .preview-btn {
            background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
            margin-bottom: 12px;
        }

        .preview-btn:hover {
            box-shadow: 0 10px 20px -5px rgba(59, 130, 246, 0.4);
        }

        .city-examples {
            display: flex;
            gap: 8px;
            margin-top: 8px;
            flex-wrap: wrap;
        }

        .city-chip {
            background: #f1f5f9;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 11px;
            color: #475569;
            cursor: pointer;
            transition: all 0.2s;
        }

        .city-chip:hover {
            background: #667eea;
            color: white;
        }

        .footer {
            text-align: center;
            margin-top: 24px;
            font-size: 11px;
            color: #94a3b8;
        }

        .spinner {
            display: inline-block;
            width: 14px;
            height: 14px;
            border: 2px solid #e2e8f0;
            border-top: 2px solid #667eea;
            border-radius: 50%;
            animation: spin 0.8s linear infinite;
            margin-right: 6px;
            vertical-align: middle;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .status {
            margin-top: 16px;
            padding: 10px;
            border-radius: 12px;
            text-align: center;
            font-size: 12px;
            display: none;
        }

        .status.show {
            display: block;
        }

        .status.success {
            background: #dcfce7;
            color: #166534;
        }

        .status.error {
            background: #fee2e2;
            color: #991b1b;
        }
    </style>
</head>
<body>
    <div class="card">
        <div class="header">
            <div class="weather-icon">🌍</div>
            <h1>Настройки погоды</h1>
            <div class="subtitle">Выберите город для отображения</div>
        </div>

        <div class="preview-card" id="previewCard">
            <div class="preview-temp" id="previewTemp">--°C</div>
            <div class="preview-desc" id="previewDesc">🌤️ Нажмите "Предпросмотр"</div>
        </div>

        <form id="settingsForm">
            <div class="input-group">
                <label>🏙️ Город (на английском)</label>
                <input type="text" id="city" placeholder="Например: Moscow, London, Tokyo" value="Moscow">
                <div class="city-examples">
                    <span class="city-chip" onclick="setCity('Moscow')">Moscow</span>
                    <span class="city-chip" onclick="setCity('London')">London</span>
                    <span class="city-chip" onclick="setCity('New York')">New York</span>
                    <span class="city-chip" onclick="setCity('Tokyo')">Tokyo</span>
                    <span class="city-chip" onclick="setCity('Paris')">Paris</span>
                </div>
            </div>

            <div class="input-group">
                <label>📏 Единицы измерения</label>
                <select id="units">
                    <option value="metric" selected>°C, метры в секунду</option>
                    <option value="imperial">°F, мили в час</option>
                </select>
            </div>

            <button type="button" class="preview-btn" onclick="previewWeather()">👁️ Предпросмотр погоды</button>
            <button type="submit">💾 Сохранить и перезагрузить</button>
        </form>

        <div id="status" class="status"></div>
        <div class="footer">Данные погоды предоставлены wttr.in</div>
    </div>

    <script>
        async function previewWeather() {
            const city = document.getElementById('city').value;
            const units = document.getElementById('units').value;
            
            if (!city) {
                showStatus('Введите название города', 'error');
                return;
            }
            
            document.getElementById('previewTemp').innerHTML = '<span class="spinner"></span> Загрузка...';
            document.getElementById('previewDesc').innerHTML = 'Получение данных...';
            
            try {
                const url = `http://wttr.in/${city}?format=%t+%C&${units === 'metric' ? 'm' : 'u'}`;
                const response = await fetch(url);
                const text = await response.text();
                
                const parts = text.trim().split(' ');
                const temp = parts[0];
                const condition = parts.slice(1).join(' ');
                
                document.getElementById('previewTemp').innerHTML = temp;
                document.getElementById('previewDesc').innerHTML = condition || '☁️ ' + city;
                showStatus('✅ Данные получены', 'success');
            } catch (error) {
                document.getElementById('previewTemp').innerHTML = '--°C';
                document.getElementById('previewDesc').innerHTML = '❌ Ошибка получения';
                showStatus('❌ Не удалось получить погоду', 'error');
            }
        }
        
        function setCity(city) {
            document.getElementById('city').value = city;
            previewWeather();
        }
        
        function showStatus(message, type) {
            const statusDiv = document.getElementById('status');
            statusDiv.className = `status show ${type}`;
            statusDiv.innerHTML = message;
            setTimeout(() => {
                statusDiv.className = 'status';
            }, 3000);
        }
        
        document.getElementById('settingsForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const city = document.getElementById('city').value;
            const units = document.getElementById('units').value;
            
            const statusDiv = document.getElementById('status');
            statusDiv.className = 'status show';
            statusDiv.innerHTML = '<span class="spinner"></span> Сохранение настроек...';
            
            try {
                const response = await fetch('/save', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `city=${encodeURIComponent(city)}&units=${units}`
                });
                
                const text = await response.text();
                
                if (text.includes('✅')) {
                    statusDiv.className = 'status show success';
                    statusDiv.innerHTML = '✅ Настройки сохранены! Перезагрузка...';
                    setTimeout(() => {
                        window.location.href = '/';
                    }, 2000);
                } else {
                    statusDiv.className = 'status show error';
                    statusDiv.innerHTML = '❌ Ошибка сохранения';
                }
            } catch (error) {
                statusDiv.className = 'status show error';
                statusDiv.innerHTML = '❌ Ошибка соединения';
            }
        });
    </script>
</body>
</html>
)rawliteral";

#endif
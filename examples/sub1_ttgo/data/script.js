var modal = document.getElementById("myModal");
var btn = document.getElementById("myBtn");
var span = document.getElementsByClassName("close")[0];
var ipCheckInterval; 
btn.onclick = function () {
  modal.style.display = "block";
};

span.onclick = function () {
  modal.style.display = "none";
};

window.onclick = function (event) {
  if (event.target == modal) {
    modal.style.display = "none";
  }
};
const menuItems = document.querySelectorAll(".menu-item");
const slides = document.querySelectorAll(".slide");
menuItems.forEach((item) => {
  item.addEventListener("click", function () {
    menuItems.forEach((i) => i.classList.remove("active"));
    slides.forEach((slide) => slide.classList.remove("active"));
    this.classList.add("active");
    const subsystem = this.getAttribute("data-subsystem");
    document.getElementById(`slide-${subsystem}`).classList.add("active");
  });
});

document.querySelector(".menu-item").classList.add("active");
document.querySelector("#slide-1").classList.add("active");
function updateConnectionStatus() {
  console.log("Оновлення статусу підключення..."); 
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/connection-status", true);
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      var response = JSON.parse(xhr.responseText);
      const statusDiv = document.getElementById("connection-status");
      const btn = document.getElementById("myBtn");

      console.log("Статус відповіді:", response.status); 

      if (response.status === "connected") {
        statusDiv.textContent = "Успішно підключено до Wi-Fi.";
        statusDiv.className = "connection-status connected";
        btn.disabled = true; 
        btn.textContent = "Вже підключено"; 
        btn.style.backgroundColor = "gray"; 
        btn.style.color = "white"; 
      } else {
        statusDiv.textContent = "Відсутнє підключення до Wi-Fi.";
        statusDiv.className = "connection-status disconnected";
      }
    }
  };
  xhr.send();
}

function checkForNewIP() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/get-new-ip", true);
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      var response = JSON.parse(xhr.responseText);
      console.log("IP-адреса відповіді:", response.ip);
      if (response.ip !== "not_connected") {
        window.location.href = "http://" + response.ip; 
      } else {
        console.log("Не вдалося отримати нову IP-адресу."); 
      }
    }
  };
  xhr.send();
}

document.getElementById("wifiForm").onsubmit = function (event) {
  event.preventDefault();
  var form = document.getElementById("wifiForm");
  var formData = new FormData(form);

  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/setwifi", true);
  xhr.onreadystatechange = function () {
    if (xhr.readyState == 4 && xhr.status == 200) {
      updateConnectionStatus();
      ipCheckInterval = setInterval(checkForNewIP, 5000);
    }
  };
  xhr.send(formData);
};
document.addEventListener("DOMContentLoaded", () => {
  const fireTemp = document.getElementById("fireTemp");
  const firePres = document.getElementById("firePres");
  const fireHum = document.getElementById("fireHum");
  const fireMq2 = document.getElementById("fireMq2");

  const motionDetected = document.getElementById("motionDetected");
  const motionTime = document.getElementById("motionTime");

  const accessLastSuccess = document.getElementById("accessLastSuccess");
  const accessTotalAttempts = document.getElementById("accessTotalAttempts");

  const alarmStatus = document.getElementById("alarm-status-6");

  const connectionStatus3 = document.getElementById("connection-status-3");
  const connectionStatus4 = document.getElementById("connection-status-4");
  const connectionStatus5 = document.getElementById("connection-status-5");
  const connectionStatus6 = document.getElementById("connection-status-6");

  function fetchData() {
    fetch("/get-all-data")
      .then(response => response.json())
      .then(data => {
        fireTemp.textContent = `${data.fire.temp.toFixed(2)}°C`;
        firePres.textContent = `${data.fire.pres.toFixed(2)} hPa`;
        fireHum.textContent = `${data.fire.hum.toFixed(2)}%`;
        fireMq2.textContent = `${data.fire.mq2}ppm`;
        motionDetected.textContent = `${data.motion.motionDetected ? "Виявлено" : "Не виявлено"}`;
        motionTime.textContent = `${data.motion.timeDetected}`;
        accessLastSuccess.textContent = `${data.access.lastAttemptSuccess ? "Успішна" : "Неуспішна"}`;
        accessTotalAttempts.textContent = `${data.access.totalAttempts}`;
        alarmStatus.textContent = `${data.alert.activeAlert ? "Так" : "Ні"}`;

        if (connectionStatus3) {
          connectionStatus3.textContent = data.status.subsystem3 ? "Підключено" : "Немає зв'язку";
        }
        if (connectionStatus4) {
          connectionStatus4.textContent = data.status.subsystem4 ? "Підключено" : "Немає зв'язку";
        }
        if (connectionStatus5) {
          connectionStatus5.textContent = data.status.subsystem5 ? "Підключено" : "Немає зв'язку";
        }
        if (connectionStatus6) {
          connectionStatus6.textContent = data.status.subsystem6 ? "Підключено" : "Немає зв'язку";
        }
      })
      .catch(error => console.error("Error fetching data:", error));
  }
  setInterval(fetchData, 2000);
  fetchData();
});
const buttonDoor = document.getElementById("toggleDoorButton");
const message = document.getElementById("responseMessage");
buttonDoor.addEventListener("click", () => {
  if (buttonDoor.textContent.includes("Відкрити двері")) {
    const password = prompt("Введіть пароль для відкриття дверей:");
    if (password) {
      fetch("/toggle-door-password", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `password=${encodeURIComponent(password)}`
      })
        .then(response => response.text())
        .then(data => {
          message.textContent = data;
          if (data.includes("відкрито")) {
            buttonDoor.textContent = "Закрити двері";
          }
        })
        .catch(error => {
          message.textContent = "Помилка: Не вдалося виконати дію.";
          console.error("Error:", error);
        });
    }
  } else {
    fetch("/toggle-door-password", { method: "POST" })
      .then(response => response.text())
      .then(data => {
        message.textContent = data;
        if (data.includes("закрито")) {
          buttonDoor.textContent = "Відкрити двері";
        }
      })
      .catch(error => {
        message.textContent = "Помилка: Не вдалося виконати дію.";
        console.error("Error:", error);
      });
  }
});

const buttonAlarm = document.getElementById("testAlarmButton");
const alarmStatus = document.getElementById("alarmStatus");
buttonAlarm.addEventListener("click", () => {
  fetch("/test-alarm", { method: "POST" })
    .then(response => response.text())
    .then(data => {
      alarmStatus.textContent = data;
      alarmStatus.classList.remove("hidden");
      setTimeout(() => {
        alarmStatus.classList.add("hidden");
      }, 3000);
    })
    .catch(error => {
      console.error("Error:", error);
      alarmStatus.textContent = "Помилка активації сигналізації!";
      alarmStatus.classList.remove("hidden");

      setTimeout(() => {
        alarmStatus.classList.add("hidden");
      }, 3000);
    });
});
let cam_ip;
async function getConnectionStatus() {
  try {
    const response = await fetch('/connection-status-sub2');
    if (!response.ok) {
      throw new Error(`HTTP помилка: ${response.status}`);
    }
    const data = await response.text();
    console.log("Отримані дані:", data);
    const [status, camIp] = data.split("|");
    cam_ip = camIp;
    const statusElement = document.getElementById('connection-status-2');
    if (statusElement) {
      statusElement.textContent = status || "Немає даних";
    } else {
      console.error("Елемент з ID 'connection-status-2' не знайдено");
    }

    if (camIp) {
      console.log(`IP камери: ${camIp}`);
    }
  } catch (error) {
    console.error('Помилка:', error);

    const statusElement = document.getElementById('connection-status-2');
    if (statusElement) {
      statusElement.textContent = "Помилка";
    }
  }
}
setInterval(getConnectionStatus, 2000);
function logAction(action) {
  const actionLog = document.getElementById('action-log');
  const logItem = document.createElement('li');
  logItem.textContent = action;
  actionLog.appendChild(logItem);
}

function captureImage() {
  fetch('/capture-image')
    .then(response => response.text())
    .then(() => {

      const img = document.getElementById('capturedImage');
      img.src = '/photo.jpg?' + new Date().getTime(); 
      img.style.display = 'block';
      const downloadLink = document.getElementById('downloadLink');
      downloadLink.href = img.src;
      logAction('Фото зроблено та збережено');
    })
    .catch(error => {
      console.error('Помилка:', error);
      logAction('Помилка при захопленні фото: ' + error.message);
    });
}
async function toggleLED() {
  const buttonElement = document.getElementById('toggleLEDButton');
  const iconElement = buttonElement.querySelector('i');

  try {
    const response = await fetch('/toggle-led');
    if (!response.ok) {
      throw new Error(`HTTP помилка: ${response.status}`);
    }
    const result = await response.text();

    if (buttonElement.textContent.includes('Вимкнути')) {
      buttonElement.textContent = 'Увімкнути LED-спалах';
      buttonElement.appendChild(iconElement); 
      iconElement.classList.remove('fa-lightbulb'); 
      iconElement.classList.add('fa-lightbulb-o'); 
      logAction('LED стан: LED вимкнено');
    } else {
      buttonElement.textContent = 'Вимкнути LED-спалах';
      buttonElement.appendChild(iconElement); 
      iconElement.classList.remove('fa-lightbulb-o'); 
      iconElement.classList.add('fa-lightbulb'); 
      logAction('LED стан: LED увімкнено');
    }
  } catch (error) {
    logAction('Помилка при зміні стану LED: ' + error.message);
  }
}

async function startStreaming() {
  const response = await fetch('/start-stream');
  const result = await response.text();
  alert(result);
  if (result === "Streaming started")
    logAction('Стрімінг розпочато');
  document.getElementById('videoStream').style.display = 'block';
  document.getElementById('videoStream').src = `http://${cam_ip}/stream`;
}

async function stopStreaming() {
  const response = await fetch('/stop-stream');
  const result = await response.text();
  if (result === "Streaming stopped")
    logAction('Стрімінг зупинено');
  document.getElementById('videoStream').style.display = 'none';
  document.getElementById('videoStream').src = '';
}
let selectedEffect = 0;

function selectEffect(element) {
  document.querySelectorAll('.effect').forEach(effect => {
    effect.classList.remove('active');
  });
  element.classList.add('active');
  selectedEffect = element.getAttribute('data-effect');
  console.log(`Вибрано ефект: ${selectedEffect}`);
}
async function sendSettings() {
  const form = document.getElementById('camera-settings-form');
  const formData = new FormData(form);
  formData.append('effect', selectedEffect);

  const settings = {};
  formData.forEach((value, key) => {
    settings[key] = value;
  });

  try {
    const response = await fetch('/update-settings', {
      method: 'POST',
      body: JSON.stringify(settings),
      headers: {
        'Content-Type': 'application/json'
      }
    });

    if (!response.ok) {
      throw new Error(`HTTP помилка: ${response.status}`);
    }

    const result = await response.text();
    logAction('Налаштування збережено: ' + result);
  } catch (error) {
    logAction('Помилка при збереженні налаштувань: ' + error.message);
  }
}
document.addEventListener("DOMContentLoaded", () => {
  const volumeSlider = document.getElementById("volume-slider");
  const volumeValue = document.getElementById("volume-value");
  volumeSlider.addEventListener("input", () => {
    const currentVolume = volumeSlider.value;
    volumeValue.textContent = currentVolume;
  });
  volumeSlider.addEventListener("change", () => {
    const currentVolume = volumeSlider.value;
    sendVolumeToCentralModule(currentVolume);
  });
});

/**
 * Надсилання рівня гучності на центральний модуль
 * @param {number} volume - Обраний рівень гучності
 */
async function sendVolumeToCentralModule(volume) {
  try {
    const response = await fetch("/update-volume", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ volume }),
    });

    if (!response.ok) {
      throw new Error(`HTTP помилка: ${response.status}`);
    }

    const result = await response.text();
    console.log(`Гучність оновлено: ${result}`);
  } catch (error) {
    console.log(`Помилка при оновленні гучності: ${error.message}`);
  }
}
const togglePassword = document.getElementById("togglePassword");
const newPasswordInput = document.getElementById("newPassword");

togglePassword.addEventListener("click", () => {
  const isPasswordVisible = newPasswordInput.type === "text";
  newPasswordInput.type = isPasswordVisible ? "password" : "text";
  togglePassword.classList.toggle("fa-eye", isPasswordVisible); 
  togglePassword.classList.toggle("fa-eye-slash", !isPasswordVisible); 
  togglePassword.classList.toggle("active", !isPasswordVisible);
});

const changePasswordButton = document.getElementById("changePasswordButton");
changePasswordButton.addEventListener("click", () => {
  const oldPassword = prompt("Введіть поточний пароль:");
  if (!oldPassword) {
    alert("Старий пароль обов'язковий для зміни.");
    return;
  }

  const newPassword = newPasswordInput.value.trim();
  if (!newPassword) {
    alert("Будь ласка, введіть новий пароль.");
    return;
  }

  if (newPassword.length < 6) {
    alert("Пароль повинен містити щонайменше 6 символів.");
    return;
  }
  fetch("/change-password", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ oldPassword, newPassword }),
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error(response.status === 403 ? "Невірний старий пароль!" : `Помилка: ${response.status}`);
      }
      return response.text();
    })
    .then((data) => {
      alert(data); 
      newPasswordInput.value = ""; 
    })
    .catch((error) => {
      console.error("Помилка при зміні пароля:", error);
      alert(error.message || "Не вдалося змінити пароль. Спробуйте пізніше.");
    });
});
updateConnectionStatus();

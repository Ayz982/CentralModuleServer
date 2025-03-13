#include <CentralModuleServer.h>

CentralModuleServer centralServer(80);

// Завдання для обробки запитів веб-сервера
void handleServerTask(void *param) {
    while (true) {
        centralServer.handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Затримка для запобігання зайвому завантаженню CPU
    }
}

// Завдання для запитів даних від підлеглого модуля
void requestDataTask(void *param) {
    while (true) {
        centralServer.requestDataFromSlave();
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Запит кожну секунду
    }
}

void setup() {
    // Ініціалізація веб-сервера
    centralServer.begin();

    // Запуск завдань на різних ядрах
    xTaskCreatePinnedToCore(handleServerTask,   // Вказівник на функцію завдання
                            "HandleServer",    // Ім'я завдання
                            4096,              // Розмір стеку завдання
                            NULL,              // Параметри для передачі у завдання
                            1,                 // Пріоритет завдання
                            NULL,              // Ідентифікатор завдання
                            0);                // Ядро 0

    xTaskCreatePinnedToCore(requestDataTask,
                            "RequestData",
                            4096,
                            NULL,
                            1,
                            NULL,
                            1);                // Ядро 1
}

void loop() {
    // Нічого не потрібно тут виконувати, оскільки всі функції працюють у завданнях
}

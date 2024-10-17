# Logger
Dynamic library for logging system calls. File I/O: open,close,lseek, read,write mem mgmt: malloc, free, realloc

# Установка (Linux)
У Вас должны быть установлены [зависимости проекта](https://github.com/IzmaylovI/Logger#Зависимости_проекта)


1. Клонирование репозитория `git clone https://github.com/IzmaylovI/Logger.git`
2. Сконфигурировать проект в директории `build` с помощью команды `cmake -B build `
3. Перейти в директорию, где сконфигурировался проект и настроить сборку с помощью команды `make`

В ранее указанной директории, указанной с помощью ключа `-B` будет создана динамическая библиотека с переопределнными функциями-обертками системных вызовов `liblog.so`

# Зависимости_проекта
1. cmake версии не ниже 3.10

# Решение задачи
![image](https://github.com/user-attachments/assets/ba6cfa66-3eab-487e-9f4e-44a6888c8b91)
![image](https://github.com/user-attachments/assets/fe86af70-1138-4e57-b5db-ff33186287d1)



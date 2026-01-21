
python -m venv .venv

call .venv\Scripts\activate.bat

pip install west

west init -l tools
west update

west zephyr-export

pip install -r zephyr-rtos\scripts\requirements.txt

cd zephyr-rtos
git am ..\patch\1_mec5_all.patch
git am ..\patch\2_mec5_uart.patch
cd ..

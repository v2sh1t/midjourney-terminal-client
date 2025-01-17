@echo off
echo 配置代理...
set http_proxy=http://127.0.0.1:7890
set https_proxy=http://127.0.0.1:7890
git config --global http.proxy https://127.0.0.1:7890
git config --global https.proxy https://127.0.0.1:7890

echo 正在创建build目录...
if not exist "build" mkdir build

echo 进入build目录...
cd build

echo 配置CMake项目...
cmake -G "MinGW Makefiles" ..

echo 编译项目...
cmake --build .

echo 完成！
echo 可执行文件位于 build/midjourney_cli.exe

cd ..
pause

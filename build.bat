@echo off
echo 配置git代理...
git config --global url."https://gh.tryxd.cn/https://github.com/".insteadOf "https://github.com/"

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
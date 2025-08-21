MapUnlocker 插件 (C++ 版, 适配 ArkApi v3.54)
===============================================

安装步骤:
1. 将编译好的 MapUnlocker.dll 放入 ArkApi/Plugins/MapUnlocker/ 目录
2. 将 config.json 放在同一目录
3. 重启服务器

功能:
- 玩家必须击败前一张地图的 BOSS 并获得 Trophy，才能传送进入下一张地图
- 顺序: The Island → Scorched Earth → Aberration → Extinction → Genesis → Genesis: Part 2
- 解锁 Genesis2 后, 玩家可自由进入其他支线地图

编译方式 (自动):
- 上传整个工程到 GitHub
- GitHub → Actions → 会自动运行 build.yml, 产出 MapUnlocker.dll
- 下载 dll, 放入服务器即可

联系方式:
- 使用过程中遇到问题, 可以查看 ShooterGame.log 日志排查

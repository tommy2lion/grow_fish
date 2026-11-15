# grow_fish

大鱼吃小鱼课程设计项目（C 语言风格 + EasyX 图形库）。

## 构建与运行

使用 Visual Studio 2022 打开 `grow_fish.sln`：

1. 选择 `Debug | x64` 或 `Release | x64`；
2. 按 F5 运行。

命令行构建：

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  src\grow_fish.vcxproj /p:Configuration=Debug /p:Platform=x64
```

构建输出位于 `build\<Platform>\<Configuration>\grow_fish.exe`。

## 操作说明

- 登录界面输入英文/数字玩家名，按 Enter 进入；
- 方向键或 WASD 移动；
- 吃小鱼加分并变大，躲避大鱼（碰撞扣生命）和炸弹（减时间）；
- 吃闹钟增加时间；
- P 暂停/继续，ESC 结束本局；
- 达到目标分过关，时间耗尽或生命耗尽则游戏结束；
- 成绩保存到运行目录下的 `records.txt`，排行榜显示前 3 名。

## 目录

| 路径 | 说明 |
| --- | --- |
| `src/` | 源码（main / game / fish / player / render / input / data） |
| `docs/` | 课程设计指导书、需求文档、学习文档、报告模板 |
| `tools/md2pdf/` | Markdown 转 PDF 工具 |
| `refer/` | 参考资料（已 gitignore） |
| `build/` | 构建输出（已 gitignore） |
| `spacex/` | md2pdf 输出（已 gitignore） |

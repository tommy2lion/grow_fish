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

- 启动后自动进入无边框全屏；登录/菜单界面按 ESC 退出程序；
- 内部按 1024×720 设计，全屏时自动缩放到当前屏幕分辨率；
- 登录界面输入英文/数字玩家名，按 Enter 进入；
- 方向键 / WASD 移动，或按住鼠标左键让玩家鱼游向鼠标；
- 吃小鱼加分并变大，躲避大鱼（碰撞扣 1 生命）和炸弹；
- **时间模式**：吃闹钟 +5 秒，炸弹 -5 秒，60 秒内达到目标分过关；
- **生命模式**：无时间限制，闹钟 +1 生命，炸弹 -1 生命，生命耗尽前达到目标分过关；
- 菜单界面按 M 切换时间/生命模式，按 S 开关音效；
- 游戏中按 **Ctrl+B** 开关 BUG 模式：大鱼不造成伤害、计时冻结、不触发达标/超时结束；仍可手动 ESC 或点【结束】退出；
- P 暂停/继续，ESC 结束本局；
- 每局结束写 `last_game.txt`；正分写入 `records.txt`，排行榜显示前 5 名；
- 鱼游到边缘时可能掉头，伴随 whoosh 音效；
- 吃鱼、撞鱼、炸弹、闹钟、胜负均有音效，另有水下环境音。

## 目录

| 路径 | 说明 |
| --- | --- |
| `src/` | 源码（main / game / fish / player / render / input / data / assets / sound） |
| `assets/` | Kenney CC0 鱼类素材、程序化生成的音效 |
| `docs/` | 课程设计指导书、需求文档、学习文档、报告模板、设计文档 |
| `tools/md2pdf/` | Markdown 转 PDF 工具 |
| `tools/gen_sounds.py` | 音效生成脚本（标准库，原创 CC0） |
| `refer/` | 参考资料（已 gitignore） |
| `build/` | 构建输出（已 gitignore） |
| `spacex/` | md2pdf 输出（已 gitignore） |

素材授权：鱼类/水草/气泡来自 Kenney Fish Pack 2.0（CC0），详见 `assets/README.md` 和 `assets/fish/License.txt`。

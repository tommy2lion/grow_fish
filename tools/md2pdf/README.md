# md2pdf — Markdown 转 PDF

把仓库里的 `.md` 文档转成便于分发/打印的 PDF(中文排版友好)。
链路是 **md → 自包含 HTML → headless 浏览器 print-to-pdf**。

## 用法

本仓库根目录已备好虚拟环境 `.venv`(含 `markdown` 依赖),可直接运行:

```powershell
.\.venv\Scripts\python.exe tools\md2pdf\md2pdf.py "docs\20261115202211-黄金矿工需求文档.md"
```

也可以先进入工具目录:

```powershell
cd tools\md2pdf
..\..\.venv\Scripts\python.exe md2pdf.py "..\..\docs\20261115202211-黄金矿工需求文档.md"
```

**md 路径相对当前工作目录**,所以 `cd tools/md2pdf` 之后写 `../../docs/...` 即可。

一次转多个:

```powershell
python md2pdf.py ../../docs/a.md ../../docs/b.md
```

`.md` 后缀可省略(`xxx` 会自动补成 `xxx.md`)。

## 输出位置

缺省**中间 HTML 和最终 PDF 都落在 `<repo>/spacex/_pdf/`**,文件名取源文件的 stem:

```
spacex/_pdf/20261115202211-黄金矿工需求文档.html   ← 自包含,可直接分享
spacex/_pdf/20261115202211-黄金矿工需求文档.pdf
```

- 该路径**相对仓库根**解析(靠脚本自身位置向上找 `.git`),所以从哪个目录调用都一致。
- `spacex/` 整体 gitignore ⇒ **产物永远不会污染版本库**,也不会弄脏 `docs/` 树。
- 换目录:`-o / --out-dir`

```powershell
python md2pdf.py ../../docs/a.md -o ./out
```

退出码:全部成功为 `0`,有失败为 `1`(可用于脚本串联)。

## 依赖

| 依赖 | 说明 |
|---|---|
| `markdown` | `pip install markdown`(唯一的 Python 第三方依赖) |
| Edge 或 Chrome | 用其 headless 模式打印 PDF。脚本按 `BROWSERS` 列表顺序自动探测,找不到会明确报错 —— 装在别处就把路径加进那个列表 |

本仓库根目录的 `.venv/` 已安装上述依赖,直接使用即可;`.venv/` 已被 gitignore。

## 支持的内容

- **表格 / 代码块 / 引用 / TOC** —— markdown 扩展 `tables`、`fenced_code`、`sane_lists`、`toc`
- **本地图片自动内嵌** —— `png/jpg/jpeg/gif/svg` 转 base64 塞进 HTML,**相对 md 自身目录**解析,
  所以产出的 HTML 是自包含的,单独发给别人也能看
- **mermaid 图** —— ```` ```mermaid ```` 代码块由 mermaid.js 渲染
  (⚠️ 这一项走 CDN,**需要联网**;纯文本文档不受影响)
- **中文排版** —— `Microsoft YaHei` 字体栈 + A4 页边距,长表格自动换行

## 改样式

排版全在 `md2pdf.py` 顶部的 `CSS` 常量里(字号、页边距、表格、代码块配色),直接改即可。

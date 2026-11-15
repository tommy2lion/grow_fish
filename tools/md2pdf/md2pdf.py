"""md -> self-contained HTML -> PDF (via MS Edge / Chrome headless print-to-pdf).

Embeds images as base64, renders ```mermaid``` via mermaid.js, CJK-friendly CSS.

用法(md 路径相对**当前工作目录**):
    cd tools/md2pdf
    python md2pdf.py ../../docs/20261115202211-黄金矿工需求文档.md

输出(中间 HTML + 最终 PDF)缺省都落在 <repo>/spacex/_pdf/ —— 该目录整体 gitignore,
产物不会污染版本库。用 -o 可另指目录。详见同目录 README.md。
"""
from __future__ import annotations

import argparse
import base64
import html
import pathlib
import re
import subprocess
import sys

try:
    import markdown
except ImportError:
    sys.exit("missing dependency: pip install markdown")

# 候选浏览器(headless print-to-pdf),按序取第一个存在的
BROWSERS = [
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
]

CSS = """
@page { size: A4; margin: 16mm 14mm; }
* { box-sizing: border-box; }
body { font-family: "Microsoft YaHei","Segoe UI",-apple-system,Arial,sans-serif;
       font-size: 12px; line-height: 1.6; color: #1f2937; }
h1 { font-size: 22px; border-bottom: 2px solid #e5e7eb; padding-bottom: 6px; margin: 18px 0 12px; }
h2 { font-size: 18px; border-bottom: 1px solid #eee; padding-bottom: 4px; margin: 20px 0 10px; }
h3 { font-size: 15px; margin: 16px 0 8px; color: #111827; }
h4 { font-size: 13px; margin: 12px 0 6px; }
p, li { margin: 4px 0; }
code { font-family: ui-monospace,Consolas,Menlo,monospace; background: #f3f4f6;
       padding: 1px 4px; border-radius: 3px; font-size: 11px; }
pre { background: #f6f8fa; border: 1px solid #e5e7eb; border-radius: 6px;
      padding: 10px; overflow-x: auto; font-size: 10.5px; white-space: pre-wrap; word-break: break-word; }
pre code { background: none; padding: 0; }
pre.mermaid { background: #fff; border: none; text-align: center; }
table { border-collapse: collapse; width: 100%; margin: 8px 0; font-size: 11px; }
th, td { border: 1px solid #d1d5db; padding: 4px 8px; text-align: left; vertical-align: top; }
th { background: #f3f4f6; font-weight: 600; }
tr:nth-child(even) td { background: #fafafa; }
blockquote { border-left: 3px solid #cbd5e1; margin: 8px 0; padding: 2px 12px;
             color: #475569; background: #f8fafc; }
img { max-width: 100%; height: auto; border: 1px solid #e5e7eb; border-radius: 4px; margin: 6px 0; }
hr { border: none; border-top: 1px solid #e5e7eb; margin: 16px 0; }
a { color: #2563eb; text-decoration: none; }
"""


def repo_root() -> pathlib.Path:
    """从本脚本向上找 .git 定位仓库根;找不到则退回 tools/md2pdf 的上两级。

    ★ 必须相对**脚本自身**而不是 CWD —— 否则从任意目录调用时缺省输出会跑偏。
    """
    here = pathlib.Path(__file__).resolve()
    for parent in here.parents:
        if (parent / ".git").exists():
            return parent
    return here.parents[2]


def find_browser() -> str:
    for exe in BROWSERS:
        if pathlib.Path(exe).is_file():
            return exe
    sys.exit("no Edge/Chrome found - add your browser path to BROWSERS in md2pdf.py")


def render_html(md_path: pathlib.Path) -> tuple[str, int]:
    """md -> 自包含 HTML 字符串。返回 (html, mermaid 块数)。"""
    text = md_path.read_text(encoding="utf-8")

    # stash mermaid blocks before markdown processing
    blocks: list[str] = []

    def _stash(m):
        blocks.append(m.group(1))
        return f"\n\nMERMAIDSTASH{len(blocks) - 1}ENDSTASH\n\n"

    text = re.sub(r"```mermaid\n(.*?)```", _stash, text, flags=re.S)

    body = markdown.markdown(
        text, extensions=["tables", "fenced_code", "sane_lists", "toc"]
    )

    for i, code in enumerate(blocks):
        body = body.replace(
            f"<p>MERMAIDSTASH{i}ENDSTASH</p>",
            f'<pre class="mermaid">{html.escape(code)}</pre>',
        )

    # inline-embed local images as base64(相对 md 自身所在目录解析)
    def _embed(m):
        src = m.group(1)
        p = (md_path.parent / src).resolve()
        if p.exists() and p.suffix.lower() in (".png", ".jpg", ".jpeg", ".gif", ".svg"):
            b64 = base64.b64encode(p.read_bytes()).decode()
            ext = "svg+xml" if p.suffix.lower() == ".svg" else p.suffix.lstrip(".")
            return f'src="data:image/{ext};base64,{b64}"'
        return m.group(0)

    body = re.sub(r'src="([^"]+)"', _embed, body)

    mermaid_js = (
        "<script src='https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js'></script>"
        "<script>mermaid.initialize({startOnLoad:true,theme:'neutral'});</script>"
        if blocks else ""
    )
    return (
        f"<!doctype html><html lang='zh-CN'><head><meta charset='utf-8'>"
        f"<style>{CSS}</style>{mermaid_js}</head><body>{body}</body></html>",
        len(blocks),
    )


def convert(arg: str, out_dir: pathlib.Path, browser: str) -> bool:
    # ★ md 路径按**当前工作目录**解析 —— 所以 cd tools/md2pdf 后可写 ../../docs/...
    md_path = pathlib.Path(arg)
    if md_path.suffix.lower() != ".md":
        md_path = md_path.with_suffix(".md")
    if not md_path.is_file():
        print(f"skip (not found): {arg}", file=sys.stderr)
        return False

    full, n_mermaid = render_html(md_path)

    out_dir.mkdir(parents=True, exist_ok=True)
    html_path = out_dir / f"{md_path.stem}.html"
    pdf_path = out_dir / f"{md_path.stem}.pdf"
    html_path.write_text(full, encoding="utf-8")

    subprocess.run(
        [browser, "--headless=new", "--disable-gpu", "--no-pdf-header-footer",
         f"--print-to-pdf={pdf_path.resolve()}",
         "--virtual-time-budget=20000",
         html_path.resolve().as_uri()],
        check=True, timeout=120,
    )
    if not pdf_path.is_file():
        print(f"FAILED (browser produced no PDF): {md_path}", file=sys.stderr)
        return False
    print(f"OK  {pdf_path}  ({pdf_path.stat().st_size // 1024} KB, mermaid={n_mermaid})")
    return True


def main() -> int:
    default_out = repo_root() / "spacex" / "_pdf"
    ap = argparse.ArgumentParser(
        description="Markdown -> PDF (self-contained HTML + headless browser print)",
        epilog="e.g. cd tools/md2pdf && python md2pdf.py ../../docs/20261115202211-黄金矿工需求文档.md",
    )
    ap.add_argument("files", nargs="+", help="md file paths, relative to the current directory")
    ap.add_argument("-o", "--out-dir", default=None,
                    help=f"output dir for HTML + PDF (default: {default_out})")
    args = ap.parse_args()

    out_dir = pathlib.Path(args.out_dir) if args.out_dir else default_out
    browser = find_browser()

    ok = sum(convert(f, out_dir, browser) for f in args.files)
    print(f"done {ok}/{len(args.files)}", file=sys.stderr)
    return 0 if ok == len(args.files) else 1


if __name__ == "__main__":
    sys.exit(main())

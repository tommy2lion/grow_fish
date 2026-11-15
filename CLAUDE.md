# CLAUDE.md

本文件为 Claude Code 在本仓库工作时的指导说明。

## 项目简介

gold_miner（黄金矿工）课程设计项目。`docs/` 目录存放课程设计指导书、实验报告及相关文档。

## 文档命名规范

- 新建文档统一命名为：`YYYYMMDDHHMMSS-文档实际内容.md`
  - 时间前缀为文档创建时刻，共 14 位数字（年 4 位、月 2 位、日 2 位、时 2 位、分 2 位、秒 2 位）。
  - 示例：`20261115194530-黄金矿工需求分析.md`
- 缺省保存位置为 `docs/` 目录；用户明确指定其他位置时除外。

## 代码提交规范

- 每次提交使用简洁的英文 commit 信息，不超过 30 个单词。
- 采用祈使语气概括本次改动。
- 按改动类型添加前缀：

| 分类 | 前缀 | 用途 |
| --- | --- | --- |
| 文档类 | `docs: ` | 文档新增或修改（如 `docs/` 下的需求、学习、报告文档） |
| 文档类 | `chore: ` | 杂项事务（配置、依赖、清理等） |
| 代码类 | `framework: ` | 框架、基础结构与公共设施相关改动 |
| 代码类 | `feature_YYYY: ` | 功能开发相关改动（`YYYY` 为功能标识或年份，按实际填写） |

- 示例：
  - `docs: split course design guide into three docs`
  - `chore: add build configuration`
  - `framework: add game main loop skeleton`
  - `feature_2026: add claw launch and collision detection`

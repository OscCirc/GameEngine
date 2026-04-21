# GameEngine 文档编撰标准与规范

为了保证本项目在无限期的迭代和重构中，文档能够保持清晰、结构化且具备历史追溯能力，特制定以下标准。

## 1. 核心原则
- **记录“Why”大于“How”**：永远优先解释“为什么采用现在的架构”（比如为什么需要工厂函数解耦），然后才是“怎么实现的”。
- **单向时间线**：主目录（`00_Overview`, `01_Core` 等）只能反映**当前 Git Commit 对应的最新代码状态**。
- **文档即代码 (Docs as Code)**：所有文档更改必须随着代码更改一起 Commit，确保版本对齐。

## 2. 格式规范
- 全量使用 **Markdown**。
- 重点逻辑和生命周期使用 **Mermaid** 流程图绘制。
- 在描述复杂类关系时，尽量以 UML 类图辅助。
- 所有文档 (.md) 和代码文件都必须统一使用 **UTF-8** 编码。

## 3. 重构处理策略 (Architectural Decision Records - ADR)
- **小重构**：直接更新当前文档，并在文档末尾开辟 `## 架构演进记录` 追加演进史。
- **大重构（被完全废弃或重写）**：
  1. 将旧版 `.md` 原封不动移入 `99_Archive` 文件夹。
  2. 在 `99_Logs_and_Refactors` 中新建一篇 ADR（重构决策记录），说明旧版哪里不好、新版好在哪里。
  3. 在原目录下新建一份与当前系统匹配的全新文档。

## 4. 代码锚点 (Code Anchors)
当遇到极其复杂的 Hack 手法，或者由于第三方库版本变更导致的调整，在文档中需注明当时的 Git Commit Hash 或教程对应的集数。

## 5. Git 提交规范 (Commit Convention)
我们规定本项目遵循 [Conventional Commits](https://www.conventionalcommits.org/zh-hans/v1.0.0/) （约定式提交标准），保持项目的 Git 提交日志干净且高度语义化：

*   **`feat:`** 新增特性/系统（例如：引入 EventSystem、完成 Renderer2D 基础）。
*   **`fix:`**  修复 Bug 与崩溃（例如：修复 spdlog 重名注册异常、解决 dllimport 与 static inline 冲突）。
*   **`docs:`** 单纯的文档改动（例如：更新 ADR 记录、补充引擎 API 手册）。
*   **`refactor:`** 架构重构（没有新功能也没有修复 Bug，纯粹的代码结构重组，引擎中后期极速激增的类别）。
*   **`chore:`** 构建系统与包管理改动（例如：更新 vcxproj 配置、引入 vcpkg 包、升级 Premake 脚本）。
*   **`style:`** 代码格式化（不影响运行逻辑的空格、缩进调整）。

> **最佳实践要求**：保持提交的「**原子性**」。不可以在一次名为 `update` 的提交中，既改了渲染器代码，又修了 UI 的 Bug，还顺手改了文档。请分拆为 `feat: xxx`，`fix: xxx` 和 `docs: xxx`。
# 日志系统 (Log System)

引擎内置了一套轻量、高性能、高度可配置的控制台日志系统。

## 1. 架构选型
- **底层依赖**：[spdlog](https://github.com/gabime/spdlog) （极速的 C++ 日志库）。
- **设计模式**：单例化 / 静态类管理。
- **输出策略**：在控制台中带彩色高亮（`stdout_color_mt`），并且通过 `set_pattern("%^[%T] %n: %v%$")` 格式化了打印前缀。

## 2. 为什么需要两个 Logger？
由于采用引擎/沙盒客户端的双层架构，日志输出必须能够清楚地区分“是谁在说话”：
*   **引擎级 (Core Logger)**：名为 `Engine`。当引擎内部初始化窗口、编译着色器、报错时，日志会以此开头。
*   **客户端级 (App Logger)**：名为 `App`。沙盒游戏中自定义逻辑的调试输出。

通过这种严格区分的渠道，哪怕成百上千条日志滚屏，开发者也能快速锁定 Bug 到底是属于引擎架构，还是因为自己写的游戏逻辑出错了。

## 3. 宏定义的运用与注意事项
在 `Log.h` 文件末尾，引擎专门提供了一系列的宏来包装对 `GetLogger()` 的直接调用，如：

```cpp
#define GE_CORE_WARN(...) ::GE::Log::GetCoreLogger()->warn(__VA_ARGS__)
```

### 重点踩坑 (Gotcha)：函数调用的括号
在这个宏定义中，`GetCoreLogger()` 必须带上参数括号 `()`，因为它是**静态函数**的执行。
如果在宏定义中漏掉了括号（写成 `GetCoreLogger->warn()`），预处理器进行文本替换时就会试图对一个函数的“内存入口地址”使用指针解引用 `->`。
结果会导致极其晦涩的报错：`error C2227: “->warn”的左边必须指向类/结构/联合/泛型类型`。

### 高阶优化：剥离发行版日志 (Log Stripping in Dist Build)
为了确保极限性能与防止引擎底层信息泄露给玩家，在发布最终的 **“发行版(Dist Build)”** 时，所有的日志宏会被重新定义为空。
通过条件编译指令 `#ifdef GE_BUILD_DIST` 控制：
在非发行版（如 Debug、Release）：宏会展开为完整的 `spdlog` 调用。
在最终发行版（Dist）：`#define GE_CORE_INFO(...)` 后面什么也不会跟。编译器执行到预处理这一阶段时直接将其抹去，达成**零开销（Zero Overhead）的极限性能优化**。

## 4. 架构决策记录 (ADR)：DLL导出遇上 C++17 inline

### 背景与问题
在初次实现中，利用 C++17 特性尝试在 `.h` 头文件中初始化日志实例：
`inline static std::shared_ptr<spdlog::logger> s_CoreLogger;`
但在项目（生成为 DLL）被使用 `__declspec(dllimport)` 导入到 Sandbox 沙盒中时，发生了惨烈的编译失败。

### 为什么会失败？
这里存在无法调和的矛盾：
- `__declspec(dllimport)` 指示编译器：“不要在这个作用域里分配内存，这个静态变量存在于外部的 DLL 中”。
- `inline` 却强烈指示：“请就在这里，为这个变量开辟内存并原地定义”。
矛盾导致 MSVC 完全抛弃了对 `s_CoreLogger` 对应数据类型的解析，Sandbox 的编译器将它当作一个垃圾指针来处理。

### 解决方案
不要在包含 `dllimport` 宏逻辑的头文件里用 `inline` 去初始化带有状态数据的静态变量。
回归到传统的实现机制：
1. **Log.h (声明)**：`static std::shared_ptr<spdlog::logger> s_CoreLogger;`
2. **Log.cpp (定义)**：交回给当前模块（DLL 工程）自己开辟内存空间， `std::shared_ptr<spdlog::logger> Log::s_CoreLogger;`。
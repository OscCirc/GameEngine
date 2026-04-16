# 入口点与生命周期 (EntryPoint & Lifecycle)

## 1. 隐藏的 EntryPoint (入口点)

为了在全平台统一初始化与结束逻辑，引擎将原本应用层的 `int main()` 函数隐藏并封装在了引擎内部的 `core/EntryPoint.h` 中。

```cpp
// EntryPoint.h 伪代码
int main(int argc, char** argv)
{
    // 1. 初始化引擎子系统 (如 Log、Window 等)
    
    // 2. 调用客户端实现的工厂函数，索要一个具体的 Application 实例
    auto app = GE::CreateApplication();
    
    // 3. 开始游戏主循环
    app->run();
    
    // 4. 游戏结束，系统清理
    delete app;
}
```
**为什么这样做？**
1. 引擎可以在客户端逻辑执行前，强行插入内存分配器初始化、日志库初始化等底层逻辑。
2. 即使是在不同的操作系统（Windows 平台 WinMain, Linux 平台 main），客户端代码都无需改动，引擎会在平台相关的入口点中处理好适配。

## 2. 客户端的责任：工厂函数
引擎在 `main` 执行期间，对即将运行的是什么游戏一无所知。为了拿到真正的游戏实例，引擎通过暴露一个在外部定义的工厂函数来向客户端“要”对象：

```cpp
// 由客户端去实现它
extern GE::Application* GE::CreateApplication();
```

客户端（如 `SandboxApp.cpp`）只需要做两件事：
1. 继承基类：`class Sandbox : public GE::Application`
2. 实现函数并交出自己：`return new Sandbox();`

这完成了引擎驱动客户端逻辑的闭环。
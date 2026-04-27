# Window Abstraction

## 1. 为什么需要 Window 抽象 (Why)

在不同操作系统（Windows, macOS, Linux）上创建窗口和呈现画面的底层 API 差异巨大（例如 Windows 使用 Win32 API，Linux 使用 X11 或 Wayland）。如果我们直接把操作系统提供的底层代码塞进 `Application` 甚至各处渲染逻辑中，引擎将完全失去跨平台能力。

因此，我们需要设计一层**窗口系统抽象层 (Window Abstraction)**：
1. **统一 API**：对引擎其余部分提供一致的方法来获取窗口的高、宽，或者订阅窗口产生的系统事件（例如按键、鼠标操作）。
2. **隐藏实现细节**：对于开发者和引擎的其他模块，不关心底层是使用 `GLFW`、`SDL` 还是手写的 `Win32`。
3. **渲染上下文的载体**：图形渲染 API (如 OpenGL、Vulkan) 需要一个平台环境即「Context」才能在屏幕上画出东西，而这些通常和窗口是强绑定的。

**为什么选择 GLFW**：
对于桌面级应用程序来说，**GLFW** 是一个极其成熟、轻量且天然支持多平台的 C 库。它的主要职责就是“打开窗口，处理简单的键鼠输入，并创建适用于 OpenGL 等图形库的 Context（渲染上下文）”。这样我们就可以避免从头手撸浩如烟海且难以维护的跨平台原生窗口脚手架代​​码。

## 2. 核心架构与设计 (How)

### `GE::Window` 接口类

`GE::Window` 规定了引擎中任何平台下的 Window 必须具备的行为，作为一个纯接口（C++ 抽象基类）存在。
主要方法包括：
*   **状态获取**：`GetWidth()`, `GetHeight()`
*   **事件分发对接**：`SetEventCallback(...)` 返回窗口底层的各类事件给 `Application` 的事件总线处理。
*   **垂直同步控制**：`SetVSync()`, `IsSync()`
*   **创建入口**：`static Window* Create(...)` —— 这是一个**工厂方法**，根据编译时确定的操作系统宏自动返回适配该系统的窗口实例。

### `GE::WindowsWindow` 实现类

这是针对 Windows 平台的 `Window` 派生类。
在这个类中，我们实际上引入了 `GLFW` 的相关代码：
*   使用 `glfwInit()` 和 `glfwCreateWindow()` 进行核心配置。
*   在 `OnUpdate()` 时通过调用 `glfwPollEvents()` 来拉取操作系统派发给窗口的消息。
*   将由于窗口回调（例如尺寸更改、关闭请求）触发的 GLFW 响应包装成游戏引擎自定的 `Event` 结构，发回给 `EventCallbackFn` (主要交给并由 `Application` 进行主事件处理)。

## 3. 生命周期与工作流

基于目前的实现，其流转生命周期大体如下：

```mermaid
sequenceDiagram
    participant App as Application
    participant Win as WindowsWindow
    participant GLFW

    App->>Win: Window::Create() 请求实例化
    Win->>GLFW: glfwInit() 初始化库 (如果尚未初始化)
    Win->>GLFW: glfwCreateWindow(...)
    GLFW-->>Win: 返回 GLFWwindow 指针
    Win-->>App: 返还包装好的 Window 智能指针

    loop Every Frame
        App->>Win: OnUpdate()
        Win->>GLFW: glfwPollEvents() (拉取并处理系统消息)
        Win->>GLFW: glfwSwapBuffers() (后续实现)
    end
```

## 4. GLFW 事件分发

窗口层是引擎的**事件源**。GLFW 原生只提供一组 C 风格回调（`glfwSetWindowCloseCallback` / `glfwSetKeyCallback` / `glfwSetMouseButtonCallback` / `glfwSetScrollCallback` / `glfwSetCursorPosCallback` / `glfwSetWindowSizeCallback`），我们需要将它们包装成引擎统一的 `Event` 对象，再通过 `EventCallbackFn` 投递给上层（`Application`）。

### 为什么需要 `glfwSetWindowUserPointer`

GLFW 的回调函数签名是纯 C 函数指针，**不能捕获 lambda 的外部变量**，也就意味着回调内部没有办法直接访问 `WindowsWindow` 的成员（比如 `EventCallback`、`Width`、`Height`）。

为解决这个问题，我们使用了 GLFW 提供的「用户指针」机制：
1. 在 `WindowsWindow::Init()` 中调用 `glfwSetWindowUserPointer(m_Window, &m_Data)`，把一个包含引擎状态（含事件回调）的结构体指针挂在 `GLFWwindow` 上。
2. 每一个回调 lambda 内部再通过 `glfwGetWindowUserPointer(window)` 取回这份 `WindowData`，从而拿到 `EventCallback`。

```cpp
struct WindowData
{
    std::string Title;
    unsigned int Width, Height;
    bool VSync;
    EventCallbackFn EventCallback; // Application 注入
};
```

### 回调到 Event 的映射

| GLFW 回调 | 包装出的引擎事件 |
|-----------|------------------|
| `glfwSetWindowSizeCallback`   | `WindowResizeEvent(width, height)` |
| `glfwSetWindowCloseCallback`  | `WindowCloseEvent` |
| `glfwSetKeyCallback`          | `KeyPressedEvent` / `KeyReleasedEvent`（`GLFW_REPEAT` 作为 `repeatCount=1` 合并进 Pressed） |
| `glfwSetMouseButtonCallback`  | `MouseButtonPressedEvent` / `MouseButtonReleasedEvent` |
| `glfwSetScrollCallback`       | `MouseScrolledEvent(xOffset, yOffset)` |
| `glfwSetCursorPosCallback`    | `MouseMovedEvent(xPos, yPos)` |

此外我们还注册了 `glfwSetErrorCallback`，把 GLFW 的错误码和描述转发到引擎日志（`GE_CORE_ERROR`），方便在 GLFW 初始化/运行失败时定位问题。

### 完整分发链路

```mermaid
sequenceDiagram
    participant OS as 操作系统
    participant GLFW
    participant Win as WindowsWindow (回调 lambda)
    participant App as Application::OnEvent
    participant Disp as EventDispatcher

    OS->>GLFW: 键盘/鼠标/窗口消息
    GLFW->>Win: 触发 C 风格回调
    Win->>Win: glfwGetWindowUserPointer() 取回 WindowData
    Win->>Win: new XxxEvent(...) 包装
    Win->>App: data.EventCallback(event)
    App->>Disp: EventDispatcher(e).Dispatch<T>(handler)
    Disp-->>App: 匹配到 WindowCloseEvent → m_Running = false
```

> `Application` 通过 `m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent))` 把自身的成员函数绑定进来，窗口侧对上层零依赖，只认一个 `std::function<void(Event&)>`。

## 架构演进记录

- **[feat] Wire GLFW callbacks to engine event dispatch**：窗口层正式接入事件系统。在 `WindowsWindow::Init()` 中注册 6 类 GLFW 回调，统一借助 `glfwSetWindowUserPointer` 回取 `WindowData` 并转发为引擎 `Event`；`Application` 通过 `SetEventCallback` 订阅，并以 `EventDispatcher` 消费 `WindowCloseEvent` 以实现正常退出。同步增加 `glfwSetErrorCallback` 以便诊断。
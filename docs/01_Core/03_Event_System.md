# 事件系统 (Event System)

引擎采用了一套基于类型的、同步阻塞式的事件分发系统，用于解耦各子系统之间的通信。

## 1. 为什么需要事件系统？

在游戏引擎中，窗口、输入、渲染等子系统之间存在大量的交互需求。如果让它们直接互相调用，会导致严重的耦合问题。事件系统充当了一个中间层：

- **解耦发送者与接收者**：产生事件的模块不需要知道谁在监听。
- **统一接口**：所有事件共享同一套基类与分发机制，新增事件类型的成本极低。
- **分类过滤**：通过位掩码 (Bitmask) 实现事件类别过滤，接收方可以快速判断自己是否关心某个事件。

## 2. 架构设计

### 核心组件

```
Event (抽象基类)
├── ApplicationEvent.h  → WindowResizeEvent, WindowCloseEvent, AppTickEvent, AppUpdateEvent, AppRenderEvent
├── KeyEvent.h          → KeyPressedEvent, KeyReleasedEvent
└── MouseEvent.h        → MouseMovedEvent, MouseScrolledEvent, MouseButtonPressedEvent, MouseButtonReleasedEvent

EventDispatcher (分发器)
```

### 类型系统

事件通过两个维度进行标识：

1. **EventType (枚举)**：精确标识每一种事件，用于 `EventDispatcher` 的类型匹配。
2. **EventCategory (位掩码)**：标识事件所属的大类，用于快速过滤。

```cpp
enum EventCategory
{
    None = 0,
    EventCategoryApplication  = BIT(0),
    EventCategoryInput        = BIT(1),
    EventCategoryKeyboard     = BIT(2),
    EventCategoryMouse        = BIT(3),
    EventCategoryMouseButton  = BIT(4)
};
```

> 一个事件可以同时属于多个类别。例如 `KeyPressedEvent` 同时属于 `EventCategoryKeyboard | EventCategoryInput`。

### 宏辅助系统

为了避免每个具体事件类都重复编写大量样板代码，引擎提供了两个辅助宏：

- **`EVENT_CLASS_TYPE(type)`**：自动生成 `GetStaticType()`、`GetEventType()`、`GetName()` 三个函数。
- **`EVENT_CLASS_CATEGORY(category)`**：自动生成 `GetCategoryFlags()` 函数。

这使得定义一个新事件类只需要关注其自身的数据字段和 `ToString()` 方法。

## 3. EventDispatcher 的工作原理

`EventDispatcher` 是事件系统的核心分发机制，采用**编译期类型匹配**的策略：

```cpp
template<typename T>
bool Dispatch(EventFn<T> func)
{
    if (m_Event.GetEventType() == T::GetStaticType())
    {
        m_Event.m_Handled = func(static_cast<T&>(m_Event));
        return true;
    }
    return false;
}
```

**工作流程**：
1. 调用方将一个通用的 `Event&` 引用传入 `EventDispatcher`。
2. 通过 `Dispatch<具体事件类型>(回调函数)` 尝试匹配。
3. 如果运行时类型与模板参数 `T` 的静态类型一致，执行 `static_cast` 并调用回调。
4. 回调返回 `true` 表示事件已被消费 (`m_Handled = true`)，不再继续传播。

### 为什么用 static_cast 而非 dynamic_cast？

`static_cast` 是零开销的编译期转换。由于我们已经通过 `GetEventType() == T::GetStaticType()` 在运行时确认了类型安全，所以不需要 `dynamic_cast` 的 RTTI 开销。这对于每帧可能产生数十个事件的游戏引擎来说至关重要。

## 4. 当前设计的局限性

当前的事件系统是**同步阻塞式**的——事件产生后立即被处理，没有缓冲队列。这意味着：

- 事件不能被延迟处理或批量消费。
- 在高频事件（如 `MouseMoved`）场景下，可能会对性能产生影响。

> **未来优化方向**：引入事件队列 (Event Queue)，将事件的产生与消费解耦为异步模式，在每帧的特定阶段统一 Flush。

## 5. 文件结构

| 文件 | 职责 |
|------|------|
| `Events/Event.h` | 基类 `Event`、枚举定义、宏、`EventDispatcher` |
| `Events/ApplicationEvent.h` | 窗口与应用级事件 |
| `Events/KeyEvent.h` | 键盘输入事件 |
| `Events/MouseEvent.h` | 鼠标输入事件 |

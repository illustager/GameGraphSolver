# GameGraphSolver

前不久，sageblue 就我们小时候很流行的一个小游戏向我提了一个问题。因为这个游戏涉及的合法局面很少（两千多种），所以我很快便通过朴素算法解决了。事实上，我们小时候玩的很多益智小游戏涉及的合法局面都较少，所以我索性把这次工作中与具体游戏无关的部分剥离出来，说不定以后还有用。

这个项目通过**朴素**的**确定算法**处理**完美信息**的回合制游戏。具体来说，你可以通过它判断游戏过程中某个局面是否存在先手（后手）必胜（必不败）策略，并找到一个策略；它也可以判断哪些局面意味着死局——如果该游戏存在死局的话。

## 用法

可以参考 [examples/](examples/) 目录下给出的示例和 README 文档。

首先，根据具体的游戏，定义一个 `GameGraphPositionBase` 的派生类（假设是 `CustomPosition`），并重载以下纯虚方法。这个派生类用于实现对游戏中局面的建模，因此它必须包含所有能够确定一个游戏局面的信息。不等值的 `CustomPosition` 实例对应游戏中的不同局面；相等的实例则表示相同的局面。

```c++
virtual std::vector<std::unique_ptr<GameGraphPositionBase>> get_next_positions() const = 0;
virtual bool is_terminal() const = 0;
virtual bool less(const GameGraphPositionBase* rhs) const = 0;
```

此外，还建议用户定义类似于下面的静态函数或常量用于提供游戏的开始局面，因为它们通常是固定的。

```c++
static std::vector<std::unique_ptr<GameGraphPositionBase>> get_starting_positions();
```

`GameGraphSolver` 则提供以下方法：

```c++
using PositionBase			= GameGraphPositionBase;
using PositionBaseUniquePtr	= std::unique_ptr<PositionBase>;
using PT					= GameGraphSolver::PositionType;

void build_graph(std::vector<PositionBaseUniquePtr> starting_positions);
void color_graph();

PT get_position_type(const PositionBase& pos) const;
PT get_position_type(const PositionBase* pos) const;

std::vector<const PositionBase*> get_positions() const;
std::vector<const PositionBase*> get_terminals() const;
std::vector<const PositionBase*> get_adjacent_positions(const PositionBase& pos) const;
std::vector<const PositionBase*> get_adjacent_positions(const PositionBase* pos) const;

bool is_draw(const PositionBase& position) const;
bool is_draw(const PositionBase* position) const;
```

这些方法应该顾名思义：`build_graph` 和 `color_graph` 完成建图和染色。在完成建图和染色后，`get_position_type` 返回局面类型，`is_draw` 判断一个局面是否意味着死局，`get_positions` 返回所有可能出现的局面，`get_terminals()` 返回游戏终局，`get_adjacent_positions` 返回一个局面在一回合行动后可以到达的所有局面。

其中枚举类 `GameGraphSolver::PositionType` 的所有枚举项及其含义如下：

- **P_POSITION**：当前局面存在后手必胜策略；
- **PT_POSITION**：当前局面存在后手必不败策略；
- **T_POSITION**：当前局面无论先后手都存在必不败策略；
- **NT_POSITION**：当前局面存在先手必不败策略；
- **N_POSITION**：当前局面存在先手必胜策略；
- **UNDETERMINED**：当前局面没有被染色（请检查是否执行了 `color_graph` 方法）；
- **INVALID**：当前局面不是游戏中可能出现的局面（请检查局面是否合法，或者是否执行了 `build_graph` 方法）。

我没有系统学习过博弈论。据我所知，P-Position 和 N-Position 是博弈论中真实存在的术语，而其它称呼是我编程时临时使用的；如果对于上述含义对应有更符合习惯的术语，请让我知道。

## 理论

如果一个游戏可以被建模为一个有限的 DAG（有向无环图），那么不难证明，这个游戏的每一种局面都存在先手或后手必胜策略。具体地，有如下染色法：

1. 出度为 0 的节点是 P-Position；
2. 邻接 P-Position 的节点是 N-Position；
3. 邻接且只邻接 N-Position 的节点是 P-Position；

而当这个有限的有向图中有环时，就**可能**有一些点无法被上述染色法定义（记作 Undefined-Position）。不难发现，由全部 Undefined-Position 导出的子图中一定存在 NTSSCC（Non‑trivial Sink SCC，非平凡汇强连通分量）。只需要补充如下定义，就可以消除所有 Undefined-Position：

4. 由全部 Undefined-Position 导出的子图中的 NTSSCC 上的点是 T-Position；
5. 邻接且只邻接 T-Position 的节点是 T-Position；
6. 邻接 PT-Position 的节点是 NT-Position；
7. 邻接且只邻接 N-Position，NT-Position 或 T-Position，且不只邻接 T-Position 的节点是 PT-Position。

这里的 NTSSCC 是指至少满足以下条件之一的 Sink SCC：

- 节点数不为 1；或
- 存在自环。

我认为上述染色法是良定义，但我不知道如何用严格的数学语言证明它；我只能给一个简单的陈述：考察由全部 Undefined-Position 导出的子图 $G'$。显然 $G'$ 中不存在出度为 0 的点，所以 $G'$ 要么为空，要么存在 NTSSCC。而根据 Def. 4.，$G'$ 中不可能存在 NTSSCC，因此 $G'$ 为空。

## To-do

To-do list:

- `GameGraphSolver` 中用户可以选择使用 `std::map` 还是 `std::unordered_map` 建立 Position 实例指针到实例序号的索引；
- 每次染色 SCC 时，不对整个 Undefined-Position 导出的子图缩点，只对发生变化的 SCC 导出的子图缩点。










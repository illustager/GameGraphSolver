# GameGraphSolver

**[简体中文](README_CN.md)**

> **Note**: The working repository is located at [GitHub](https://github.com/illustager/GameGraphSolver), and repositories on other platforms are read-only mirrors that do not accept PRs.

Not long ago, sageblue asked me a question about a small puzzle game that was popular in our childhood. Since this game involved few legal positions (just over two thousand), I was able to solve it quickly with a brute-force algorithm. In fact, many childhood puzzle games share this property of having a manageable number of legal positions. This inspired me to extract and generalize the game-independent components of my work—perhaps they’ll be useful in the future.

This project processes **perfect-information**,  turn-based games using a **naive** and **deterministic** algorithm. With this tool, you can determine, for any game position, whether there exists a guaranteed winning or non-losing strategy for the first or second player, and also recover such a strategy. It can also detect positions that are deadlocks (i.e., positions that result in perpetual cycles with no possible winner), if such positions exist in the game.

## Usage

See the [tests/](tests/) directory there for usage examples.

### CustomPosition

To use this library, define a class that models the positions of your specific game (for example, `CustomPosition`), and implement the following methods. Each instance of your class should represent a unique game state: unequal instances represent different positions; equal instances, the same position.

```c++
// Returns all positions reachable from the current position in one move
std::vector<std::unique_ptr<CustomPosition>> get_next_positions() const;
// Determines whether the current position is a terminal position
bool is_terminal() const;
// Returns all possible starting positions of the game
static std::vector<std::unique_ptr<CustomPosition>> get_starting_positions();
```

Additionally, `GameGraphSolver` uses a mapping from `CustomPosition` instances to position IDs, using either `std::map` or `std::unordered_map`. If you want to use `std::unordered_map`, you need to define/overload the following methods/operators:

```c++
bool operator==(const CustomPosition& other) const;
std::size_t hash() const;
```

Conversely, if you want to use `std::map`, then:

```c++
bool operator<(const CustomPosition& other) const;
```

### GameGraphSolver

`GameGraphSolver` provides the following methods:

```c++
using PositionBase            = GameGraphPositionBase;
using PositionBaseUniquePtr   = std::unique_ptr<PositionBase>;
using PT                      = GameGraphSolver::PositionType;
  
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

These methods are self-explanatory: `build_graph` and `color_graph` build and color the game graph. Afterward, `get_position_type` returns the type of a position, `is_draw` checks whether a position is a deadlock, `get_positions` returns all possible positions, `get_terminals()` returns terminal positions, and `get_adjacent_positions` returns all positions reachable in a single move from the given position.

`GameGraphSolver::PositionType` enumerates position types as follows:

- **P_POSITION**: The second player has a guaranteed winning strategy from this position.
- **PT_POSITION**: Both players have a guaranteed non-losing strategy from this position, but the first player must choose a deadlock to avoid losing.
- **T_POSITION**: Both players have a guaranteed non-losing strategy from this position.
- **NT_POSITION**: Both players have a guaranteed non-losing strategy from this position, but the second player must choose a deadlock to avoid losing.
- **N_POSITION**: The first player has a guaranteed winning strategy from this position.
- **UNDETERMINED**: The position has not been classified yet (make sure to call `color_graph`).
- **INVALID**: The position is not recognized as a valid position in the game (check position legality or ensure `build_graph` was called).

Except for `UNDETERMINED` and `INVALID`, the five position types are ordered by favorability to the first player. For any two classified legal positions, you can compare their `PositionType` values using `operator<` and `operator>` to determine which position is more advantageous for the first player.

I have not systematically studied game theory. To my knowledge, "P-Position" and "N-Position" are standard terms in game theory, while the other names are ones I came up with while programming; if there are more standard terms for these cases, please let me know.

## Theory

If a game can be modeled as a finite DAG (Directed Acyclic Graph), then for every position, either the first or the second player has a guaranteed winning strategy. Specifically, the classification proceeds as follows:

1. Nodes with out-degree 0 are P-Positions;
2. Any node adjacent to a P-Position is an N-Position;
3. Any node whose adjacent nodes are all N-Positions is a P-Position;

If the finite directed graph contains cycles, some positions may not be classifiable by the above method (called Undefined-Positions). It is straightforward to verify that within the subgraph induced by all Undefined-Positions, there must exist at least one NTSSCC (Non-Trivial Sink Strongly Connected Component). Introducing further definitions allows classifying all positions:

4. In the subgraph induced by all Undefined-Positions, nodes in NTSSCCs are T-Positions;
5. Any node adjacent only to T-Positions is a T-Position;
6. Any node adjacent to a PT-Position is an NT-Position;
7. Any node adjacent only to N-, NT-, or T-Position, and not solely to T-Positions or solely to N-Positions, is a PT-Position.

Here, an NTSSCC is a sink strongly connected component that:
- Contains more than one node, or
- Contains a self-loop.

I believe this classification is well-defined, although I do not have a rigorous proof; intuitively: in the subgraph $G'$ of Undefined-Positions, there are no nodes with out-degree 0, so $G'$ is either empty or contains at least one NTSSCC. By Def. 4. above, $G'$ cannot contain any NTSSCC, so it must be empty.

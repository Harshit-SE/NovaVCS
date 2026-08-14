# NovaVCS Merge Engine

The Merge Engine performs integration of separate development histories using a Three-Way Merge algorithm. 

## Algorithm & Time Complexity
The engine evaluates files line-by-line, computing the difference between `Base -> Ours` and `Base -> Theirs`. 
Because it relies on parallel linear traversal without backtracking through unmodified hunks, the time complexity is bounded by $O(N)$, where $N$ is the maximum number of lines across the three input states.

## Merge Logic
1. **Match:** If `Ours` == `Theirs`, the change is cleanly accepted.
2. **Fast-Forward Line:** If `Ours` == `Base` but `Theirs` != `Base`, `Theirs` is accepted.
3. **Conflict:** If `Ours` != `Base`, `Theirs` != `Base`, and `Ours` != `Theirs`, the algorithm halts automatic resolution for that block and throws a Conflict.

## AI-Assisted Resolution Architecture
To support future integration with AI agents or backend analytics, conflicts do not just emit raw text strings. When a conflict occurs, the engine serializes the data into a structured payload via `generateAIContext()`.

**Sample Context Payload:**
```json
{
  "conflict_type": "three_way",
  "base": "void init() {}",
  "ours": "void init(int memory_limit) {}",
  "theirs": "void init(string db_path) {}"
}

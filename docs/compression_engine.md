# NovaVCS Compression Engine

The Compression Engine is responsible for the transition from loose object storage to optimized `packfiles`. This mirrors Git's garbage collection architecture, dramatically reducing repository footprint on disk.

## Mathematics & Formulas

**Compression Ratio ($C_r$)**
Calculates the efficiency of the packaging mechanism:
$$ C_r = \frac{\text{Uncompressed Size}}{\text{Compressed Size}} $$

**Space Saved Percentage**
Determines the direct storage reclamation:
$$ \text{Saved} = \left( 1 - \frac{1}{C_r} \right) \times 100 $$

## Algorithms Used

| Algorithm | Role | Time Complexity |
| :--- | :--- | :--- |
| **Huffman Coding** | Entropy encoding for compressing raw text/binary blobs. | $O(N \log N)$ |
| **Delta Compression** | Computes the edit distance instruction set between similar blobs. | $O(ND)$ |

## Packfile Structure
The output is stored in `.nova/pack/nova_data.pack`. The file relies on concatenated Object headers, actual byte sizes, and the encoded data stream, allowing for sequential read/write operations during network pushes.

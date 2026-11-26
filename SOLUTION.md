# ESet speedtest solution

The standalone driver implements the seven operations documented by the upstream
`SJTUJohnClass/ESet/test/1.cpp`. Each set is a randomized treap with distinct
signed 64-bit keys. Subtree sizes support inclusive range counts by subtracting
the number of keys below the lower endpoint from the number at or below the
upper endpoint. This avoids overflow at either signed 64-bit limit.

The shared iterator stores its owning set and key. A successful insertion or
find selects it; unsuccessful operations preserve it. Erasing its key or moving
past either boundary invalidates it. Strict predecessor/successor searches
implement movement without depending on node addresses or tree rotations.

Tree nodes occupy 24 bytes and are allocated in dynamically added blocks.
Deleted nodes are recycled. Copying a set clones its tree, preserving independence
and priorities. Insert, erase, lookup, range count, and iterator movement take
expected O(log n) time; copying takes O(n) time and space. Storage is proportional
to the high-water mark of allocated nodes, with no fixed input-size limit.

## Validation

- Both input/output samples from the linked upstream repository matched exactly.
- `python3 tests/differential.py` passed 50 seeds and 500,000 operations against a
  sorted-list model, including copies, duplicates, invalid iterators, reversed
  ranges, and signed 64-bit endpoints.
- AddressSanitizer and UndefinedBehaviorSanitizer passed the same randomized suite.
- The local data path specified in README.md was not present in this environment.

## Judge result

- Submission ID: **879554** (first attempt).
- Submitted commit: `380be1f149d5aa6d568c956b3713dfb76e2d250c`.
- Result: **Accepted**, all six groups passed, reported score **25/25**.
- Maximum individual test runtime: 2909 ms.
- Reported peak memory: 206102528 bytes. The actual judge accepted this usage;
  its limits differ from the 58 MiB limit stated in the supplied README.
- Push success was verified by comparing local HEAD with `git ls-remote`.
- No repository edits, commits, or pushes were made while judging was active.

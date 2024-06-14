# TODO

- allow multiple connections
    - multithreaded: implement thread pool from scratch
        - look into std::shared_mutex
    - event loop, non blocking IO
    - coroutines, boost or raw
- support more data structures
    - lists (implement ziplists)
    - sets
    - hashes
- implement hashmap from scratch
- expiry
- support more commands
    - set NX -- Only set the key if it does not already exist.
      XX -- Only set the key if it already exists.
- write ahead log -> persistence to disk


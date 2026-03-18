# Benchmark Baseline (P5)

## Host snapshot

- Environment: OpenClaw workspace host (Linux)
- Compiler/toolchain: GCC via CMake Release
- Build mode: `-DCMAKE_BUILD_TYPE=Release`
- Benchmark binary: `build/ohmycmd_bench_dispatch`

## Command

```bash
./build/ohmycmd_bench_dispatch 5000 600000
```

## Result sample (2026-03-18, 3 runs)

```text
dispatch_benchmark | commands=5000 | iterations=600000 | hits=600000 | elapsed_ms=222.12 | ns_per_op=370.21 | ops_per_sec=2701196.49
dispatch_benchmark | commands=5000 | iterations=600000 | hits=600000 | elapsed_ms=177.74 | ns_per_op=296.24 | ops_per_sec=3375651.99
dispatch_benchmark | commands=5000 | iterations=600000 | hits=600000 | elapsed_ms=203.12 | ns_per_op=338.54 | ops_per_sec=2953875.08
```

- Median: `338.54 ns/op` (~`2.95M ops/s`)

## Notes

- Benchmark measures parser + registry lookup path (without Pawn callback execution).
- Use this as a relative baseline between commits, not as an absolute cross-machine metric.

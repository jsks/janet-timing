janet-timing
---

Microbenchmark Janet code on POSIX compliant systems.

```shell
$ jpm install https://github.com/jsks/janet-timing
```

Use the `elapsed` macro to time the execution of an expression.

```janet
> (import timing)
> (timing/elapsed (filter odd? (range 0 100)) :times 1e4)
36.94 us [± 10.43 us] IQR: 31.22 .. 39.39
```

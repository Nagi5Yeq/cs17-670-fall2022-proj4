(module
  (global $g (mut i32) (i32.const 20))
  (func $ref (export "main") (param i32) (result i32)
    (global.get $g)
    (br_if 0 (i32.le_s (local.get 0) (i32.const 0)))
    (i32.const 1)
    (i32.add)
    (global.set $g)
    (call $ref (i32.sub (local.get 0) (i32.const 1)))
  )
)

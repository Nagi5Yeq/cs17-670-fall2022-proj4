(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.eq" (func $eq (param externref externref) (result i32)))

  (func $equal (param externref externref i32) (result i32)
    (local.get 2)
    (br_if 0 (call $eq (local.get 0) (local.get 1)))
    (drop)
    (i32.const 0)
  )

  (func (export "main") (result i32)
    (local $o1 externref)
    (local $o2 externref)
    (local $o3 externref)
    (local.set $o2 (call $n))
    (local.set $o3 (call $n))
    (i32.or
      (i32.or
        (i32.or
          (call $equal (local.get 0) (local.get 0) (i32.const 1))
          (call $equal (local.get 0) (local.get 1) (i32.const 2)))
        (i32.or
          (call $equal (local.get 0) (local.get 2) (i32.const 4))
          (call $equal (local.get 1) (local.get 1) (i32.const 8))))
      (call $equal (local.get 1) (local.get 2) (i32.const 16)))
  )
)

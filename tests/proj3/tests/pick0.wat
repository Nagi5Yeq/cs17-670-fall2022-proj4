(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.eq" (func $eq (param externref externref) (result i32)))

  (func $get (param externref externref externref i32) (result externref)
    (local.get 0)
    (br_if 0 (i32.eq (local.get 3) (i32.const 0)))
    (drop)
    
    (local.get 1)
    (br_if 0 (i32.eq (local.get 3) (i32.const 1)))
    (drop)

    (local.get 2)
  )

  (func (export "main") (param i32) (result i32)
    (local $a externref)
    (local $b externref)
    (local $c externref)
    (local.set $a (call $n))
    (local.set $b (call $n))
    (local.set $c (call $n))

    (call $eq (call $get (local.get $a) (local.get $b) (local.get $c) (local.get 0)) (local.get $a))
  )
)
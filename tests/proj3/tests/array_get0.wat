(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.box_i32" (func $b (param i32) (result externref)))
  (import "weewasm" "i32.unbox" (func $u (param externref) (result i32)))
  (import "weewasm" "obj.get" (func $g (param externref externref) (result externref)))
  (import "weewasm" "obj.set" (func $s (param externref externref externref)))
  
  (func $alloc (param i32) (result externref)
    (local $obj externref)
    (local.set $obj (call $n))
    (loop $l
      (call $s (local.get $obj) (call $b (local.get 0)) (call $b (local.get 0)))
      (local.set 0 (i32.sub (local.get 0) (i32.const 1)))
      (br_if $l (i32.ge_s (local.get 0) (i32.const 0)))
    )
    (local.get $obj)
  )

  (func $get (param externref i32) (result i32)
    (call $u (call $g (local.get 0) (call $b (local.get 1))))
  )

  (func (export "main") (param i32) (result i32)
    (call $get (call $alloc (local.get 0)) (i32.const 4))
  )
)

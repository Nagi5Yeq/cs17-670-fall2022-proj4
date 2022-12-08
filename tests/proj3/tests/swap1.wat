(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.eq" (func $eq (param externref externref) (result i32)))

  (func $swap (param externref externref) (result externref)
    (local.get 1)
  )

  (func (export "main") (result i32)
    (local $a externref)
    (local $b externref)
    (local.set $a (call $n))
    (local.set $b (call $n))
    
    (call $eq (call $swap (local.get $b) (local.get $a)) (local.get $b))
  )
)
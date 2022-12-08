(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.eq" (func $eq (param externref externref) (result i32)))
  
  (func (export "main") (result i32)
    (local $null externref)
    (call $eq (local.get $null) (call $n))
  )
)

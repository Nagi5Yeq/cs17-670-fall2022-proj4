(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.eq" (func $eq (param externref externref) (result i32)))
  
  (func (export "main") (result i32)
    (call $eq (call $n) (call $n))
  )
)

(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  
  (table funcref (elem $main $empty))
  (type $v_r (func (param externref) (result externref)))
  
  (func $main (export "main") (param i32) (result i32)
    (call_indirect (type $v_r) (call $n) (local.get 0))
    (drop)
    (i32.const -1865)
  )
  (func $empty (param externref) (result externref)
    (call $n)
  )
)

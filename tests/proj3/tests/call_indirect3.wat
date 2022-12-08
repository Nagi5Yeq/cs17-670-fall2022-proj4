(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  
  (table funcref (elem $main $empty $n))
  (type $v_r (func (result externref)))
  (func $main (export "main") (param i32) (result i32)
    (call_indirect (type $v_r) (local.get 0))
    (drop)
    (i32.const -1877)
  )
  (func $empty (result externref)
    (call $n)
  )
)

(module
  (table 5 funcref)
  (type $v_r (func (result externref)))
  
  (func (export "main") (param i32) (result i32)
    (call_indirect (type $v_r) (local.get 0))
    (drop)
    (i32.const 0)
  )
)

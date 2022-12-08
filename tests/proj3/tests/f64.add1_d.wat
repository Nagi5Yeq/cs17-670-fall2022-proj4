(module
  (import "weewasm" "obj.box_f64" (func $b (param f64) (result externref)))
  (import "weewasm" "f64.unbox" (func $u (param externref) (result f64)))
  
  (func (export "main") (param f64 f64) (result f64)
    (f64.add (call $u (call $b (local.get 0))) (local.get 1))
  )
)

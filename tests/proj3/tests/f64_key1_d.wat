(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.box_f64" (func $b (param f64) (result externref)))
  (import "weewasm" "f64.unbox" (func $u (param externref) (result f64)))
  (import "weewasm" "obj.set" (func $s (param externref externref externref)))
  (import "weewasm" "obj.get" (func $g (param externref externref) (result externref)))
  
  (func (export "main") (param f64) (result f64)
    (local $obj externref)
    (local.set $obj (call $n))
    (call $s (local.get $obj) (call $b (local.get 0)) (call $b (f64.add (local.get 0) (f64.const 100))))
    (call $u (call $g (local.get $obj) (call $b (local.get 0))))
  )
)

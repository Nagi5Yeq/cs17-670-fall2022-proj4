(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.get" (func $g (param externref externref) (result externref)))
  (import "weewasm" "obj.set" (func $s (param externref externref externref)))
  (import "weewasm" "obj.box_f64" (func $b (param f64) (result externref)))
  (import "weewasm" "obj.box_i32" (func $bi (param i32) (result externref)))
  (import "weewasm" "i32.unbox" (func $u (param externref) (result i32)))
  (func (export "main") (param f64 i32) (result i32)
    (local $f externref)
    (local.set $f (call $n))
    (call $s (local.get $f) (call $b (local.get 0)) (call $bi (local.get 1)))
    (call $u (call $g (local.get $f) (call $b (f64.convert_i32_s (local.get 1)))))
  )
)

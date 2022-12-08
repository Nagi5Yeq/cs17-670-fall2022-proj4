(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.box_i32" (func $b (param i32) (result externref)))
  (import "weewasm" "i32.unbox" (func $u (param externref) (result i32)))
  (import "weewasm" "obj.get" (func $g (param externref externref) (result externref)))
  (import "weewasm" "obj.set" (func $s (param externref externref externref)))
  (import "weewasm" "obj.eq" (func $eq (param externref externref) (result i32)))

  (func $array_get (param externref i32) (result externref)
    (call $g (local.get 0) (call $b (local.get 1)))
  )

  (func $array_set (param externref i32 externref)
    (call $s (local.get 0) (call $b (local.get 1)) (local.get 2))
  )

  (func $left_key (param externref) (result externref)
    (call $array_get (local.get 0) (i32.const 11)))

  (func $right_key (param externref) (result externref)
    (call $array_get (local.get 0) (i32.const 22)))

  (func $keys (result externref)
    (local $r externref)
    (local.set $r (call $n))
    (call $array_set (local.get $r) (i32.const 11) (call $n))
    (call $array_set (local.get $r) (i32.const 22) (call $n))
    (local.get $r)
  )

  (func $isnull (param externref) (result i32)
    (local $n externref)
    (call $eq (local.get $n) (local.get 0)))

  (func $alloc (param externref i32) (result externref)
    (local $obj externref)
    (local.set $obj (call $n))
    (block $l
      (local.set 1 (i32.sub (local.get 1) (i32.const 1)))
      (br_if $l (i32.le_s (local.get 1) (i32.const 0)))
      (call $s (local.get $obj) (call $left_key (local.get 0)) (call $alloc (local.get 0) (local.get 1)))
      (call $s (local.get $obj) (call $right_key (local.get 0)) (call $alloc (local.get 0) (local.get 1)))
    )
    (local.get $obj)
  )

  (func $height (param externref externref) (result i32)
    (local $h i32)
    (block $b
    (loop $l
      (br_if $b (call $isnull (local.get 1)))
      (local.set $h (i32.add (local.get $h) (i32.const 1)))
      (local.set 1 (call $g (local.get 1) (call $left_key (local.get 0))))
      (br 0)
    )
    )
    (local.get $h)
  )

  (func (export "main") (param i32) (result i32)
    (local $k externref)
    (local $t externref)
    (local.set $k (call $keys))
    (local.set $t (call $alloc (local.get $k) (local.get 0)))
    (call $height (local.get $k) (local.get $t))
  )
)

(module
  (import "weewasm" "obj.new" (func $n (result externref)))
  (import "weewasm" "obj.box_i32" (func $b (param i32) (result externref)))
  (import "weewasm" "i32.unbox" (func $u (param externref) (result i32)))
  (import "weewasm" "obj.get" (func $g (param externref externref) (result externref)))
  (import "weewasm" "obj.set" (func $s (param externref externref externref)))
  
  (func $cons (param i32 externref) (result externref)
    (local $obj externref)
    (local.set $obj (call $n))
    (call $s (local.get $obj) (call $b (i32.const 0)) (call $b (local.get 0)))
    (call $s (local.get $obj) (call $b (i32.const 1)) (local.get 1))
    (local.get $obj)
  )

  (func $cdr (param externref) (result i32)
    (call $u (call $g (local.get 0) (call $b (i32.const 0))))
  )  

  (func $car (param externref) (result externref)
    (call $g (local.get 0) (call $b (i32.const 1)))
  )  

  (func $get (param externref i32) (result i32)
    (block $b
      (loop $l
        (br_if $b (i32.eqz (local.get 1)))
        (local.set 0 (call $car (local.get 0)))
	(local.set 1 (i32.sub (local.get 1) (i32.const 1)))
	(br $l)
      )
    )
    (call $cdr (local.get 0))
  )

  (func $alloc (param i32) (result externref)
    (local $r externref)
    (block $b
      (loop $l
        (br_if $b (i32.eqz (local.get 0)))
        (local.set $r (call $cons (i32.mul (local.get 0) (i32.const 33)) (local.get $r)))
	(local.set 0 (i32.sub (local.get 0) (i32.const 1)))
	(br $l)
      )
    )
    (local.get $r)
  )

  (func (export "main") (param i32 i32) (result i32)
    (local $null externref)
    (local $r externref)
    (local.set $r (call $alloc (local.get 0)))
    (call $get (local.get $r) (local.get 1))
  )
)
